package com.violet.libmedia.util;

import android.content.Context;
import android.content.res.AssetManager;

public class NativeAssertManager {
    static {
        System.loadLibrary("x264");
        System.loadLibrary("moonmedia");
    }

    private long assertManagerHandle = -1;

    public void init(Context context) {
        assertManagerHandle = setAssertManager(context.getAssets());
    }

    public void destroy() {

    }

    private native long setAssertManager(AssetManager manager);

    private native void destroyNativeAssertManager(long assertManagerHandle);
}
