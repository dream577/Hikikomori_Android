package com.violet.hikikomori.model

import android.graphics.Bitmap
import android.hardware.camera2.CaptureResult
import android.media.Image
import java.io.Closeable

open class MediaItem(
    val id: Int,
    val title: String,
    val path: String,
)

class VideoItem(
    id: Int,
    title: String,
    val album: String? = null,
    val artist: String? = null,
    val displayName: String? = null,
    val mimeType: String? = null,
    path: String,
    val size: Long = 0,
    val duration: Int = 0,
    val thumbnail: Bitmap? = null
) : MediaItem(id, title, path)

class ImageItem(
    id: Int = 0,
    title: String,
    path: String,
    val height: Int? = 0,
    val width: Int? = 0,
    val orientation: Int? = 0
) : MediaItem(id, title, path)

data class CombinedCaptureResult(
    val image: Image,
    val metadata: CaptureResult,
    val orientation: Int,
    val format: Int
) : Closeable {
    override fun close() = image.close()
}