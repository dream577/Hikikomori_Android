package com.violet.hikikomori.view.media.camera

import android.content.Context
import android.content.Intent
import android.view.View
import com.violet.hikikomori.R
import com.violet.hikikomori.databinding.ActivityCameraBinding
import com.violet.hikikomori.view.base.BaseBindingActivity
import com.violet.libmedia.recoder.video.VioletCameraRecorder

class CameraActivity : BaseBindingActivity<ActivityCameraBinding>() {

    companion object {
        @JvmStatic
        fun launch(context: Context) {
            val intent = Intent(context, CameraActivity::class.java)
            context.startActivity(intent)
        }
    }

    override fun onClick(v: View?) {

    }

    override fun getLayoutId(): Int = R.layout.activity_camera
}