package com.violet.hikikomori.view.media.opengl

import android.os.Bundle
import android.view.LayoutInflater
import android.view.SurfaceHolder
import android.view.View
import android.view.ViewGroup
import com.violet.hikikomori.R
import com.violet.hikikomori.databinding.FragmentGlExampleBinding
import com.violet.hikikomori.view.base.BaseBindingFragment
import com.violet.libmedia.MediaPlayer
import com.violet.libmedia.render.imagerender.GLRenderWindow

class GLExampleFragment : BaseBindingFragment<FragmentGlExampleBinding>(), SurfaceHolder.Callback {

    companion object {
        const val TAG = "GLExampleFragment"
    }

    private lateinit var window: GLRenderWindow

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        window = GLRenderWindow()
        val view = super.onCreateView(inflater, container, savedInstanceState)
        mBinding.exampleSurfaceView.holder.addCallback(this)
        return view
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
//        window.start()
//        window.onSurfaceCreated(holder.surface)
        val player = MediaPlayer(holder.surface)
        player.play("/storage/emulated/0/视频/[Airota][Fate stay night Heaven's Feel III.spring song][Movie][BDRip 1080p AVC AAC][CHS].mp4")
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
//        window.onSurfaceChanged(width, height)
    }


    override fun surfaceDestroyed(holder: SurfaceHolder) {
//        window.onSurfaceDestroyed(holder.surface)
    }

    override fun getLayoutId(): Int = R.layout.fragment_gl_example
}