package com.violet.libmedia;

import android.view.Surface;

public class VioletMediaClient {
    static {
        System.loadLibrary("moonmedia");
    }

    private long mNativePlayerHandle = 0;

    public static String GetFFmpegVersion() {
        return native_GetFFmpegVersion();
    }

    public void init(String url, Surface surface) {
        mNativePlayerHandle = native_Init(url, 0, 0, surface);
    }

    public void play() {
        native_Play(mNativePlayerHandle);
    }

    public void pause() {
        native_pause(mNativePlayerHandle);
    }

    public void resume() {
        native_resume(mNativePlayerHandle);
    }

    private static native String native_GetFFmpegVersion();

    private native long native_Init(String url, int playerType, int renderType, Object surface);

    private native void native_Play(long playerHandle);

    private native void native_pause(long playerHandle);

    private native void native_resume(long playerHandle);
}
