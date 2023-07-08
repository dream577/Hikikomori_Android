package com.violet.libmedia.render.imagerender

import android.view.Surface
import com.violet.libmedia.model.MediaFrame

interface GLRender {
    fun onSurfaceCreated(surface: Surface)

    fun onSurfaceChanged(width: Int, height: Int)

    fun setTransformMatrix(
        translateX: Float, translateY: Float,
        scaleX: Float, scaleY: Float,
        degree: Int, mirror: Int
    )

    fun onDrawFrame(frame: MediaFrame)

    fun onSurfaceDestroyed(surface: Surface)
}