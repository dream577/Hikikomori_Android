package com.violet.hikikomori.view.media

import android.content.Context
import android.content.Intent
import android.os.Bundle
import android.view.View
import android.view.WindowManager
import com.violet.hikikomori.R
import com.violet.hikikomori.databinding.ActivityVideoPlayBinding
import com.violet.hikikomori.view.base.BaseBindingActivity
import com.violet.libbasetools.model.Constant

class VideoPlayActivity : BaseBindingActivity<ActivityVideoPlayBinding>() {

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

    override fun initView() {
        super.initView()
        updateMediaUI()
        addVideoPlayFragment()
    }

    override fun onClick(v: View?) {

    }

    override fun getLayoutId(): Int = R.layout.activity_video_play

    private fun addVideoPlayFragment() {
        val fragmentManager = supportFragmentManager
        val transaction = fragmentManager.beginTransaction()
        transaction.add(R.id.fragment_video, VideoPlayFragment.newInstance(path))
        transaction.commit()
    }

    /**
     * 更新视频播放窗口布局：
     *     横屏时：隐藏状态栏和导航栏
     *     竖屏时：显示状态栏和导航栏
     */
    private fun updateMediaUI() {
        window.clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN)                  // 显示状态栏
        window.clearFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS)
        window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS)          // 取消状态栏透明
        window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS)  // 添加Flag把状态栏设为可绘制模式
        window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_VISIBLE
        window.statusBarColor = resources.getColor(R.color.black)                      // 设置状态栏颜色
    }
}