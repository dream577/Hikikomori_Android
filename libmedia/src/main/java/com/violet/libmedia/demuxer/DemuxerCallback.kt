package com.violet.libmedia.demuxer

import com.violet.libmedia.model.Frame

interface DemuxerCallback {
    fun onDemuxerReady(
        videoMineType: String, width: Int, height: Int,
        audioMineType: String, sampleRate: Int, audioChannels: Int,
        duration: Long
    )

    fun onDataAvailable(frame: Frame)
}