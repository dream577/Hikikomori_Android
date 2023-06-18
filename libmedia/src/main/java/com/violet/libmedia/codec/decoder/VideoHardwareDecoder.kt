package com.violet.libmedia.codec.decoder

import android.media.MediaFormat
import com.violet.libmedia.model.VideoDecoderConfig

class VideoHardwareDecoder : HardwareDecoder() {

    companion object {
        const val TAG = "VideoHardwareDecoder"
    }

    var config: VideoDecoderConfig? = null

    fun changeCodecConfig(config: VideoDecoderConfig) {
        this.config = config
    }

    override fun getMediaFormat(): MediaFormat {
        var mime = ""
        var width = 0
        var height = 0

        config?.let {
            width = it.width
            height = it.height
            mime = it.videoMineType
        }

        return MediaFormat.createVideoFormat(mime, width, height)
    }

    override fun release() {

    }
}