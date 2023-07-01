package com.violet.libmedia;

import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.view.Surface;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.violet.libmedia.codec.decoder.Decoder;
import com.violet.libmedia.codec.decoder.audio.AudioHwDecoder;
import com.violet.libmedia.codec.decoder.video.VideoHwDecoder;
import com.violet.libmedia.model.MediaFrame;
import com.violet.libmedia.render.RenderCallback;
import com.violet.libmedia.render.audiorender.AudioRender;
import com.violet.libmedia.render.imagerender.GLRenderWindow;
import com.violet.libmedia.render.sync.MediaSync;
import com.violet.libmedia.util.RecycledPool;

import java.util.concurrent.atomic.AtomicBoolean;

public class MediaClient implements RenderCallback {
    private static final String TAG = "MediaPlayer";
    private static final String MEDIA_SOURCE = "source";

    private static final int EVENT_START_PLAY = 0;
    private static final int EVENT_STOP_PLAY = 2;
    private static final int EVENT_DESTROY = 3;

    private final AtomicBoolean isStop = new AtomicBoolean(false);

    private final HandlerThread mThread = new HandlerThread(TAG);
    private final Handler mHandler;

    private final Decoder mVideoDecoder;
    private final Decoder mAudioDecoder;

    private final GLRenderWindow mRenderWindow;
    private final AudioRender mAudioRender;

    private final MediaSync mMediaSync;

    public MediaClient() {
        mThread.start();
        mVideoDecoder = new VideoHwDecoder();
        mAudioDecoder = new AudioHwDecoder();
        mRenderWindow = new GLRenderWindow(this);
        mAudioRender = new AudioRender(this);
        mMediaSync = new MediaSync();

        mHandler = new Handler(mThread.getLooper()) {
            @Override
            public void handleMessage(@NonNull Message msg) {
                MediaClient.this.handleMessage(msg);
            }
        };
    }

    private void handleMessage(Message msg) {
        switch (msg.what) {
            case EVENT_START_PLAY:
                String path = msg.getData().getString(MEDIA_SOURCE);
                mVideoDecoder.startDecoder(path);
                mRenderWindow.startRender();
                mAudioDecoder.startDecoder(path);
                mAudioRender.startRender();
                break;
            case EVENT_STOP_PLAY:
                mRenderWindow.stopRender();
                mVideoDecoder.release();
                mAudioRender.stopRender();
                mAudioDecoder.release();
                break;
            case EVENT_DESTROY:
                mThread.quitSafely();
                break;
        }
    }

    public void play(String path) {
        isStop.set(false);
        Message msg = Message.obtain();
        msg.what = EVENT_START_PLAY;
        Bundle data = msg.getData();
        data.putString(MEDIA_SOURCE, path);
        mHandler.sendMessage(msg);
    }

    public void stop() {
        isStop.set(true);
        mHandler.sendEmptyMessage(EVENT_STOP_PLAY);
    }

    public void destroy() {
        stop();
        mHandler.sendEmptyMessage(EVENT_DESTROY);
    }

    @Nullable
    @Override
    public RecycledPool.Element<MediaFrame> requestRenderFrame(boolean isVideo) {
        RecycledPool.Element<MediaFrame> element;
        do {
            if (isVideo) {
                element = mVideoDecoder.outputOneFrame();
            } else {
                element = mAudioDecoder.outputOneFrame();
            }
        } while (element == null && !isStop.get());
        if (element != null) {
            mMediaSync.syncMediaFrame(element.getValue());
        }
        return element;
    }

    public final void onSurfaceCreated(Surface surface) {
        mRenderWindow.onSurfaceCreated(surface);
    }

    public final void onSurfaceChanged(int width, int height) {
        mRenderWindow.onSurfaceChanged(width, height);
    }

    public final void onSurfaceDestroyed(Surface surface) {
        mRenderWindow.onSurfaceDestroyed(surface);
    }

    static class Builder {

    }
}
