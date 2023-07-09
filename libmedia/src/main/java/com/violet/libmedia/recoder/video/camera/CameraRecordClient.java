package com.violet.libmedia.recoder.video.camera;

import android.content.Context;
import android.view.Surface;

import androidx.annotation.NonNull;

import com.violet.libbasetools.util.FileUtilKt;
import com.violet.libmedia.recoder.audio.AudioRecorder;

public class CameraRecordClient implements AudioRecorder.AudioRecordCallback {
    static {
        System.loadLibrary("x264");
        System.loadLibrary("moonmedia");
    }

    private Context mContext;
    private long recordHandle;
    private AudioRecorder mAudioRecorder;

    public void initRecorder(Context context) {
        mContext = context;
        mAudioRecorder = new AudioRecorder();
        mAudioRecorder.setRecordCallback(this);
        recordHandle = native_Init();
    }

    public void startRecord(String name) {
        String path = FileUtilKt.checkVideoDir(mContext) + name;
        native_startRecord(recordHandle, path, name);
        mAudioRecorder.startRecord();
    }

    public void stopRecord() {
        mAudioRecorder.stopRecord();
        native_stopRecord(recordHandle);
    }

    public void inputVideoFrame(byte[] data, int width, int height, int format, long timestamp) {
        native_InputVideoFrame(recordHandle, data, width, height, format, timestamp);
    }

    public void inputAudioFrame(byte[] data, int size, long timestamp, int sampleRate, int sampleFormat, int channelLayout) {
        native_InputAudioFrame(recordHandle, data, size, timestamp, sampleRate, sampleFormat, channelLayout);
    }

    public void onSurfaceCreated(Surface surface) {
        native_onSurfaceCreated(recordHandle, surface);
    }

    public void onSurfaceChanged(int width, int height) {
        native_onSurfaceChanged(recordHandle, width, height);
    }

    public void setTransformMatrix(float translateX, float translateY, float scaleX, float scaleY, int degree, int mirror) {
        native_SetTransformMatrix(recordHandle, translateX, translateY, scaleX, scaleY, degree, mirror);
    }

    @Override
    public void onReadSampleData(@NonNull byte[] data, int size, long timestamp, int sampleRate, int sampleFormat, int channelLayout) {
        inputAudioFrame(data, size, timestamp, sampleRate, sampleFormat, channelLayout);
    }

    public void onSurfaceDestroyed() {
        native_onSurfaceDestroyed(recordHandle);
    }

    private native long native_Init();

    private native void native_startRecord(long recordHandle, String path, String name);

    private native void native_stopRecord(long recordHandle);

    private native void native_InputVideoFrame(long recordHandle, byte[] data, int width, int height, int format, long timestamp);

    private native void native_InputAudioFrame(long recordHandle, byte[] data, int size, long timestamp, int sampleRate, int sampleFormat, int channelLayout);

    private native void native_onSurfaceCreated(long recordHandle, Object surface);

    private native void native_onSurfaceChanged(long recordHandle, int width, int height);

    private native void native_onSurfaceDestroyed(long recordHandle);

    private native void native_SetTransformMatrix(long recordHandle, float translateX, float translateY, float scaleX, float scaleY, int degree, int mirror);
}
