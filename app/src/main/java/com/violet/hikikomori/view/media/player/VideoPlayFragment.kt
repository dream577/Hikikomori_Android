package com.violet.hikikomori.view.media.player

import android.Manifest
import android.content.pm.ActivityInfo
import android.content.res.Configuration
import android.graphics.Color
import android.os.*
import android.util.DisplayMetrics
import android.view.*
import android.widget.SeekBar
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.lifecycle.ViewModelProvider
import com.violet.hikikomori.R
import com.violet.hikikomori.databinding.FragmentVideoPlayBinding
import com.violet.hikikomori.view.base.BaseBindingFragment
import com.violet.hikikomori.viewmodel.media.VideoPlayViewModel
import com.violet.libbasetools.util.KLog
import com.violet.libmedia.VioletVideoClient
import com.violet.libmedia.model.MediaEvent
import pub.devrel.easypermissions.EasyPermissions

class VideoPlayFragment : BaseBindingFragment<FragmentVideoPlayBinding>(), SurfaceHolder.Callback,
    View.OnClickListener, View.OnTouchListener, VioletVideoClient.MediaEventCallback {

    companion object {
        const val TAG = "VideoPlayFragment"
        const val VIDEO_PATH = "VIDEO_PATH"
        const val EVENT_VALUE = "VALUE"
    }

    private val mHandler: Handler = object : Handler(Looper.getMainLooper()) {
        override fun handleMessage(msg: Message) {
            val value = msg.data.getLong(EVENT_VALUE)
            when (msg.what) {
                MediaEvent.EVENT_DURATION -> {
                    mBinding.panelBottom.sbProgress.max = (value / 1000).toInt()
                }

                MediaEvent.EVENT_PLAYING -> {
                    if (!isSeeking) {
                        mBinding.panelBottom.sbProgress.progress = (value / 1000).toInt()
                    }
                }

                MediaEvent.EVENT_SEEK_FINISH -> {
                    isSeeking = false
                }
            }
        }
    }

    private lateinit var playVM: VideoPlayViewModel
    private lateinit var path: String
    private lateinit var client: VioletVideoClient

    private lateinit var mGestureDetector: GestureDetector

    private val readPermission = Manifest.permission.READ_EXTERNAL_STORAGE

    private var isPause = false
    private var isSeeking = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        arguments?.let {
            path = it.getString(VIDEO_PATH, "")
        }
    }

    override fun initData() {
        super.initData()
        playVM = ViewModelProvider(this)[VideoPlayViewModel::class.java]
    }

    override fun initView() {
        super.initView()

        toggleFullScreen(requireActivity().requestedOrientation == Configuration.ORIENTATION_LANDSCAPE)

        mBinding.apply {
            videoSurfaceView.apply {
                holder.addCallback(this@VideoPlayFragment)
                setOnTouchListener(this@VideoPlayFragment)
            }
            clickListener = this@VideoPlayFragment
            mBinding.panelBottom.sbProgress.keyProgressIncrement = 1
            mBinding.panelBottom.sbProgress.setOnSeekBarChangeListener(object :
                SeekBar.OnSeekBarChangeListener {
                override fun onProgressChanged(
                    seekBar: SeekBar?,
                    progress: Int,
                    fromUser: Boolean
                ) {
                }

                override fun onStartTrackingTouch(seekBar: SeekBar?) {
                    isSeeking = true
                }

                override fun onStopTrackingTouch(seekBar: SeekBar?) {
                    val progress = seekBar?.progress
                    KLog.d(TAG, "onStopTrackingTouch:${seekBar?.progress}")
                    progress?.let { client.seekToPosition(it.toFloat()) }
                }

            })
        }

        mGestureDetector = GestureDetector(requireContext(), mGestureListener)
        mGestureDetector.setOnDoubleTapListener(mGestureListener)
        mGestureDetector.setIsLongpressEnabled(false)

    }

    override fun subscribeToViewModel() {
        super.subscribeToViewModel()
        playVM.mFullScreenLiveData.observe(this) { isFullScreen ->
            requireActivity().requestedOrientation =
                if (isFullScreen) {
                    ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
                } else {
                    ActivityInfo.SCREEN_ORIENTATION_PORTRAIT
                }
        }
    }

    override fun onResume() {
        super.onResume()
        if (!EasyPermissions.hasPermissions(requireContext(), readPermission)) {
            EasyPermissions.requestPermissions(this, "读取存储", 101, readPermission)
        } else {
        }
    }

    override fun onStop() {
        super.onStop()
        client.stop()
    }

    override fun getLayoutId(): Int = R.layout.fragment_video_play

    override fun surfaceCreated(holder: SurfaceHolder) {
        KLog.d(TAG, "surfaceCreated time=" + System.currentTimeMillis())
        client = VioletVideoClient()
        client.apply {
            registerEventCallback(this@VideoPlayFragment)
            init(path)
            onSurfaceCreated(holder.surface)
            play()
        }
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        KLog.d(TAG, "surfaceChanged time=" + System.currentTimeMillis())
        client.onSurfaceChanged(width, height)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        client.onSurfaceDestroyed(holder.surface)
        client.stop()
    }

    override fun onClick(v: View?) {
        when (v?.id) {
            R.id.full_screen -> {
                playVM.toggleFullScreen()
            }
        }
    }

    override fun onTouch(v: View?, event: MotionEvent?): Boolean {
        if (event == null || v == null) return false
        when (v.id) {
            R.id.video_surface_view -> {
                mGestureDetector.onTouchEvent(event)
            }
        }
        return true
    }

    private val mGestureListener = object : GestureDetector.SimpleOnGestureListener() {
        override fun onFling(
            e1: MotionEvent,
            e2: MotionEvent,
            velocityX: Float,
            velocityY: Float
        ): Boolean {
            return super.onFling(e1, e2, velocityX, velocityY)
        }

        override fun onSingleTapUp(e: MotionEvent): Boolean {
            return super.onSingleTapUp(e)
        }

        override fun onDoubleTap(e: MotionEvent): Boolean {
            isPause = if (isPause) {
                client.resume()
                false
            } else {
                client.pause()
                true
            }
            return true
        }
    }

    override fun onReceiveEvent(messageType: Int, value: Long) {
        val msg = Message.obtain()
        msg.what = messageType
        val data = msg.data
        data.putLong(EVENT_VALUE, value)
        mHandler.sendMessage(msg)
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)
        val isLandScape = newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE
        toggleFullScreen(isLandScape)
    }

    /**
     * 横竖屏切换时更新视频播放窗口布局
     */
    private fun toggleFullScreen(isFullScreen: Boolean) {
        val window = requireActivity().window
        if (isFullScreen) {
            window.addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN)
            window.addFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS)
            if (Build.VERSION.SDK_INT > Build.VERSION_CODES.LOLLIPOP) {
                window.decorView.systemUiVisibility =
                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE or
                            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or
                            View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION or
                            View.SYSTEM_UI_FLAG_FULLSCREEN or
                            View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or
                            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                window.statusBarColor = Color.TRANSPARENT
                window.navigationBarColor = Color.TRANSPARENT
            } else {
                window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_FULLSCREEN
            }
            mBinding.videoSurfaceView.layoutParams.width =
                ConstraintLayout.LayoutParams.MATCH_PARENT
            mBinding.videoSurfaceView.layoutParams.height =
                ConstraintLayout.LayoutParams.MATCH_PARENT
        } else {
            window.clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN)   // 显示状态栏
            window.clearFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS)
            window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS)   // 取消状态栏透明
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS)  // 添加Flag把状态栏设为可绘制模式
            window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_VISIBLE
            window.statusBarColor = resources.getColor(R.color.black)     // 设置状态栏颜色

            val params = mBinding.videoSurfaceView.layoutParams as ConstraintLayout.LayoutParams
            val dm = DisplayMetrics()
            requireActivity().windowManager.defaultDisplay.getRealMetrics(dm)
            params.width = dm.widthPixels
            params.height = dm.widthPixels / 16 * 9
        }
    }

    override fun onBackPressed(): Boolean {
        if (playVM.isFullScreen.get()) {
            playVM.toggleFullScreen()
            return true
        }
        return false
    }
}