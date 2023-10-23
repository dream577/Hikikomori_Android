package com.violet.libmedia.model

import java.nio.ByteBuffer

class MediaFrame(
        var dts: Long,
        var pts: Long,
        var buffer: ByteBuffer,
        var planeSize: IntArray,

    // video
        var isKeyFrame: Boolean,
        var width: Int,
        var height: Int,
        var format: AVPixelFormat,

    // audio
        var channels: Int,
        var sampleRate: Int,
        var bitRate: Int,
        var audioFormat: Int,

        var isVideo: Boolean
) {
    fun isVideoFrame(): Boolean = isVideo
}

enum class AVPixelFormat(val format: Int) {
    AV_PIX_FMT_NONE(-1),
    AV_PIX_FMT_YUV420P(0),
    AV_PIX_FMT_NV12(23),
    AV_PIX_FMT_NV21(24),
    AV_PIX_FMT_RGBA(26),
    IMAGE_FORMAT_YV21(0x04),
    IMAGE_FORMAT_YV12(0x05), ;
}

object MediaEvent {
    const val EVENT_DURATION = 0
    const val EVENT_SHOW_LOADING = 1
    const val EVENT_HIDE_LOADING = 2
    const val EVENT_PLAYING = 3
    const val EVENT_EXCEPTION = 4
    const val EVENT_PAUSED = 5
    const val EVENT_SEEK_FINISH = 6
}
