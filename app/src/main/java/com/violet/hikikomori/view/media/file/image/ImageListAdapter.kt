package com.violet.hikikomori.view.media.file.image

import android.content.Context
import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.violet.hikikomori.R
import com.violet.hikikomori.view.media.file.MediaAdapterCallback
import com.violet.hikikomori.view.media.file.MediaViewHolderCallback
import com.violet.hikikomori.view.media.file.model.ImageBean
import com.violet.hikikomori.view.media.file.model.MediaBean

class ImageListAdapter(val mContext: Context) : RecyclerView.Adapter<ImageViewHolder>(),
    MediaViewHolderCallback {
    private lateinit var mAdapterCallback: MediaAdapterCallback

    val imageBeanList = mutableListOf<ImageBean>()

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ImageViewHolder {
        val view =
            LayoutInflater.from(parent.context).inflate(R.layout.item_image_layout, parent, false)
        return ImageViewHolder(mContext, view)
    }

    override fun onBindViewHolder(holder: ImageViewHolder, position: Int) {
        holder.initViewHolder(imageBeanList[position], this)
    }

    override fun getItemCount(): Int {
        return imageBeanList.size
    }

    fun refreshMediaList(list: List<ImageBean>) {
        imageBeanList.clear()
        imageBeanList.addAll(list)
        notifyDataSetChanged()
    }

    fun setMediaAdapterCallback(callback: MediaAdapterCallback) {
        mAdapterCallback = callback
    }

    override fun onClick(mediaBean: MediaBean) {
        mAdapterCallback.onClick(mediaBean)
    }
}