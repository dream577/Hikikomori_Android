package com.violet.libmedia.render.audiorender

import android.media.AudioTrack

class AudioRender {
    private lateinit var audioTrack: AudioTrack

    fun init(streamType: Int, sampleRateInHz: Int, channelConfig: Int, audioFormat: Int) {
        val minBufferSize = AudioTrack.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat)
        audioTrack = AudioTrack(
            streamType, sampleRateInHz, channelConfig,
            audioFormat, minBufferSize, AudioTrack.MODE_STREAM
        )


    }
}