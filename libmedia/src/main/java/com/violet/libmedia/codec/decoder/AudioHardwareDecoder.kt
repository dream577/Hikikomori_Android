package com.violet.libmedia.codec.decoder

import android.media.MediaFormat
import com.violet.libmedia.model.AudioDecoderConfig

class AudioHardwareDecoder : HardwareDecoder() {

    companion object {
        const val TAG = "AudioHardwareDecoder"
    }

    var config: AudioDecoderConfig? = null

    fun changeCodecConfig(config: AudioDecoderConfig) {
        this.config = config
    }

    override fun getMediaFormat(): MediaFormat {
        var mime = ""
        var sampleRate = 0
        var audioChannels = 0
        config?.let {
            mime = it.audioMineType
            sampleRate = it.sampleRate
            audioChannels = it.audioChannels
        }
        return MediaFormat.createAudioFormat(mime, sampleRate, audioChannels)
    }

}