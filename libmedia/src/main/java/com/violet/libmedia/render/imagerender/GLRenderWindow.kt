package com.violet.libmedia.render.imagerender

import android.opengl.GLES30
import android.view.Surface
import com.violet.libmedia.core.EglCore
import com.violet.libmedia.model.RenderMessage
import com.violet.libmedia.util.VThread

class GLRenderWindow(name: String) : VThread(name), GLRender {
    companion object {
        const val TAG = "GLRenderWindow"
    }

    private val mCore: EglCore = EglCore()
    private lateinit var mSurface: Surface
    private val mRenders: List<GLRender>

    private var windowWidth = 0
    private var windowHeight = 0
    private var imageWidth = 0
    private var imageHeight = 0

    constructor() : this(TAG)

    init {
        mRenders = arrayListOf(
            TriangleSampleRender()
        )
    }

    override fun onSurfaceCreated(surface: Surface) {
        this.mSurface = surface
        putMessage(RenderMessage.ON_SURFACE_CREATED)
        startLoop(RenderMessage.ON_DRAW_FRAME)
    }

    override fun onSurfaceChanged(width: Int, height: Int) {
        this.windowWidth = width
        this.windowHeight = height
        putMessage(RenderMessage.ON_SURFACE_CHANGED)
    }

    override fun onDrawFrame() {
        for (render in mRenders) {
            render.onDrawFrame()
        }
        mCore.swapBuffers()
    }

    override fun onSurfaceDestroyed(surface: Surface) {
        putMessage(RenderMessage.ON_SURFACE_DESTROYED)
        quit()
    }

    private fun onSurfaceCreated() {
        mCore.init(mSurface)
        for (render in mRenders) {
            render.onSurfaceCreated(mSurface)
        }
    }

    private fun onSurfaceChanged() {
        val x: Int
        val y: Int
        val width: Int
        val height: Int
        if (imageWidth <= 0) imageWidth = windowWidth
        if (imageHeight <= 0) imageHeight = windowHeight
        if (imageHeight.toFloat() / imageWidth < windowHeight.toFloat() / windowWidth) {
            width = windowWidth
            height = windowWidth * imageHeight / imageWidth
        } else {
            width = windowHeight * imageWidth / imageHeight
            height = windowHeight
        }

        x = (windowWidth - width) / 2
        y = (windowHeight - height) / 2

        GLES30.glViewport(x, y, width, height)
        GLES30.glClearColor(1.0f, 1.0f, 1.0f, 1.0f)

        for (render in mRenders) {
            render.onSurfaceChanged(windowWidth, windowHeight)
        }
    }

    private fun onSurfaceDestroyed() {
        for (render in mRenders) {
            render.onSurfaceDestroyed(mSurface)
        }
        mCore.unInit()
    }

    override fun handleMessage(msg: Int) {
        super.handleMessage(msg)
        when (msg) {
            RenderMessage.ON_SURFACE_CREATED -> {
                onSurfaceCreated()
            }
            RenderMessage.ON_SURFACE_CHANGED -> {
                onSurfaceChanged()
            }
            RenderMessage.ON_DRAW_FRAME -> {
                onDrawFrame()
            }
            RenderMessage.ON_SURFACE_DESTROYED -> {
                onSurfaceDestroyed()
            }
        }
    }

}