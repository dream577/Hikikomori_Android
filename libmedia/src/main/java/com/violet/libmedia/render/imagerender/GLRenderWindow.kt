package com.violet.libmedia.render.imagerender

import android.opengl.GLES30
import android.view.Surface
import com.violet.libmedia.core.EglCore
import com.violet.libmedia.util.VThread

class GLRenderWindow(name: String) : VThread(name), GLRender {
    companion object {
        const val TAG = "GLRenderWindow"

        const val ON_SURFACE_CREATED = 1
        const val ON_SURFACE_CHANGED = 2
        const val ON_DRAW_FRAME = 3
        const val ON_SURFACE_DESTROYED = 4
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
        putMessage(ON_SURFACE_CREATED)
        startLoop(ON_DRAW_FRAME)
    }

    override fun onSurfaceChanged(width: Int, height: Int) {
        this.windowWidth = width
        this.windowHeight = height
        putMessage(ON_SURFACE_CHANGED)
    }

    override fun onDrawFrame() {
        for (render in mRenders) {
            render.onDrawFrame()
        }
        mCore.swapBuffers()
    }

    override fun onSurfaceDestroyed(surface: Surface) {
        putMessage(ON_SURFACE_DESTROYED)
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
            ON_SURFACE_CREATED -> {
                onSurfaceCreated()
            }
            ON_SURFACE_CHANGED -> {
                onSurfaceChanged()
            }
            ON_DRAW_FRAME -> {
                onDrawFrame()
            }
            ON_SURFACE_DESTROYED -> {
                onSurfaceDestroyed()
            }
        }
    }

}