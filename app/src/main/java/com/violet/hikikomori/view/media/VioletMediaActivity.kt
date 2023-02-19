package com.violet.hikikomori.view.media

import android.content.Context
import android.content.Intent
import android.view.View
import com.violet.hikikomori.R
import com.violet.hikikomori.databinding.ActivityVioletMediaBinding
import com.violet.hikikomori.view.base.BaseBindingActivity

class VioletMediaActivity : BaseBindingActivity<ActivityVioletMediaBinding>() {

    companion object {
        @JvmStatic
        fun launch(context: Context) {
            val intent = Intent(context, VioletMediaActivity::class.java)
            context.startActivity(intent)
        }
    }

    override fun initView() {
        super.initView()
    }

    override fun onClick(v: View?) {

    }

    override fun getLayoutId(): Int = R.layout.activity_violet_media
}