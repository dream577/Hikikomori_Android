package com.violet.hikikomori.view.media.camera

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.ImageFormat
import android.graphics.SurfaceTexture
import android.hardware.camera2.*
import android.media.ImageReader
import android.media.ImageReader.OnImageAvailableListener
import android.os.Handler
import android.os.HandlerThread
import android.text.TextUtils
import android.util.Size
import android.view.Surface
import android.view.SurfaceHolder
import android.view.View
import androidx.lifecycle.lifecycleScope
import com.violet.hikikomori.R
import com.violet.hikikomori.databinding.FragmentCameraRecordBinding
import com.violet.hikikomori.view.base.BaseBindingFragment
import com.violet.libbasetools.util.KLog
import com.violet.libmedia.MediaContext
import com.violet.libmedia.recoder.video.camera.CameraFrameCallback
import com.violet.libmedia.recoder.video.camera.CameraRecordClient
import com.violet.libmedia.recoder.video.camera.util.ImageUtils
import com.violet.libmedia.recoder.video.camera.util.getPreviewOutputSize
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.suspendCancellableCoroutine
import java.util.concurrent.atomic.AtomicBoolean
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException

class CameraRecordFragment : BaseBindingFragment<FragmentCameraRecordBinding>(),
    SurfaceHolder.Callback, CameraFrameCallback, View.OnClickListener {
    companion object {
        const val TAG = "CameraRecordFragment"
    }

    private val DEFAULT_CAMERA_ID = "0"
    private val THRESHOLD = 0.001f

    private lateinit var mCameraManager: CameraManager
    private lateinit var mCharacteristics: CameraCharacteristics
    private var mCameraDevice: CameraDevice? = null
    private var mCameraId: String = DEFAULT_CAMERA_ID
    private lateinit var mSupportCameraIds: Array<String>

    private var mCaptureSession: CameraCaptureSession? = null
    private var mPreviewRequest: CaptureRequest? = null
    private var mCaptureRequest: CaptureRequest? = null
    private var mPreviewImageReader: ImageReader? = null
    private var mCaptureImageReader: ImageReader? = null
    private var mPreviewSurface: Surface? = null
    private var mCaptureSurface: Surface? = null
    private var mSensorOrientation: Int? = null

    private val mDefaultPreviewSize = Size(1280, 720)
    private val mDefaultCaptureSize = Size(1280, 720)
    private var mPreviewSize = mDefaultPreviewSize
    private var mCaptureSize = mDefaultCaptureSize
    private lateinit var mSupportPreviewSize: List<Size>
    private lateinit var mSupportCaptureSize: List<Size>

    private val mPreviewThread = HandlerThread("CameraThread")
    private var mCameraHandler: Handler? = null

    private lateinit var mCameraRecordClient: CameraRecordClient

    private var isRecording = AtomicBoolean(false)

    override fun initData() {
        super.initData()
        mCameraRecordClient = CameraRecordClient()
    }

    override fun initView() {
        super.initView()
        mBinding.btnRecord.setOnClickListener(this)
        mBinding.cameraRecordView.holder.addCallback(this)
    }

    private fun initializeEnv() = lifecycleScope.launch(Dispatchers.Main) {
        val result = initializeCamera()
        if (result) {
            mCameraDevice = openCamera()
            mCaptureSession = createCaptureSession()
            createPreviewRequest()
        }
    }

    private suspend fun initializeCamera(): Boolean = suspendCancellableCoroutine { cont ->
        var result = false
        do {
            mCameraManager =
                requireContext().getSystemService(Context.CAMERA_SERVICE) as CameraManager
            var isSupportCameraId = false
            mSupportCameraIds = mCameraManager.cameraIdList
            KLog.d(TAG, "supportCameraId:${mSupportCameraIds.contentToString()}")
            for (cameraId in mSupportCameraIds) {
                if (TextUtils.equals(cameraId, mCameraId)) {
                    isSupportCameraId = true
                    break
                }
            }
            if (!isSupportCameraId) break
            mCharacteristics = mCameraManager.getCameraCharacteristics(mCameraId)
            val capabilities =
                mCharacteristics.get(CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES)
            var isBackWardCompatible = false
            for (capability in capabilities!!) {
                if (capability == CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES_BACKWARD_COMPATIBLE) {
                    isBackWardCompatible = true
                    break
                }
            }
            if (!isBackWardCompatible) break
            val config =
                mCharacteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP) ?: break
            mSupportPreviewSize = listOf(*config.getOutputSizes(SurfaceTexture::class.java))
            KLog.d(TAG, "support size:${mSupportPreviewSize}")
            mSupportCaptureSize = listOf(*config.getOutputSizes(ImageFormat.YUV_420_888))

            val display = mBinding.cameraRecordView.display
            mPreviewSize =
                getPreviewOutputSize(display, mCharacteristics, SurfaceTexture::class.java, null)
            mCaptureSize = getPreviewOutputSize(
                display,
                mCharacteristics,
                SurfaceTexture::class.java,
                ImageFormat.YUV_420_888
            )
            mSensorOrientation = mCharacteristics.get(CameraCharacteristics.SENSOR_ORIENTATION)
            mCameraRecordClient.setTransformMatrix(0f, 0f, 1.0f, 1.0f, mSensorOrientation!!, 0)

            KLog.d(
                TAG, String.format(
                    "mPreviewSize=[%s,%s]  mSensorOrientation=%s",
                    mPreviewSize.width,
                    mPreviewSize.height,
                    mSensorOrientation.toString()
                )
            )
            result = true;
        } while (false)

        cont.resume(result)
    }

    @SuppressLint("MissingPermission")
    private suspend fun openCamera(): CameraDevice = suspendCancellableCoroutine { cont ->
        mPreviewThread.start()
        mCameraHandler = Handler(mPreviewThread.looper)

        if (mPreviewImageReader == null) {
            mPreviewImageReader = ImageReader.newInstance(
                mPreviewSize.width,
                mPreviewSize.height,
                ImageFormat.YUV_420_888,
                2
            )
            mPreviewImageReader?.setOnImageAvailableListener(
                mPreviewImageAvailableListener,
                mCameraHandler
            )
            mPreviewSurface = mPreviewImageReader!!.surface
        }

        if (mCaptureImageReader == null) {
            mCaptureImageReader = ImageReader.newInstance(
                mCaptureSize.width,
                mCaptureSize.height,
                ImageFormat.YUV_420_888,
                2
            )
            mCaptureImageReader?.setOnImageAvailableListener(
                mCaptureImageAvailableListener,
                mCameraHandler
            )
            mCaptureSurface = mCaptureImageReader!!.surface
        }

        mCameraManager.openCamera(mCameraId, object : CameraDevice.StateCallback() {
            override fun onOpened(camera: CameraDevice) {
                cont.resume(camera)
            }

            override fun onDisconnected(camera: CameraDevice) {
                camera.close()
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
                val exc = RuntimeException("Camera $mCameraId error: ($error) $msg")
                KLog.e(TAG, exc)
                if (cont.isActive) cont.resumeWithException(exc)
            }

        }, mCameraHandler)
    }

    private suspend fun createCaptureSession(): CameraCaptureSession =
        suspendCancellableCoroutine { cont ->
            val targets = listOf(mPreviewSurface)
            mCameraDevice?.createCaptureSession(
                targets,
                object : CameraCaptureSession.StateCallback() {
                    override fun onConfigured(session: CameraCaptureSession) {
                        cont.resume(session)
                    }

                    override fun onConfigureFailed(session: CameraCaptureSession) {
                        val exc = RuntimeException("Camera $mCameraId session configuration failed")
                        KLog.e(TAG, exc)
                        cont.resumeWithException(exc)
                    }

                },
                mCameraHandler
            )
        }

    private fun createPreviewRequest() {
        val builder = mCameraDevice?.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW)
        builder?.addTarget(mPreviewSurface!!)
        mPreviewRequest = builder?.build()
        mCaptureSession?.setRepeatingRequest(mPreviewRequest!!, null, mCameraHandler)
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        mCameraRecordClient.initRecorder(requireContext())
        mCameraRecordClient.onSurfaceCreated(holder.surface)

        mBinding.cameraRecordView.post {
            initializeEnv()
        }
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        mCameraRecordClient.onSurfaceChanged(width, height)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        closeCamera()
        mCameraRecordClient.onSurfaceDestroyed()
    }

    override fun onPreviewFrame(
        data: ByteArray?,
        width: Int,
        height: Int,
        format: Int,
        timestamp: Long
    ) {

    }

    override fun onCaptureFrame(
        data: ByteArray?,
        width: Int,
        height: Int,
        format: Int,
        timestamp: Long
    ) {
    }

    private val mPreviewImageAvailableListener =
        OnImageAvailableListener { reader ->
            val image = reader.acquireLatestImage()
            val data = ImageUtils.convert_YUV420_888_To_YUV420I_Data(image)
            mCameraRecordClient.inputVideoFrame(
                data,
                image.width,
                image.height,
                MediaContext.VIDEO_FRAME_FORMAT_I420,
                image.timestamp / 1000
            )
            image.close()
        }

    private val mCaptureImageAvailableListener = OnImageAvailableListener { }

    private fun closeCamera() {
        mCaptureSession?.close()
        mCameraDevice?.close()
        mPreviewImageReader?.close()
        mCaptureImageReader?.close()
        mPreviewThread.quitSafely()
    }

    override fun getLayoutId(): Int = R.layout.fragment_camera_record

    override fun onClick(v: View?) {
        when (v?.id) {
            R.id.btn_record -> {
                if (isRecording.get()) {
                    isRecording.set(false)
                    mCameraRecordClient.stopRecord()
                } else {
                    isRecording.set(true)
                    mCameraRecordClient.startRecord("${System.currentTimeMillis()}.mp4")
                }
            }
        }
    }
}