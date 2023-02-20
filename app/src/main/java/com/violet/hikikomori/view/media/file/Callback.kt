package com.violet.hikikomori.view.media.file

import com.violet.hikikomori.model.MediaItem

interface MediaViewHolderCallback {
    fun onClick(mediaBean: MediaItem)
}

interface MediaAdapterCallback {
    fun onClick(mediaBean: MediaItem)
}