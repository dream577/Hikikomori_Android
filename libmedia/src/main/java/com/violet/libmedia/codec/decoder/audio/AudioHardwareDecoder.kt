package com.violet.libmedia.codec.decoder.audio

import android.media.MediaFormat
import com.violet.libbasetools.util.KLog
import com.violet.libmedia.codec.decoder.HardwareDecoder
import com.violet.libmedia.demuxer.Demuxer
import com.violet.libmedia.demuxer.MediaDemuxer
import com.violet.libmedia.model.ImageFormat
import com.violet.libmedia.model.MediaFrame
import com.violet.libmedia.util.RecycledPool
import java.nio.ByteBuffer

class AudioHardwareDecoder : HardwareDecoder() {

    companion object {
        const val TAG = "AudioHardwareDecoder"
        private const val AAC_PACKET_SIZE = 4096
    }

    override fun createDemuxer(): Demuxer {
        return MediaDemuxer(false)
    }

    override fun onOutputFormatChanged(format: MediaFormat, pool: RecycledPool<MediaFrame>) {
//        val bitRate = format.getInteger(MediaFormat.KEY_BIT_RATE)
        val sampleRate = format.getInteger(MediaFormat.KEY_SAMPLE_RATE)
        val channels = format.getInteger(MediaFormat.KEY_CHANNEL_COUNT)

        KLog.d(TAG, "AudioOutputFormat[$format")

        val planeSize = intArrayOf(AAC_PACKET_SIZE, 0, 0)

        pool.initRecycledPool {
            MediaFrame(
                0, 0, ByteBuffer.allocateDirect(AAC_PACKET_SIZE),
                planeSize, false, 0, 0, ImageFormat.IMAGE_FORMAT_NONE,
                channels, sampleRate, 0, 0, false
            )
        }
    }
}