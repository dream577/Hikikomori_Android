package com.violet.hikikomori.view.media.player

import android.content.Context
import android.content.Intent
import android.os.Bundle
import com.violet.hikikomori.view.base.FragmentPagerActivity
import com.violet.libbasetools.model.Constant
import com.violet.libbasetools.util.tag

class VideoPlayActivity : FragmentPagerActivity() {

    companion object {
        fun launch(mContext: Context, mPath: String) {
            val intent = Intent(mContext, VideoPlayActivity::class.java)
            intent.putExtra(Constant.VIDEO_PATH, mPath)
            mContext.startActivity(intent)
        }
    }

    private lateinit var path: String

    override fun initData() {
        super.initData()
        path = intent.getStringExtra(Constant.VIDEO_PATH) ?: ""
    }

    override fun registerFragment() {
        registerFragment(VideoPlayFragment.tag, VideoPlayFragment::class.java)
    }

    override fun initView() {
        super.initView()
        enterVideoPlayFragment()
    }

    private fun enterVideoPlayFragment() {
        val bundle = Bundle()
        bundle.putString(VideoPlayFragment.VIDEO_PATH, path)
        pagerManager.enterFragment(VideoPlayFragment.tag, bundle)
    }
}