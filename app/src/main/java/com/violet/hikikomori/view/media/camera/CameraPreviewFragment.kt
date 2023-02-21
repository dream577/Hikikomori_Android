package com.violet.hikikomori.view.media.camera

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Color
import android.graphics.ImageFormat
import android.hardware.camera2.*
import android.media.Image
import android.media.ImageReader
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.HandlerThread
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.View
import androidx.core.graphics.drawable.toDrawable
import androidx.exifinterface.media.ExifInterface
import androidx.lifecycle.Observer
import androidx.lifecycle.lifecycleScope
import androidx.navigation.NavController
import androidx.navigation.Navigation
import androidx.navigation.fragment.navArgs
import com.violet.hikikomori.R
import com.violet.hikikomori.databinding.FragmentCameraPreviewBinding
import com.violet.hikikomori.model.CombinedCaptureResult
import com.violet.hikikomori.view.base.BaseBindingFragment
import com.violet.hikikomori.view.media.camera.util.OrientationLiveData
import com.violet.hikikomori.view.media.camera.util.computeExifOrientation
import com.violet.hikikomori.view.media.camera.util.getPreviewOutputSize
import com.violet.libbasetools.util.KLog
import com.violet.libbasetools.util.createFile
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.suspendCancellableCoroutine
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.util.concurrent.ArrayBlockingQueue
import java.util.concurrent.TimeoutException
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException
import kotlin.coroutines.suspendCoroutine

