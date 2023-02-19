package com.violet.hikikomori.view.media.file

import com.violet.hikikomori.view.media.file.model.MediaBean

interface MediaViewHolderCallback {
    fun onClick(mediaBean: MediaBean)
}

interface MediaAdapterCallback {
    fun onClick(mediaBean: MediaBean)
}