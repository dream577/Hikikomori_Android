package com.violet.hikikomori.view.util;

import android.view.View;

import androidx.databinding.BindingAdapter;

public class DataBindingAdapterUtil {
    @BindingAdapter({"visibleOrGone"})
    public static void setVisibleOrGone(View view, Boolean visible) {
        view.setVisibility(visible ? View.VISIBLE : View.GONE);
    }
}
