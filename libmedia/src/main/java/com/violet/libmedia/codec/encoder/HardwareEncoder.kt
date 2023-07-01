package com.violet.libmedia.codec.encoder

import android.media.MediaCodec
import com.violet.libmedia.util.VThread
import java.util.concurrent.atomic.AtomicBoolean

abstract class HardwareEncoder(name: String) : VThread(name), Encoder {
    companion object {
        private const val TAG = "HardwareEncoder"
    }

    private val isStop = AtomicBoolean(false)
    private lateinit var mediaCodec: MediaCodec
    private lateinit var path: String
    private val mBufferInfo = MediaCodec.BufferInfo()

    constructor() : this(TAG)
}