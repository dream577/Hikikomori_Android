package com.violet.libmedia.render.imagerender

import android.view.Surface
import com.violet.libmedia.model.MediaFrame

interface GLRender {
    fun onSurfaceCreated(surface: Surface)

    fun onSurfaceChanged(width: Int, height: Int)

    fun onDrawFrame(frame: MediaFrame)

    fun onSurfaceDestroyed(surface: Surface)
}