package com.violet.libmedia.codec.decoder

import android.media.MediaCodec
import android.media.MediaFormat
import android.os.Handler
import android.os.HandlerThread
import android.os.Message
import com.violet.libbasetools.util.KLog
import com.violet.libmedia.model.Frame
import java.util.concurrent.LinkedBlockingQueue
import java.util.concurrent.TimeUnit

abstract class HardwareDecoder : MediaCodec.Callback(), Decoder {
    companion object {
        const val TAG = "HardwareCodec"
        const val BUFFER_INDEX = "index"

        const val INPUT_FRAME_QUEUE_SIZE = 5
        const val OUTPUT_FRAME_QUEUE_SIZE = 5

        /**
         * 解码器消息类型
         */
        const val MSG_INIT_DECODER = 0
        const val MSG_INPUT_FRAME = 1
        const val MSG_OUTPUT_FRAME = 2
    }

    private lateinit var mediaCodec: MediaCodec
    private val mBufferInfo = MediaCodec.BufferInfo()

    private val mHandlerThread = HandlerThread("HardwareCodec")
    private val mHandler: Handler

    /*
     * 解码器是否已配置完成的标志位
     */
    @Volatile
    protected var configured = false

    /*
     * 帧队列
     */
    private val inputFrameQueue = LinkedBlockingQueue<Frame>(INPUT_FRAME_QUEUE_SIZE)
    private val outputFrameQueue = LinkedBlockingQueue<Frame>(OUTPUT_FRAME_QUEUE_SIZE)

    init {
        mHandlerThread.start()
        mHandler = object : Handler(mHandlerThread.looper) {
            override fun handleMessage(msg: Message) {
                super.handleMessage(msg)
                when (msg.what) {
                    MSG_INIT_DECODER -> {
                        configureCodec()
                    }
                    MSG_INPUT_FRAME -> {
                        val index = msg.data.getInt(BUFFER_INDEX)
                        realInputFrame(index)
                    }
                    MSG_OUTPUT_FRAME -> {
                        val index = msg.data.getInt(BUFFER_INDEX)
                        realOutputFrame(index)
                    }
                }
            }
        }
    }

    final override fun start() {
        mHandler.sendEmptyMessage(MSG_INIT_DECODER)
    }

    final override fun configureCodec(): Boolean {
        val result = false
        do {
            try {
                val format = getMediaFormat()
                val mineType = format.getString(MediaFormat.KEY_MIME) ?: break
                mediaCodec = MediaCodec.createDecoderByType(mineType)
                mediaCodec.setCallback(this)
                mediaCodec.configure(format, null, null, 0)
                mediaCodec.start()
                configured = true
                KLog.d(TAG, "配置解码器成功")
            } catch (e: Exception) {
                e.printStackTrace()
                KLog.d(TAG, "配置解码器发生异常")
            } finally {
                release()
            }
        } while (false)

        return result
    }

    abstract fun getMediaFormat(): MediaFormat

    final override fun onInputBufferAvailable(codec: MediaCodec, index: Int) {
        val message = Message.obtain()
        message.what = MSG_INPUT_FRAME
        val data = message.data
        data.putInt(BUFFER_INDEX, index)
        mHandler.sendMessage(message)
    }

    final override fun onOutputBufferAvailable(
        codec: MediaCodec,
        index: Int,
        info: MediaCodec.BufferInfo
    ) {
        val message = Message.obtain()
        message.what = MSG_OUTPUT_FRAME
        val data = message.data
        data.putInt(BUFFER_INDEX, index)
        mHandler.sendMessage(message)
    }

    final override fun onError(codec: MediaCodec, e: MediaCodec.CodecException) {
        KLog.d(TAG, e)
    }

    final override fun onOutputFormatChanged(codec: MediaCodec, format: MediaFormat) {
        val colorFormat = format.getInteger(MediaFormat.KEY_COLOR_FORMAT)
        KLog.d(TAG, "decoder output format:$colorFormat")
    }

    override fun isConfigured(): Boolean = configured

    override fun inputOneFrame(frame: Frame): Boolean {
//        return inputFrameQueue.offer(frame, 50, TimeUnit.MILLISECONDS)
//        KLog.d(TAG, frame)
        return false
    }

    private fun realInputFrame(index: Int) {
        while (configured) {
            val frame = inputFrameQueue.poll(50, TimeUnit.MILLISECONDS) ?: continue
            if (index >= 0) {
                val sourceBuffer = frame.buffer
                val size = sourceBuffer.remaining()
                val pts = frame.pts
                // 经过测试，输入buffer大小默认为 1024 * 6912 = 7077888
                val targetBuffer = mediaCodec.getInputBuffer(index)
                KLog.d(TAG, "BufferDataSize: $size  MediaCodec Buffer capacity: ${targetBuffer?.capacity()}")
                targetBuffer?.apply {
                    clear()
                    put(sourceBuffer)
                    mediaCodec.queueInputBuffer(index, 0, size, pts, 0)
                }
                break
            }
        }
    }

    override fun outputOneFrame(): Frame? {
        return outputFrameQueue.poll(50, TimeUnit.MILLISECONDS)
    }

    private fun realOutputFrame(index: Int) {
        while (configured) {
            if (index >= 0) {
                val sourceBuffer = mediaCodec.getOutputBuffer(index)
                val size = mBufferInfo.size
                val offset = mBufferInfo.offset
                val pts = mBufferInfo.presentationTimeUs
            }
            break
        }
    }

    override fun release() {
        mediaCodec.apply {
            stop()
            release()
        }
    }
}