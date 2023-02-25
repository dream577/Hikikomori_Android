package com.violet.libmedia.recoder.video;

import android.content.Context;
import android.hardware.camera2.CameraAccessException;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.NonNull;

import com.violet.libmedia.recoder.video.camera.CameraFrameCallback;
import com.violet.libmedia.recoder.video.camera.VioletCamera;
import com.violet.libmedia.recoder.video.camera.VioletCameraRecordClient;

import org.jetbrains.annotations.NotNull;

public class VioletCameraRecorder implements SurfaceHolder.Callback, CameraFrameCallback {
    private final VioletCameraRecordClient mCameraRecordClient;
    private final VioletCamera mCameraClient;

    private final SurfaceView mSurfaceView;

    public VioletCameraRecorder(@NotNull SurfaceView surfaceView) {
        Context context = surfaceView.getContext();
        mSurfaceView = surfaceView;
        mSurfaceView.getHolder().addCallback(this);
        mCameraClient = new VioletCamera(context, this);
        mCameraRecordClient = new VioletCameraRecordClient();
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        mCameraRecordClient.initRecorder();
        mCameraRecordClient.onSurfaceCreated(holder.getSurface());
        try {
            mCameraClient.initializeCamera(mSurfaceView.getDisplay());
            mCameraClient.startCamera();
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        mCameraRecordClient.onSurfaceChanged(width, height);
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        mCameraClient.closeCamera();
        mCameraRecordClient.onSurfaceDestroyed();
    }

    @Override
    public void onPreviewFrame(byte[] data, int width, int height, int format, long timestamp) {
        mCameraRecordClient.rendPreviewVideoFrame(data, width, height, format, timestamp);
    }

    @Override
    public void onCaptureFrame(byte[] data, int width, int height, int format, long timestamp) {

    }
}
