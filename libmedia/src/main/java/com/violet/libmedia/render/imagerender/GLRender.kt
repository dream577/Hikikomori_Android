package com.violet.libmedia.render.imagerender

import android.view.Surface

interface GLRender {
    fun onSurfaceCreated(surface: Surface)

    fun onSurfaceChanged(width: Int, height: Int)

    fun onDrawFrame()

    fun onSurfaceDestroyed(surface: Surface)
}