package com.violet.hikikomori.view.media.camera

import android.content.Context
import android.content.Intent
import android.view.View
import com.violet.hikikomori.R
import com.violet.hikikomori.databinding.ActivityCameraBinding
import com.violet.hikikomori.view.base.BaseBindingActivity

class CameraActivity : BaseBindingActivity<ActivityCameraBinding>() {

    companion object {
        /** Combination of all flags required to put activity into immersive mode */
        const val FLAGS_FULLSCREEN =
            View.SYSTEM_UI_FLAG_LOW_PROFILE or
                    View.SYSTEM_UI_FLAG_FULLSCREEN or
                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE or
                    View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY

        /** Milliseconds used for UI animations */
        const val ANIMATION_FAST_MILLIS = 50L
        const val ANIMATION_SLOW_MILLIS = 100L
        private const val IMMERSIVE_FLAG_TIMEOUT = 500L

        @JvmStatic
        fun launch(context: Context) {
            val intent = Intent(context, CameraActivity::class.java)
            context.startActivity(intent)
        }
    }

    override fun initView() {
        super.initView()
    }

    override fun onClick(v: View?) {

    }

    override fun getLayoutId(): Int = R.layout.activity_camera
}