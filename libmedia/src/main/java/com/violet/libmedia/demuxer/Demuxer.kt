package com.violet.libmedia.demuxer

import android.media.MediaFormat
import java.nio.ByteBuffer

interface Demuxer {

    fun configDemuxer(path: String): Boolean

    fun getMediaFormat(): MediaFormat

    fun readSampleData(buffer: ByteBuffer): Int

    fun getSampleTime(): Long

    fun release()
}