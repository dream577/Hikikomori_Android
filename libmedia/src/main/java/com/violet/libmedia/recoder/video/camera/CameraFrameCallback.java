package com.violet.libmedia.recoder.video.camera;

public interface CameraFrameCallback {
    void onPreviewFrame(byte[] data, int width, int height, int format, long timestamp);

    void onCaptureFrame(byte[] data, int width, int height, int format, long timestamp);
}
