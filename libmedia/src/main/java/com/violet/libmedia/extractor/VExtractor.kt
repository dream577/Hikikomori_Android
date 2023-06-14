package com.violet.libmedia.extractor

import android.media.MediaExtractor
import android.media.MediaFormat

class VExtractor {
    private val mediaExtractor: MediaExtractor = MediaExtractor()
    private var videoIndex: Int = -1
    private var audioIndex: Int = -1

    fun init(path: String) {
        mediaExtractor.setDataSource(path)

        // 获取解封装后流的数目
        val count = mediaExtractor.trackCount

        for (index in 0..count) {
            val format = mediaExtractor.getTrackFormat(index)
            val mine = format.getString(MediaFormat.KEY_MIME)
            if (mine?.startsWith("video/") == true && videoIndex != -1) {
                videoIndex = index
            }
            if (mine?.startsWith("audio/") == true) {
                audioIndex = index
            }
        }

    }
}