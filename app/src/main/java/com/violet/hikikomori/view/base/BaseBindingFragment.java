package com.violet.hikikomori.view.base;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.databinding.DataBindingUtil;
import androidx.databinding.ViewDataBinding;

public abstract class BaseBindingFragment<T extends ViewDataBinding> extends BaseFragment {

    protected T mBinding;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        initData();
        mBinding = DataBindingUtil.inflate(inflater, getLayoutId(), container, false);
        initView();
        subscribeToViewModel();
        return mBinding.getRoot();
    }


    protected void initData() {

    }

    protected void initView() {

    }

    protected void subscribeToViewModel() {

    }

    protected abstract int getLayoutId();
}