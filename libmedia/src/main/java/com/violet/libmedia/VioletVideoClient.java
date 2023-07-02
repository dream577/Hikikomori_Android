package com.violet.libmedia;

import android.view.Surface;

public class VioletVideoClient {
    static {
        System.loadLibrary("x264");
        System.loadLibrary("moonmedia");
    }

    public static final String EVENT_VALUE = "value";

    private long mNativePlayerHandle = 0;

    private MediaEventCallback callback;

    public void registerEventCallback(MediaEventCallback callback) {
        this.callback = callback;
    }

    public void unRegisterEventCallback(MediaEventCallback callback) {
        this.callback = null;
    }

    public static String GetFFmpegVersion() {
        return native_GetFFmpegVersion();
    }

    public void init(String url) {
        mNativePlayerHandle = native_Init(url, 0, 0);
    }

    public void play() {
        if (mNativePlayerHandle == 0) return;
        native_Play(mNativePlayerHandle);
    }

    public void pause() {
        if (mNativePlayerHandle == 0) return;
        native_pause(mNativePlayerHandle);
    }

    /**
     * 处于暂停状态时使用该方法恢复播放，暂停时调用play方法不会生效
     */
    public void resume() {
        if (mNativePlayerHandle == 0) return;
        native_resume(mNativePlayerHandle);
    }

    public void stop() {
        if (mNativePlayerHandle == 0) return;
        native_stop(mNativePlayerHandle);
        mNativePlayerHandle = 0;
    }

    /**
     * 定位到某个时间戳
     *
     * @param timestamp 单位：s
     */
    public void seekToPosition(float timestamp) {
        if (mNativePlayerHandle == 0) return;
        native_seekToPosition(mNativePlayerHandle, timestamp);
    }

    public void onSurfaceCreated(Surface surface) {
        native_onSurfaceCreated(mNativePlayerHandle, surface);
    }

    public void onSurfaceChanged(int w, int h) {
        native_onSurfaceChanged(mNativePlayerHandle, w, h);
    }

    public void onSurfaceDestroyed(Surface surface) {
        native_onSurfaceDestroyed(mNativePlayerHandle);
    }

    private void onReceiveMediaEvent(int type, long value) {
        if (callback != null) {
            callback.onReceiveEvent(type, value);
        }
    }

    private static native String native_GetFFmpegVersion();

    private native long native_Init(String url, int playerType, int renderType);

    private native void native_Play(long playerHandle);

    private native void native_pause(long playerHandle);

    private native void native_resume(long playerHandle);

    private native void native_stop(long playerHandle);

    private native void native_seekToPosition(long playerHandle, float position);

    private native void native_onSurfaceCreated(long playerHandle, Object surface);

    private native void native_onSurfaceChanged(long playerHandle, int w, int h);

    private native void native_onSurfaceDestroyed(long playerHandle);

    public interface MediaEventCallback {
        void onReceiveEvent(int messageType, long value);
    }

}
