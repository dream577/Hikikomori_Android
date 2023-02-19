package com.violet.hikikomori.view.base

import android.view.View
import androidx.databinding.DataBindingUtil
import androidx.databinding.ViewDataBinding
import androidx.recyclerview.widget.RecyclerView

class BaseViewHolder<T : ViewDataBinding?>(itemView: View) : RecyclerView.ViewHolder(itemView) {
    var binding: T?

    init {
        binding = DataBindingUtil.bind(itemView)
    }
}