package com.violet.libmedia.render.audiorender

import android.media.AudioFormat
import android.media.AudioManager
import android.media.AudioTrack
import com.violet.libmedia.model.MediaFrame
import com.violet.libmedia.render.RenderCallback
import com.violet.libmedia.util.VThread
import java.util.concurrent.atomic.AtomicBoolean

class AudioRender(name: String) : VThread(name) {
    companion object {
        private const val TAG = "AudioRender"

        private const val EVENT_INIT = 0
        private const val EVENT_LOOP = 1
        private const val EVENT_UNINIT = 2
    }

    private lateinit var audioTrack: AudioTrack
    private val isConfigured = AtomicBoolean(false)
    private val isStop = AtomicBoolean(false)
    private var mRenderCallback: RenderCallback? = null

    constructor() : this(TAG)

    constructor(callback: RenderCallback) : this() {
        this.mRenderCallback = callback
    }

    fun startRender() {
        isStop.set(false)
        start()
        startLoop(EVENT_LOOP)
    }

    fun stopRender() {
        isStop.set(true)
        putMessage(EVENT_UNINIT)
        quit()
    }

    private fun configAudioTrack(frame: MediaFrame) {
        try {
            val minBufferSize = AudioTrack.getMinBufferSize(
                frame.sampleRate,
                AudioFormat.CHANNEL_OUT_STEREO,
                AudioFormat.ENCODING_PCM_16BIT
            )
            audioTrack = AudioTrack(
                AudioManager.STREAM_MUSIC, frame.sampleRate, AudioFormat.CHANNEL_OUT_STEREO,
                AudioFormat.ENCODING_PCM_16BIT, minBufferSize, AudioTrack.MODE_STREAM
            )
            audioTrack.play()
            isConfigured.set(true)
        } catch (e: Exception) {
            e.printStackTrace()
            stopRender()
        }
    }

    private fun loop(frame: MediaFrame) {
        val buffer = frame.buffer
        buffer.position(0)
        buffer.limit(frame.planeSize[0])

        audioTrack.write(buffer, frame.planeSize[0], AudioTrack.WRITE_BLOCKING)

        buffer.clear()
    }

    private fun unInit() {
        audioTrack.stop()
        audioTrack.release()
        isConfigured.set(false)
    }

    override fun handleMessage(msg: Int) {
        super.handleMessage(msg)
        when (msg) {
            EVENT_INIT -> {

            }
            EVENT_LOOP -> {
                val element = mRenderCallback?.requestRenderFrame(false) ?: return
                val frame = element.value
                if (!isConfigured.get()) {
                    configAudioTrack(frame)
                }
                loop(frame)
                element.recycle()
            }
            EVENT_UNINIT -> {
                unInit()
            }
        }
    }
}