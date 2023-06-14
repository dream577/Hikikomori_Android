package com.violet.libmedia.model

import java.nio.ByteBuffer

class ImageFrame(
    val dts: Long,
    val pts: Long,
    val width: Int,
    val height: Int,
    val format: Int,
    val buffer: ByteBuffer,
    val planeSize: IntArray
) {
    constructor() : this(
        0, 0, 0, 0,
        0, ByteBuffer.allocateDirect(1024), IntArray(3)
    )
}

class AudioFrame(
    val dts: Long,
    val pts: Long,
    val format: Int,
    val buffer: ByteBuffer,
    val size: Int,
    val channels: Int,
    val sampleRate: Int
)