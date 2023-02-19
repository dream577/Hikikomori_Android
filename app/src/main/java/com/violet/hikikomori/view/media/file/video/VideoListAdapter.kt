package com.violet.hikikomori.view.media.file.video

import android.content.Context
import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.violet.hikikomori.R
import com.violet.hikikomori.view.media.file.MediaAdapterCallback
import com.violet.hikikomori.view.media.file.MediaViewHolderCallback
import com.violet.hikikomori.view.media.file.model.MediaBean
import com.violet.hikikomori.view.media.file.model.VideoBean

class VideoListAdapter(val mContext: Context) : RecyclerView.Adapter<VideoViewHolder>(),
    MediaViewHolderCallback {

    private lateinit var mAdapterCallback: MediaAdapterCallback


    val videoBeanList = mutableListOf<VideoBean>()

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): VideoViewHolder {
        val view =
            LayoutInflater.from(parent.context).inflate(R.layout.item_video_layout, parent, false)
        return VideoViewHolder(mContext, view)
    }

    override fun onBindViewHolder(holder: VideoViewHolder, position: Int) {
        holder.initViewHolder(videoBeanList[position], this)
    }

    override fun getItemCount(): Int {
        return videoBeanList.size
    }

    fun refreshMediaList(list: List<VideoBean>) {
        videoBeanList.clear()
        videoBeanList.addAll(list)
        notifyDataSetChanged()
    }

    override fun onClick(mediaBean: MediaBean) {
        mAdapterCallback.onClick(mediaBean)
    }

    fun setMediaAdapterCallback(callback: MediaAdapterCallback) {
        mAdapterCallback = callback
    }
}