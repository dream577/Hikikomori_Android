package com.violet.libmedia.demuxer

import android.media.MediaExtractor
import android.media.MediaFormat
import com.violet.libbasetools.util.KLog
import java.nio.ByteBuffer

class MediaDemuxer(private val isVideo: Boolean) : Demuxer {
    companion object {
        const val TAG = "MediaDemuxer"
    }

    private val mediaExtractor: MediaExtractor = MediaExtractor()
    private var mediaFormat: MediaFormat? = null

    private var currentSize: Int = 0
    private var currentPts: Long = 0

    override fun configDemuxer(path: String): Boolean {
        var result = false
        mediaExtractor.setDataSource(path)
        // 获取解封装后流的数目
        val count = mediaExtractor.trackCount

        for (index in 0 until count) {
            mediaFormat = mediaExtractor.getTrackFormat(index)
            val mine = mediaFormat?.getString(MediaFormat.KEY_MIME)
            if (isVideo && mine?.startsWith("video/") == true || !isVideo && mine?.startsWith("audio/") == true) {
                mediaExtractor.selectTrack(index)
                KLog.d(TAG, mediaFormat!!)
                result = true
                break
            }
        }
        return result
    }

    override fun getMediaFormat(): MediaFormat {
        if (mediaFormat == null) throw NullPointerException("mediaFormat is null")
        return mediaFormat!!
    }

    override fun readSampleData(buffer: ByteBuffer): Int {
        currentSize = mediaExtractor.readSampleData(buffer, 0)
        currentPts = mediaExtractor.sampleTime

        mediaExtractor.advance()
        // 读取数据
        return currentSize
    }

    override fun getSampleTime(): Long {
        return currentPts
    }


    override fun release() {
        mediaExtractor.release()
    }


}