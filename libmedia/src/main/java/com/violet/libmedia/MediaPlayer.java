package com.violet.libmedia;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.view.Surface;

import androidx.annotation.NonNull;

import com.violet.libmedia.codec.decoder.AudioHardwareDecoder;
import com.violet.libmedia.codec.decoder.Decoder;
import com.violet.libmedia.codec.decoder.VideoHardwareDecoder;

public class MediaPlayer {
    private static final String TAG = "MediaPlayer";

    private final HandlerThread mThread = new HandlerThread(TAG);
    private final Handler mHandler;

    private final Decoder mVideoDecoder;
    private final Decoder mAudioDecoder;

    public MediaPlayer(Surface surface) {
        mThread.start();
        mVideoDecoder = new VideoHardwareDecoder(surface);
        mAudioDecoder = new AudioHardwareDecoder();

        mHandler = new Handler(mThread.getLooper()) {
            @Override
            public void handleMessage(@NonNull Message msg) {

            }
        };
    }

    public void play(String path) {
        mVideoDecoder.start(path);
    }

    static class Builder {

    }
}
