package com.violet.hikikomori.view.media.file.image

import android.content.Context
import android.view.View
import android.widget.ImageView
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.bumptech.glide.Glide
import com.violet.hikikomori.R
import com.violet.hikikomori.model.ImageItem
import com.violet.hikikomori.view.media.file.MediaViewHolderCallback

class ImageViewHolder(val mContext: Context, itemView: View) : RecyclerView.ViewHolder(itemView) {
    private lateinit var mViewHolderCallback: MediaViewHolderCallback

    val mSelectView: ImageView
    val mIvContent: ImageView
    val mTvIntroduce: TextView

    init {
        mSelectView = itemView.findViewById(R.id.iv_select)
        mIvContent = itemView.findViewById(R.id.iv_content)
        mTvIntroduce = itemView.findViewById(R.id.tv_introduce)
    }

    fun initViewHolder(imageBean: ImageItem, callback: MediaViewHolderCallback) {
        mTvIntroduce.text = imageBean.title
        mViewHolderCallback = callback

        itemView.setOnClickListener {
            mViewHolderCallback.onClick(imageBean)
        }

        Glide.with(mContext).load(imageBean.path).into(mIvContent)
    }
}