package com.violet.libmedia.recoder.video.camera;

import android.view.Surface;

public class VioletCameraRecordClient {
    static {
        System.loadLibrary("moonmedia");
    }

    private long recordHandle;

    public void initRecorder() {
        recordHandle = native_Init();
    }

    public void startRecord(String path) {
        native_startRecord(recordHandle, path);
    }

    public void stopRecord() {
        native_stopRecord(recordHandle);
    }

    public void rendPreviewVideoFrame(byte[] data, int width, int height, int format, long timestamp) {
        native_rendPreviewVideoFrame(recordHandle, data, width, height, format, timestamp);
    }

    public void onSurfaceCreated(Surface surface) {
        native_onSurfaceCreated(recordHandle, surface);
    }

    public void onSurfaceChanged(int width, int height) {
        native_onSurfaceChanged(recordHandle, width, height);
    }

    public void onSurfaceDestroyed() {
        native_onSurfaceDestroyed(recordHandle);
    }

    private native long native_Init();

    private native void native_startRecord(long recordHandle, String path);

    private native void native_stopRecord(long recordHandle);

    private native void native_rendPreviewVideoFrame(long recordHandle, byte[] data, int width, int height, int format, long timestamp);

    private native void native_onSurfaceCreated(long recordHandle, Object surface);

    private native void native_onSurfaceChanged(long recordHandle, int width, int height);

    private native void native_onSurfaceDestroyed(long recordHandle);
}
