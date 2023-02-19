package com.violet.hikikomori.view.media.file.model

import android.graphics.Bitmap

open class MediaBean(
    val id: Int,
    val title: String,
    val path: String,
)

class VideoBean(
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
) : MediaBean(id, title, path)

class ImageBean(
    id: Int = 0,
    title: String,
    path: String,
    val height: Int? = 0,
    val width: Int? = 0,
    val orientation: Int? = 0
) : MediaBean(id, title, path)