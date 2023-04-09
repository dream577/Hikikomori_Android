package com.violet.libmedia.recoder.video.camera

interface CameraFrameCallback {
    fun onPreviewFrame(data: ByteArray?, width: Int, height: Int, format: Int, timestamp: Long)
    fun onCaptureFrame(data: ByteArray?, width: Int, height: Int, format: Int, timestamp: Long)
}