package com.violet.libmedia.model

import java.nio.ByteBuffer

class VideoDecoderConfig(
    var videoMineType: String,
    var width: Int,
    var height: Int,
) {
    fun isConfigChanged(newConfig: VideoDecoderConfig): Boolean {
        return this.videoMineType == newConfig.videoMineType
                && this.width == newConfig.width
                && this.height == newConfig.height
    }
}

class AudioDecoderConfig(
    var audioMineType: String,
    var sampleRate: Int,
    var audioChannels: Int,
    var duration: Long
) {
    fun isConfigChanged(newConfig: AudioDecoderConfig): Boolean {
        return this.audioMineType == newConfig.audioMineType
                && this.sampleRate == newConfig.sampleRate
                && this.audioChannels == newConfig.audioChannels
                && this.duration == newConfig.duration
    }
}

open class Frame(
    var dts: Long,
    var pts: Long,
    var format: Int,
    var buffer: ByteBuffer
) {
    override fun toString(): String {
        return "Frame(dts=$dts, pts=$pts, format=$format, buffer=$buffer)"
    }
}

class ImageFrame(
    dts: Long,
    pts: Long,
    format: Int,
    buffer: ByteBuffer,
    var isKeyFrame: Boolean,
    var width: Int,
    var height: Int,
    var planeSize: IntArray
) : Frame(dts, pts, format, buffer) {
    constructor(size: Int) : this(
        0, 0, 0, ByteBuffer.allocate(size), false,
        0, 0, IntArray(0)
    )

    constructor(buffer: ByteBuffer) : this(
        0, 0, 0, buffer, false,
        0, 0, IntArray(0)
    )

    override fun toString(): String {
        return "ImageFrame(dts=$dts, pts=$pts, format=$format, isKeyFrame=$isKeyFrame, width=$width, height=$height})"
    }

}

class AudioFrame(
    dts: Long,
    pts: Long,
    format: Int,
    buffer: ByteBuffer,
    var size: Int,
    var channels: Int,
    var sampleRate: Int
) : Frame(dts, pts, format, buffer) {
    constructor(size: Int) : this(
        0, 0, 0, ByteBuffer.allocate(size),
        0, 0, 0
    )

    constructor(buffer: ByteBuffer) : this(
        0, 0, 0, buffer,
        0, 0, 0
    )

    override fun toString(): String {
        return "AudioFrame(dts=$dts, pts=$pts, format=$format, size=$size, channels=$channels, sampleRate=$sampleRate)"
    }


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
