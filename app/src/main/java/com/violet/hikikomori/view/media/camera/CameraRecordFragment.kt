package com.violet.hikikomori.view.media.camera

import com.violet.hikikomori.R
import com.violet.hikikomori.databinding.FragmentCameraRecordBinding
import com.violet.hikikomori.view.base.BaseBindingFragment
import com.violet.libmedia.opengles.OpenGLSampleWrapper
import com.violet.libmedia.recoder.video.VioletCameraRecorder


class CameraRecordFragment : BaseBindingFragment<FragmentCameraRecordBinding>() {
    companion object {
        const val TAG = "Camera.CameraRecordFragment"
    }

    //    private lateinit var mCameraRecorder: VioletCameraRecorder
    private lateinit var mOpenGLSampleWrapper: OpenGLSampleWrapper

    override fun initView() {
        super.initView()
//        mCameraRecorder = VioletCameraRecorder(mBinding.cameraRecordView)
        mOpenGLSampleWrapper = OpenGLSampleWrapper(mBinding.cameraRecordView)
    }

    override fun getLayoutId(): Int = R.layout.fragment_camera_record
}