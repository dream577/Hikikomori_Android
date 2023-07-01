package com.violet.libmedia.codec.encoder

import com.violet.libmedia.model.MediaFrame
import com.violet.libmedia.util.RecycledPool.Element

interface Encoder {

    fun startEncoder()

    fun configCodec()

    fun inputOneFrame(frame: MediaFrame)

    fun outputOneFrame(): Element<MediaFrame>

    fun release()
}