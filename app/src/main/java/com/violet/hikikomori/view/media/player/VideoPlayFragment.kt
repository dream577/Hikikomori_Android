package com.violet.hikikomori.view.media.player

import android.Manifest
import android.content.res.Configuration
import android.os.Bundle
import android.view.SurfaceHolder
import android.view.View
import android.view.WindowManager
import com.violet.hikikomori.R
import com.violet.hikikomori.databinding.FragmentVideoPlayBinding
import com.violet.hikikomori.view.base.BaseBindingFragment
import com.violet.libbasetools.util.KLog
import com.violet.libmedia.VioletVideoClient
import pub.devrel.easypermissions.EasyPermissions

class VideoPlayFragment : BaseBindingFragment<FragmentVideoPlayBinding>(), SurfaceHolder.Callback,
    View.OnClickListener {

    companion object {
        const val TAG = "VideoPlayFragment"
        const val VIDEO_PATH = "VIDEO_PATH"
    }

    private lateinit var path: String
    private var client: VioletVideoClient? = null
    private val readPermission = Manifest.permission.READ_EXTERNAL_STORAGE

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        arguments?.let {
            path = it.getString(VIDEO_PATH, "")
        }
    }

    override fun initData() {
        super.initData()
    }

    override fun initView() {
        super.initView()
        mBinding.surface.holder.addCallback(this)
        mBinding.clickListener = this
    }

    override fun subscribeToViewModel() {
        super.subscribeToViewModel()
    }

    override fun onResume() {
        super.onResume()
        updateMediaUI()
        if (!EasyPermissions.hasPermissions(requireContext(), readPermission)) {
            EasyPermissions.requestPermissions(this, "读取存储", 101, readPermission)
        } else {
        }
    }

    override fun onStop() {
        super.onStop()
        client?.stop()
    }

    override fun getLayoutId(): Int = R.layout.fragment_video_play

    override fun surfaceCreated(holder: SurfaceHolder) {
        KLog.d(TAG, "surfaceCreated time=" + System.currentTimeMillis())
        client = VioletVideoClient()
        client?.init(path)
        client?.onSurfaceCreated(holder.surface)
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        KLog.d(TAG, "surfaceChanged time=" + System.currentTimeMillis())
        client?.onSurfaceChanged(width, height)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        client?.onSurfaceDestroyed(holder.surface)
    }

    override fun onClick(v: View?) {
        when (v?.id) {
            R.id.play -> client?.play()
            R.id.pause -> client?.pause()
            R.id.resume -> client?.resume()
            R.id.stop -> {
                client?.stop()
                client = null
            }
            R.id.seek -> client?.seekToPosition(50f)
        }
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)
    }

    /**
     * 更新视频播放窗口布局：
     *     横屏时：隐藏状态栏和导航栏
     *     竖屏时：显示状态栏和导航栏
     */
    private fun updateMediaUI() {
        val window = requireActivity().window
        window.clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN)                  // 显示状态栏
        window.clearFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS)
        window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS)          // 取消状态栏透明
        window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS)  // 添加Flag把状态栏设为可绘制模式
        window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_VISIBLE
        window.statusBarColor = resources.getColor(R.color.black)                      // 设置状态栏颜色
    }
}