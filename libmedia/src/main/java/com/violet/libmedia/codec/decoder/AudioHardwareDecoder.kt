package com.violet.libmedia.codec.decoder

import android.media.MediaFormat
import com.violet.libmedia.demuxer.Demuxer
import com.violet.libmedia.demuxer.MediaDemuxer
import com.violet.libmedia.model.MediaFrame
import com.violet.libmedia.util.RecycledPool

class AudioHardwareDecoder : HardwareDecoder() {

    companion object {
        const val TAG = "AudioHardwareDecoder"
    }

    override fun onOutputFormatChanged(format: MediaFormat, pool: RecycledPool<MediaFrame>) {
    }

    override fun prepareDemuxer(): Demuxer {
        val demuxer = MediaDemuxer(false)
        demuxer.configDemuxer("/storage/emulated/0/视频/[Airota][Fate stay night Heaven's Feel III.spring song][Movie][BDRip 1080p AVC AAC][CHS].mp4")
        return demuxer
    }

}