class CameraPreviewFragment : BaseBindingFragment<FragmentCameraPreviewBinding>(),
    View.OnClickListener {

    companion object {
        private const val IMAGE_BUFFER_SIZE: Int = 3

        private const val IMAGE_CAPTURE_TIMEOUT_MILLIS: Long = 5000
    }

    private val args: CameraPreviewFragmentArgs by navArgs()

    private val navController: NavController by lazy {
        Navigation.findNavController(requireActivity(), R.id.nav_host_fragment)
    }

    private val mCameraManager: CameraManager by lazy {
        val context = requireContext().applicationContext
        context.getSystemService(Context.CAMERA_SERVICE) as CameraManager
    }

    private val characteristics: CameraCharacteristics by lazy {
        mCameraManager.getCameraCharacteristics(args.cameraId)
    }

    private lateinit var imageReader: ImageReader

    private val cameraThread = HandlerThread("CameraThread").apply { start() }

    private val cameraHandler = Handler(cameraThread.looper)

    private val imageReaderThread = HandlerThread("imageReaderThread").apply { start() }

    private val imageReaderHandler = Handler(imageReaderThread.looper)

    private lateinit var cameraDevice: CameraDevice

    private lateinit var session: CameraCaptureSession

    private lateinit var relativeOrientation: OrientationLiveData

    private val animationTask: Runnable by lazy {
        Runnable {
            // Flash white animation
            mBinding.overlay.background = Color.argb(150, 255, 255, 255).toDrawable()
            // Wait for ANIMATION_FAST_MILLIS
            mBinding.overlay.postDelayed({
                // Remove white flash animation
                mBinding.overlay.background = null
            }, 50L)
        }
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        mBinding.listener = this
        mBinding.captureButton.setOnApplyWindowInsetsListener { v, insets ->
            v.translationX = (-insets.systemWindowInsetRight).toFloat()
            v.translationY = (-insets.systemWindowInsetBottom).toFloat()
            insets.consumeSystemWindowInsets()
        }

        mBinding.captureSurfaceView.holder.addCallback(object : SurfaceHolder.Callback {
            override fun surfaceDestroyed(holder: SurfaceHolder) = Unit

            override fun surfaceChanged(
                holder: SurfaceHolder,
                format: Int,
                width: Int,
                height: Int
            ) = Unit

            override fun surfaceCreated(holder: SurfaceHolder) {
                // Selects appropriate preview size and configures view finder
                val previewSize = getPreviewOutputSize(
                    mBinding.captureSurfaceView.display,
                    characteristics,
                    SurfaceHolder::class.java
                )
                KLog.d(TAG, "View finder size: ${mBinding.captureSurfaceView.width} x ${mBinding.captureSurfaceView.height}")
                KLog.d(TAG, "Selected preview size: $previewSize")
                mBinding.captureSurfaceView.setAspectRatio(
                    previewSize.width,
                    previewSize.height
                )

                // To ensure that size is set, initialize camera in the view's thread
                view.post { initializeCamera() }
            }
        })

        // Used to rotate the output media to match device orientation
        relativeOrientation = OrientationLiveData(requireContext(), characteristics).apply {
            observe(viewLifecycleOwner, Observer { orientation ->
                KLog.d(TAG, "Orientation changed: $orientation")
            })
        }
    }

    /**
     * Begin all camera operations in a coroutine in the main thread. This function:
     * - Opens the camera
     * - Configures the camera session
     * - Starts the preview by dispatching a repeating capture request
     * - Sets up the still image capture listeners
     */
    private fun initializeCamera() = lifecycleScope.launch(Dispatchers.Main) {
        // Open the selected camera
        cameraDevice = openCamera(mCameraManager, args.cameraId, cameraHandler)

        // Initialize an image reader which will be used to capture still photos
        val size = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP)!!
            .getOutputSizes(args.pixelFormat).maxByOrNull { it.height * it.width }!!
        imageReader =
            ImageReader.newInstance(size.width, size.height, args.pixelFormat, IMAGE_BUFFER_SIZE)

        // Creates list of Surfaces where the camera will output frames
        val targets = listOf(mBinding.captureSurfaceView.holder.surface, imageReader.surface)

        // Start a capture session using our open camera and list of Surfaces where frames will go
        session = createCaptureSession(cameraDevice, targets, cameraHandler)

        val captureRequest =
            cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW).apply {
                addTarget(mBinding.captureSurfaceView.holder.surface)
            }

        // This will keep sending the capture request as frequently as possible until the
        // session is torn down or session.stopRepeating() is called
        session.setRepeatingRequest(captureRequest.build(), null, cameraHandler)

        mBinding.captureBtnListener = View.OnClickListener {
            // Disable click listener to prevent multiple requests simultaneously in flight
            it.isEnabled = false

            // Perform I/O heavy operations in a different scope
            lifecycleScope.launch(Dispatchers.IO) {
                takePhoto().use { result ->
                    KLog.d(TAG, "Result received: $result")

                    val output = saveResult(result)
                    KLog.d(TAG, "Image saved:${output.absolutePath}")

                    if (output.extension == "jpg") {
                        val exif = ExifInterface(output.absolutePath)
                        exif.setAttribute(
                            ExifInterface.TAG_ORIENTATION, result.orientation.toString()
                        )
                        exif.saveAttributes()
                        KLog.d(TAG, "EXIF metadata saved: ${output.absolutePath}")
                    }
                }

                // Re-enable click listener after photo is taken
                it.post { it.isEnabled = true }
            }

        }
    }

    @SuppressLint("MissingPermission")
    private suspend fun openCamera(
        manager: CameraManager,
        cameraId: String,
        handler: Handler? = null
    ): CameraDevice = suspendCancellableCoroutine { cont ->
        manager.openCamera(cameraId, object : CameraDevice.StateCallback() {
            override fun onOpened(camera: CameraDevice) = cont.resume(camera)

            override fun onDisconnected(camera: CameraDevice) {
                KLog.d(TAG, "Camera $cameraId hsa been disconnected")
                requireActivity().finish()
            }

            override fun onError(camera: CameraDevice, error: Int) {
                val msg = when (error) {
                    ERROR_CAMERA_DEVICE -> "Fatal (device)"
                    ERROR_CAMERA_DISABLED -> "Device policy"
                    ERROR_CAMERA_IN_USE -> "Camera in use"
                    ERROR_CAMERA_SERVICE -> "Fatal (service)"
                    ERROR_MAX_CAMERAS_IN_USE -> "Maximum cameras in use"
                    else -> "Unknown"
                }
                val exc = RuntimeException("Camera $cameraId error: ($error) $msg")
                KLog.e(TAG, exc.message!!, exc)
                if (cont.isActive) cont.resumeWithException(exc)
            }

        }, handler)
    }

    private suspend fun createCaptureSession(
        cameraDevice: CameraDevice,
        targets: List<Surface>,
        handler: Handler? = null
    ): CameraCaptureSession = suspendCoroutine { cont ->
        cameraDevice.createCaptureSession(targets, object : CameraCaptureSession.StateCallback() {
            override fun onConfigured(session: CameraCaptureSession) = cont.resume(session)

            override fun onConfigureFailed(session: CameraCaptureSession) {
                val exc = RuntimeException("Camera ${cameraDevice.id} session configuration failed")
                KLog.e(TAG, exc.message!!, exc)
                cont.resumeWithException(exc)
            }
        }, handler);
    }

    private suspend fun takePhoto():
            CombinedCaptureResult = suspendCoroutine { cont ->

        // Flush any images left in the image reader
        while (imageReader.acquireNextImage() != null) {
        }

        // Start a new image queue
        val imageQueue = ArrayBlockingQueue<Image>(IMAGE_BUFFER_SIZE)
        imageReader.setOnImageAvailableListener({ reader ->
            val image = reader.acquireNextImage()
            KLog.d(TAG, "Image Available in queue: ${imageReader.surface}")
            imageQueue.add(image)
        }, imageReaderHandler)

        val captureRequest =
            cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_STILL_CAPTURE).apply {
                addTarget(imageReader.surface)
            }

        session.capture(captureRequest.build(), object : CameraCaptureSession.CaptureCallback() {
            override fun onCaptureStarted(
                session: CameraCaptureSession,
                request: CaptureRequest,
                timestamp: Long,
                frameNumber: Long
            ) {
                super.onCaptureStarted(session, request, timestamp, frameNumber)
                mBinding.captureSurfaceView.post(animationTask)
            }

            override fun onCaptureCompleted(
                session: CameraCaptureSession,
                request: CaptureRequest,
                result: TotalCaptureResult
            ) {
                super.onCaptureCompleted(session, request, result)
                val resultTimestamp = result.get(CaptureResult.SENSOR_TIMESTAMP)
                KLog.d(TAG, "Capture result received: $resultTimestamp")

                // Set a timeout in case image captured is dropped from the pipeline
                val exc = TimeoutException("Image dequeuing took too long")
                val timeoutRunnable = Runnable { cont.resumeWithException(exc) }
                imageReaderHandler.postDelayed(timeoutRunnable, IMAGE_CAPTURE_TIMEOUT_MILLIS)

                lifecycleScope.launch(cont.context) {
                    while (true) {
                        // Dequeue images while timestamps don't match
                        val image = imageQueue.take()
                        // if (image.timestamp != resultTimestamp) continue
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q &&
                            image.format != ImageFormat.DEPTH_JPEG &&
                            image.timestamp != resultTimestamp
                        ) continue
                        KLog.d(TAG, "Matching image dequeued: ${image.timestamp}")

                        imageReaderHandler.removeCallbacks(timeoutRunnable)
                        imageReader.setOnImageAvailableListener(null, null)

                        while (imageQueue.size > 0) {
                            imageQueue.take().close()
                        }

                        // Compute EXIF orientation metadata
                        val rotation = relativeOrientation.value ?: 0
                        val mirrored = characteristics.get(CameraCharacteristics.LENS_FACING) ==
                                CameraCharacteristics.LENS_FACING_FRONT
                        val exifOrientation = computeExifOrientation(rotation, mirrored)

                        // Build the result and resume progress
                        cont.resume(
                            CombinedCaptureResult(
                                image, result, exifOrientation, imageReader.imageFormat
                            )
                        )

                        // There is no need to break out of the loop, this coroutine will suspend
                    }
                }
            }
        }, cameraHandler);
    }

    private suspend fun saveResult(result: CombinedCaptureResult): File = suspendCoroutine { cont ->
        when (result.format) {
            ImageFormat.JPEG, ImageFormat.DEPTH_JPEG -> {
                val buffer = result.image.planes[0].buffer
                val bytes = ByteArray(buffer.remaining()).apply {
                    buffer.get(this)
                }
                try {
                    val output = createFile(requireContext(), "jpg")
                    FileOutputStream(output).use {
                        it.write(bytes)
                    }
                    cont.resume(output)
                } catch (exc: IOException) {
                    KLog.e(TAG, "Unable to write JPEG image to file", exc)
                    cont.resumeWithException(exc)
                }
            }

            ImageFormat.RAW_SENSOR -> {
                val dngCreator = DngCreator(characteristics, result.metadata)
                try {
                    val output = createFile(requireContext(), "dng")
                    FileOutputStream(output).use { dngCreator.writeImage(it, result.image) }
                    cont.resume(output)
                } catch (exc: IOException) {
                    KLog.e(TAG, "Unable to write DNG image to file", exc)
                    cont.resumeWithException(exc)
                }
            }

            // No other formats are supported by this sample
            else -> {
                val exc = RuntimeException("Unknown image format: ${result.image.format}")
                KLog.e(TAG, exc.message!!, exc)
                cont.resumeWithException(exc)
            }
        }
    }

    override fun onStop() {
        super.onStop()
        try {
            cameraDevice.close()
        } catch (exc: Throwable) {
            Log.e(TAG, "Error closing camera", exc)
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        cameraThread.quitSafely()
        imageReaderThread.quitSafely()
    }

    private fun listAllCameras() {
        try {
            val cameraIdList = mCameraManager.cameraIdList

            for (cameraId in cameraIdList) {
                val characteristics = mCameraManager.getCameraCharacteristics(cameraId)
                val cameraLensFacing = characteristics.get(CameraCharacteristics.LENS_FACING)
                val cameraCapabilities =
                    characteristics.get(CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES)

                // check if the selected camera device supports basic features
                // ensures backward compatibility with the original Camera API
                val isBackwardCompatible = cameraCapabilities
                    ?.contains(CameraMetadata.REQUEST_AVAILABLE_CAPABILITIES_BACKWARD_COMPATIBLE)
                    ?: false
            }

        } catch (e: CameraAccessException) {
            e.message?.let { KLog.d(TAG, it) }
        }
    }

    private fun getFirstCameraIdFacing(
        cameraManager: CameraManager,
        facing: Int = CameraMetadata.LENS_FACING_BACK
    ): String? {
        try {
            // Get list of all compatible cameras
            val cameraIds = cameraManager.cameraIdList.filter {
                val characteristics = mCameraManager.getCameraCharacteristics(it)
                val capabilities =
                    characteristics.get(CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES)
                capabilities?.contains(CameraMetadata.REQUEST_AVAILABLE_CAPABILITIES_BACKWARD_COMPATIBLE)
                    ?: false
            }

            cameraIds.forEach {
                val characteristics = mCameraManager.getCameraCharacteristics(it)
                if (characteristics.get(CameraCharacteristics.LENS_FACING) == facing) {
                    return it
                }
            }

            return cameraIds.firstOrNull()
        } catch (e: CameraAccessException) {
            e.message?.let { KLog.d(TAG, it) }
        }
        return null
    }

    override fun getLayoutId(): Int = R.layout.fragment_camera_preview

    override fun onClick(v: View?) {

    }

}