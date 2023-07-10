package com.violet.libmedia.recoder.audio

import android.annotation.SuppressLint
import android.media.AudioFormat
import android.media.AudioRecord
import android.media.MediaRecorder.AudioSource
import com.violet.libbasetools.util.KLog
import java.util.concurrent.atomic.AtomicBoolean

class AudioRecorder {
    companion object {
        private const val TAG = "AudioRecorder"
        private const val DEFAULT_SAMPLE_RATE = 44100
        private const val DEFAULT_CHANNEL_LAYOUT = AudioFormat.CHANNEL_IN_STEREO
        private const val DEFAULT_SAMPLE_FORMAT = AudioFormat.ENCODING_PCM_16BIT
    }

    private var mAudioRecord: AudioRecord? = null
    private var mRecordThread: Thread? = null
    private var mMinBufferSize = 0
    private val isStop: AtomicBoolean = AtomicBoolean(false)
    var recordCallback: AudioRecordCallback? = null

    fun startRecord() {
        mRecordThread = Thread({
            try {
                _startRecord()
            } catch (e: Exception) {
                e.printStackTrace()
                KLog.d(TAG, "Record Error")
            } finally {
                stopRecord()
            }
        }, "AudioRecordThread")
        mRecordThread!!.start()
    }

    @SuppressLint("MissingPermission")
    @Throws(Exception::class)
    private fun _startRecord(): Boolean {
        mMinBufferSize = AudioRecord.getMinBufferSize(
            DEFAULT_SAMPLE_RATE,
            DEFAULT_CHANNEL_LAYOUT,
            DEFAULT_SAMPLE_FORMAT
        )
        if (AudioRecord.ERROR_BAD_VALUE == mMinBufferSize) {
            KLog.d(TAG, "parameters are not supported by the hardware.")
            return false
        }
        mAudioRecord = AudioRecord(
            AudioSource.DEFAULT,
            DEFAULT_SAMPLE_RATE,
            DEFAULT_CHANNEL_LAYOUT,
            DEFAULT_SAMPLE_FORMAT,
            mMinBufferSize
        )
        if (mAudioRecord!!.state != AudioRecord.STATE_INITIALIZED) {
            KLog.d(TAG, "AudioRecord init failure")
            return false
        }
        val mBuffer = ByteArray(4096)
        var size: Int
        var timestamp: Long
        mAudioRecord!!.startRecording()
        while (!isStop.get()) {
            timestamp = System.nanoTime() / 1000
            size = mAudioRecord!!.read(mBuffer, 0, 4096)
            if (size > 0) {
                recordCallback?.onReadSampleData(
                    mBuffer, size, timestamp,
                    DEFAULT_SAMPLE_RATE,
                    DEFAULT_SAMPLE_FORMAT,
                    DEFAULT_CHANNEL_LAYOUT
                )
            } else {
                when (size) {
                    AudioRecord.ERROR_INVALID_OPERATION -> {
                        KLog.d(TAG, "ERROR_INVALID_OPERATION")
                    }
                    AudioRecord.ERROR_BAD_VALUE -> {
                        KLog.d(TAG, "ERROR_BAD_VALUE")
                    }
                    AudioRecord.ERROR_DEAD_OBJECT -> {
                        KLog.d(TAG, "ERROR_DEAD_OBJECT")
                    }
                }
            }
        }
        return true
    }

    fun stopRecord() {
        isStop.set(true)
    }

    interface AudioRecordCallback {
        fun onReadSampleData(
            data: ByteArray,
            size: Int,
            timestamp: Long,
            sampleRate: Int,
            sampleFormat: Int,
            channelLayout: Int
        )
    }
}