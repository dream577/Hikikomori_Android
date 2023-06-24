package com.violet.libmedia.render

import com.violet.libmedia.model.MediaFrame
import com.violet.libmedia.util.RecycledPool.Element

interface RenderCallback {
    fun requestRenderFrame(isVideo: Boolean): Element<MediaFrame>?
}