package com.violet.hikikomori.view.media.camera

import android.annotation.SuppressLint
import android.content.Context
import android.hardware.camera2.CameraDevice
import android.hardware.camera2.CameraManager
import android.os.Handler
import android.os.HandlerThread
import androidx.lifecycle.lifecycleScope
import androidx.navigation.fragment.navArgs
import com.violet.hikikomori.R
import com.violet.hikikomori.databinding.FragmentCameraRecordBinding
import com.violet.hikikomori.view.base.BaseBindingFragment
import com.violet.libbasetools.util.KLog
import kotlinx.coroutines.launch
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException

class CameraRecordFragment : BaseBindingFragment<FragmentCameraRecordBinding>() {

    private val arguments: CameraRecordFragmentArgs by navArgs()


    private val mCameraManager: CameraManager by lazy {
        requireContext().getSystemService(Context.CAMERA_SERVICE) as CameraManager
    }

    private lateinit var mCameraDevice: CameraDevice

    /**
     * 预览线程
     */
    private val mPreviewThread = HandlerThread("PreviewThread")

    /**
     * 预览线程的Handler
     */
    private val mPreviewHandler = Handler(mPreviewThread.looper)

    /**
     * 录像线程
     */
    private val mRecordThread = HandlerThread("RecordThread")

    /**
     * 录像线程的Handler
     */
    private val mRecordHandler = Handler(mRecordThread.looper)

    override fun initData() {
        super.initData()
    }

    override fun initView() {
        super.initView()
    }

    private fun initializeCamera() = lifecycleScope.launch {
        // 1. 打开Camera
        mCameraDevice = openCamera(mCameraManager, arguments.cameraId)

        val previewSurface = mBinding.cameraRecordView.holder.surface


    }

    @SuppressLint("MissingPermission")
    private suspend fun openCamera(mCameraManager: CameraManager, mCameraId: String): CameraDevice =
        suspendCancellableCoroutine { cont ->
            mCameraManager.openCamera(mCameraId, object : CameraDevice.StateCallback() {
                override fun onOpened(camera: CameraDevice) = cont.resume(camera)

                override fun onDisconnected(camera: CameraDevice) {
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
                    val exc = RuntimeException("Camera $mCameraId error: ($error) $msg")
                    KLog.e(TAG, exc.message!!, exc)
                    if (cont.isActive) cont.resumeWithException(exc)
                }
            }, mPreviewHandler)
        }

    override fun getLayoutId(): Int = R.layout.fragment_camera_record

}