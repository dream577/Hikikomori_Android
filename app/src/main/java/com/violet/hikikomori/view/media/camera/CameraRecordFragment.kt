package com.violet.hikikomori.view.media.camera

import com.violet.hikikomori.R
import com.violet.hikikomori.databinding.FragmentCameraRecordBinding
import com.violet.hikikomori.view.base.BaseBindingFragment
import com.violet.libmedia.recoder.video.VioletCameraRecorder


class CameraRecordFragment : BaseBindingFragment<FragmentCameraRecordBinding>() {
    private lateinit var mCameraRecorder: VioletCameraRecorder

    override fun initView() {
        super.initView()
        mCameraRecorder = VioletCameraRecorder(mBinding.cameraRecordView)
    }

    override fun getLayoutId(): Int = R.layout.fragment_camera_record
}