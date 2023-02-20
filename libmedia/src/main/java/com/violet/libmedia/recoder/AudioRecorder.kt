package com.violet.libmedia.recoder

import android.Manifest
import android.content.Context
import com.violet.libbasetools.util.KLog.d
import android.media.AudioRecord
import kotlin.Throws
import androidx.core.app.ActivityCompat
import android.content.pm.PackageManager
import android.media.AudioFormat
import com.violet.libmedia.recoder.AudioRecorder
import com.violet.libbasetools.util.KLog
import android.media.MediaRecorder.AudioSource
import java.lang.Exception
import java.util.concurrent.Callable
import java.util.concurrent.atomic.AtomicBoolean

class AudioRecorder(private val mContext: Context) : Callable<Boolean> {
    private val mAudioRecord: AudioRecord? = null
    private var record: AudioRecord? = null
    private var mMinBufferSize = 0
    private val isStop: AtomicBoolean = AtomicBoolean(false)

    @Throws(Exception::class)
    override fun call(): Boolean {
        if (ActivityCompat.checkSelfPermission(
                mContext,
                Manifest.permission.RECORD_AUDIO
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            return false
        }
        mMinBufferSize = AudioRecord.getMinBufferSize(
            DEFAULT_SAMPLE_RATE,
            DEFAULT_CHANNEL_LAYOUT,
            DEFAULT_SAMPLE_FORMAT
        )

        if (AudioRecord.ERROR_BAD_VALUE == mMinBufferSize) {
            d(TAG, "parameters are not supported by the hardware.")
            return false
        }
        record = AudioRecord(
            AudioSource.DEFAULT,
            DEFAULT_SAMPLE_RATE,
            DEFAULT_CHANNEL_LAYOUT,
            DEFAULT_SAMPLE_FORMAT,
            mMinBufferSize
        )
        if (record!!.state != AudioRecord.STATE_INITIALIZED) {
            d(TAG, "AudioRecord init failure")
            return false
        }
        val mBuffer = ByteArray(4096)
        var result = 0
        record!!.startRecording()
        while (isStop.get()) {
            result = record!!.read(mBuffer, 0, 4096)
            if (result > 0) {
            } else {

            }
        }
        return true
    }

    companion object {
        private const val TAG = "AudioRecorder"
        private const val DEFAULT_SAMPLE_RATE = 44100
        private const val DEFAULT_CHANNEL_LAYOUT = AudioFormat.CHANNEL_IN_STEREO
        private const val DEFAULT_SAMPLE_FORMAT = AudioFormat.ENCODING_PCM_16BIT
    }
}