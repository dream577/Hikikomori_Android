package com.violet.libmedia.decoder

import android.media.MediaCodec
import android.media.MediaExtractor
import android.media.MediaFormat
import com.violet.libmedia.util.VThread

class HardwareDecoder(name: String) : VThread(name) {
    companion object {
        const val TAG = "HardwareVideoDecoder"
    }

    private val mediaExtractor: MediaExtractor = MediaExtractor()
    private lateinit var mediaCodec: MediaCodec

    constructor() : this("")

    fun init(path: String) {
        do {
            mediaExtractor.setDataSource(path)

            // 获取解封装后流的数目
            val count = mediaExtractor.trackCount

            var videoIndex: Int
            var format: MediaFormat
            var mine: String? = null

            for (index in 0..count) {
                format = mediaExtractor.getTrackFormat(index)
                mine = format.getString(MediaFormat.KEY_MIME)
                if (mine?.startsWith("video/") == true) {
                    videoIndex = index
                }
            }

//            if (mine == null) {
//                break
//            }
//            mediaCodec = MediaCodec.createDecoderByType(mine)
//            mediaExtractor.selectTrack(videoIndex)
//            mediaExtractor.readSampleData()

        } while (false)
    }
}