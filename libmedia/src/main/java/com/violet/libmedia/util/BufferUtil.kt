package com.violet.libmedia.util

import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.FloatBuffer
import java.nio.ShortBuffer

object BufferUtil {
    fun createBuffer(array: FloatArray): FloatBuffer {
        val targetBuffer: FloatBuffer
        val buffer = ByteBuffer.allocateDirect(array.size * 4)
        buffer.order(ByteOrder.nativeOrder())
        targetBuffer = buffer.asFloatBuffer()
        targetBuffer.put(array)
        buffer.position(0)
        targetBuffer.position(0)
        return targetBuffer
    }

    fun createBuffer(array: ShortArray): ShortBuffer {
        val targetBuffer: ShortBuffer
        val buffer = ByteBuffer.allocateDirect(array.size * 2)
        buffer.order(ByteOrder.nativeOrder())
        targetBuffer = buffer.asShortBuffer()
        targetBuffer.put(array)
        buffer.position(0)
        targetBuffer.position(0)
        return targetBuffer
    }
}