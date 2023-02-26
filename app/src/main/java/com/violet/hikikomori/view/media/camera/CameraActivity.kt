package com.violet.hikikomori.view.media.camera

import android.content.Context
import android.content.Intent
import android.view.View
import android.view.WindowManager
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

    override fun initView() {
        super.initView()
        updateMediaUI()
    }

    override fun onClick(v: View?) {

    }

    private fun updateMediaUI() {
        window.addFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS)
        window.addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN)
        window.decorView.systemUiVisibility = (View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LOW_PROFILE
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION     // 隐藏导航栏
                or View.SYSTEM_UI_FLAG_FULLSCREEN          // 隐藏状态栏
                or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY)
    }

    override fun getLayoutId(): Int = R.layout.activity_camera
}