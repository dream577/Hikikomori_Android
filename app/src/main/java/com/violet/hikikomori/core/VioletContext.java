package com.violet.hikikomori.core;

import android.app.Application;

import com.violet.libmedia.util.NativeAssertManager;

public class VioletContext extends Application {
    private NativeAssertManager manager;

    @Override
    public void onCreate() {
        super.onCreate();
        initNativeAssertManager();
    }

    private void initNativeAssertManager() {
        manager = new NativeAssertManager();
        manager.init(this);
    }
}
