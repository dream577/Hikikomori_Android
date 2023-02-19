package com.violet.libmedia.recoder.audio;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder.AudioSource;

import androidx.core.app.ActivityCompat;

import com.violet.libmedia.recoder.Recorder;

public class AudioRecorder implements Recorder {
    private static final int DEFAULT_SAMPLE_RATE = 44100;

    private final Context mContext;
    private AudioRecord record;

    private int channelConfig = AudioFormat.CHANNEL_IN_MONO;

    private int audioFormat = AudioFormat.ENCODING_PCM_16BIT;

    private int bufferSize;

    public AudioRecorder(Context context) {
        mContext = context;
    }

    @Override
    public int init() {
        if (ActivityCompat.checkSelfPermission(mContext, Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
            return -1;
        }
        bufferSize = AudioRecord.getMinBufferSize(DEFAULT_SAMPLE_RATE, channelConfig, audioFormat);
        record = new AudioRecord(AudioSource.DEFAULT, DEFAULT_SAMPLE_RATE, channelConfig, audioFormat, bufferSize);
        return 0;
    }

    @Override
    public int receiveOneFrame() {
        return 0;
    }

    @Override
    public int unInit() {
        return 0;
    }
}
