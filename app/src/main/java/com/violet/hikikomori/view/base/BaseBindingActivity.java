package com.violet.hikikomori.view.base;

import androidx.databinding.DataBindingUtil;
import androidx.databinding.ViewDataBinding;

public abstract class BaseBindingActivity<T extends ViewDataBinding> extends BaseActivity {

    protected T mBinding;

    @Override
    protected void initData() {
        super.initData();
    }

    @Override
    protected void initView() {
        mBinding = DataBindingUtil.setContentView(this, getLayoutId());
    }

    @Override
    protected void subscribeToViewModel() {
        super.subscribeToViewModel();
    }
}