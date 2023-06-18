package com.violet.libmedia;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;

import androidx.annotation.NonNull;

import com.violet.libmedia.codec.decoder.AudioHardwareDecoder;
import com.violet.libmedia.codec.decoder.Decoder;
import com.violet.libmedia.codec.decoder.VideoHardwareDecoder;
import com.violet.libmedia.demuxer.DemuxerCallback;
import com.violet.libmedia.demuxer.MediaDemuxer;
import com.violet.libmedia.model.AudioDecoderConfig;
import com.violet.libmedia.model.AudioFrame;
import com.violet.libmedia.model.Frame;
import com.violet.libmedia.model.VideoDecoderConfig;

import org.jetbrains.annotations.NotNull;

public class MediaPlayer implements DemuxerCallback {
    private static final String TAG = "MediaPlayer";

    private final HandlerThread mThread = new HandlerThread(TAG);
    private final Handler mHandler;

    private MediaDemuxer mDemuxer;

    private VideoDecoderConfig mVideoConfig;
    private final Decoder mVideoDecoder;
    private AudioDecoderConfig mAudioConfig;
    private final Decoder mAudioDecoder;

    public MediaPlayer() {
        mThread.start();
        mDemuxer = new MediaDemuxer(this);
        mVideoDecoder = new VideoHardwareDecoder();
        mAudioDecoder = new AudioHardwareDecoder();

        mHandler = new Handler(mThread.getLooper()) {
            @Override
            public void handleMessage(@NonNull Message msg) {
                MediaPlayer.this.handleMessage(msg);
            }
        };
    }

    public void play(String path) {
        mDemuxer.initDemuxer(path);
    }

    @Override
    public void onDemuxerReady(@NonNull String videoMineType, int width, int height, @NonNull String audioMineType, int sampleRate, int audioChannels, long duration) {
        VideoDecoderConfig videoDecoderConfig = new VideoDecoderConfig(videoMineType, width, height);
        AudioDecoderConfig audioDecoderConfig = new AudioDecoderConfig(audioMineType, sampleRate, audioChannels, duration);
        if (mVideoConfig == null || mVideoConfig.isConfigChanged(videoDecoderConfig)) {
            mVideoConfig = videoDecoderConfig;
            if (mVideoDecoder instanceof VideoHardwareDecoder) {
                ((VideoHardwareDecoder) mVideoDecoder).changeCodecConfig(videoDecoderConfig);
                mVideoDecoder.start();
            }
        }
        if (mAudioConfig == null || mAudioConfig.isConfigChanged(audioDecoderConfig)) {
            mAudioConfig = audioDecoderConfig;
            if (mAudioDecoder instanceof AudioHardwareDecoder) {
                ((AudioHardwareDecoder) mAudioDecoder).changeCodecConfig(audioDecoderConfig);
                mAudioDecoder.start();
            }
        }
    }

    @Override
    public void onDataAvailable(@NotNull Frame frame) {
        if (frame instanceof AudioFrame) {
            mAudioDecoder.inputOneFrame(frame);
        } else {
            mVideoDecoder.inputOneFrame(frame);
        }
    }

    private void handleMessage(@NonNull Message msg) {

    }

    static class Builder {

    }
}
