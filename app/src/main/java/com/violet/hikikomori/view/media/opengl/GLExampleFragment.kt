package com.violet.hikikomori.view.media.opengl

import android.os.Bundle
import android.view.LayoutInflater
import android.view.SurfaceHolder
import android.view.View
import android.view.ViewGroup
import com.violet.hikikomori.R
import com.violet.hikikomori.databinding.FragmentGlExampleBinding
import com.violet.hikikomori.view.base.BaseBindingFragment
import com.violet.libmedia.MediaClient

class GLExampleFragment : BaseBindingFragment<FragmentGlExampleBinding>(), SurfaceHolder.Callback {

    companion object {
        const val TAG = "GLExampleFragment"
    }

    private lateinit var player: MediaClient

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        player = MediaClient()
        val view = super.onCreateView(inflater, container, savedInstanceState)
        mBinding.exampleSurfaceView.holder.addCallback(this)
        return view
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        player.onSurfaceCreated(holder.surface)
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        player.onSurfaceChanged(width, height)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        player.onSurfaceDestroyed(holder.surface)
    }

    override fun getLayoutId(): Int = R.layout.fragment_gl_example

    override fun onResume() {
        super.onResume()
        player.play("/storage/emulated/0/视频/[Airota][Fate stay night Heaven's Feel III.spring song][Movie][BDRip 1080p AVC AAC][CHS].mp4")
    }

    override fun onPause() {
        super.onPause()
        player.destroy()
    }
}