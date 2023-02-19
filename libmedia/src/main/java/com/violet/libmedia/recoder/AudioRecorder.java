package com.violet.libmedia.recoder;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder.AudioSource;

import androidx.core.app.ActivityCompat;

import com.violet.libbasetools.util.KLog;

import java.util.concurrent.Callable;
import java.util.concurrent.atomic.AtomicBoolean;

public class AudioRecorder implements Callable<Boolean> {

    private static final String TAG = "AudioRecorder";

    private AudioRecord mAudioRecord = null;
    private static final int DEFAULT_SAMPLE_RATE = 44100;
    private static final int DEFAULT_CHANNEL_LAYOUT = AudioFormat.CHANNEL_IN_STEREO;
    private static final int DEFAULT_SAMPLE_FORMAT = AudioFormat.ENCODING_PCM_16BIT;

    private final Context mContext;
    private AudioRecord record;

    private int mMinBufferSize;

    private AtomicBoolean isStop;

    public AudioRecorder(Context context) {
        mContext = context;
        isStop.set(false);
    }

    @Override
    public Boolean call() throws Exception {
        if (ActivityCompat.checkSelfPermission(mContext, Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
            return false;
        }
        mMinBufferSize = AudioRecord.getMinBufferSize(DEFAULT_SAMPLE_RATE, DEFAULT_CHANNEL_LAYOUT, DEFAULT_SAMPLE_FORMAT);
        if (AudioRecord.ERROR_BAD_VALUE == mMinBufferSize) {
            KLog.d(TAG, "parameters are not supported by the hardware.");
            return false;
        }
        record = new AudioRecord(AudioSource.DEFAULT, DEFAULT_SAMPLE_RATE, DEFAULT_CHANNEL_LAYOUT, DEFAULT_SAMPLE_FORMAT, mMinBufferSize);
        if (record.getState() != AudioRecord.STATE_INITIALIZED) {
            KLog.d(TAG, "AudioRecord init failure");
            return false;
        }
        byte[] mBuffer = new byte[4096];
        int result = 0;
        record.startRecording();
        while (isStop.get()) {
            result = record.read(mBuffer, 0, 4096);
            if (result > 0) {

            } else {

            }
        }
        return true;
    }
}
