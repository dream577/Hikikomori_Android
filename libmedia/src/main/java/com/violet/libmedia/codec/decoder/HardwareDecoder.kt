package com.violet.libmedia.codec.decoder

import android.media.MediaCodec
import android.media.MediaFormat
import android.view.Surface
import com.violet.libbasetools.util.KLog
import com.violet.libmedia.demuxer.Demuxer
import com.violet.libmedia.model.MediaFrame
import com.violet.libmedia.util.RecycledPool
import com.violet.libmedia.util.RecycledPool.Element
import com.violet.libmedia.util.VThread
import java.util.concurrent.LinkedBlockingQueue
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicBoolean

abstract class HardwareDecoder : VThread(TAG), Decoder {
    companion object {
        const val TAG = "HardwareCodec"

        private const val MSG_INIT = 0
        private const val MSG_DECODE = 1
        private const val MSG_UNINIT = 2

        const val OUTPUT_FRAME_QUEUE_SIZE = 10
    }

    private val isStop = AtomicBoolean(false)
    private lateinit var mediaCodec: MediaCodec
    private lateinit var demuxer: Demuxer
    private lateinit var path: String
    private val mBufferInfo = MediaCodec.BufferInfo()
    var surface: Surface? = null

    /*
     * 解码器是否已配置完成的标志位
     */
    @Volatile
    protected var configured = false

    private var recycledPool: RecycledPool<MediaFrame>? = null
    private var outputFrameQueue: LinkedBlockingQueue<Element<MediaFrame>>? = null

    final override fun startDecoder(path: String) {
        this.path = path
        isStop.set(false)
        start()
        putMessage(MSG_INIT)
        startLoop(MSG_DECODE)
    }

    final override fun configureCodec(): Boolean {
        val result = false
        do {
            try {
                demuxer = createDemuxer()
                demuxer.configDemuxer(path)
                val format = demuxer.getMediaFormat()
                val mineType = format.getString(MediaFormat.KEY_MIME) ?: break
                mediaCodec = MediaCodec.createDecoderByType(mineType)
                mediaCodec.configure(format, null, null, 0)
                mediaCodec.start()
                configured = true
                KLog.d(TAG, "配置解码器成功")
            } catch (e: Exception) {
                e.printStackTrace()
                realRelease()
                KLog.d(TAG, "配置解码器发生异常")
            }
        } while (false)

        return result
    }

    override fun isConfigured(): Boolean = configured

    private fun loop() {
        if (isStop.get()) return
        val codec = this.mediaCodec
        val info = this.mBufferInfo
        val demuxer = this.demuxer
        try {
            val inputIndex = codec.dequeueInputBuffer(0)
            if (inputIndex >= 0) {
                val buffer = codec.getInputBuffer(inputIndex)
                val size = demuxer.readSampleData(buffer!!)
                val pts = demuxer.getSampleTime()

//                    KLog.d(TAG, "frame[size=$size pts=$pts]")
                if (size < 0) {
                    codec.queueInputBuffer(
                        inputIndex, 0, size, pts,
                        MediaCodec.BUFFER_FLAG_END_OF_STREAM
                    )
                } else {
                    codec.queueInputBuffer(inputIndex, 0, size, pts, 0)
                }
            }

            var outputIndex: Int
            do {
                outputIndex = codec.dequeueOutputBuffer(
                    info,
                    100
                ) // 阻塞100微秒，阻塞时间过长会导致解复用 -> 输入解码器 -> 解码器输出整个过程变慢
                when (outputIndex) {
                    MediaCodec.INFO_TRY_AGAIN_LATER -> {

                    }
                    MediaCodec.INFO_OUTPUT_FORMAT_CHANGED -> {
                        if (recycledPool == null) {
                            recycledPool = RecycledPool(OUTPUT_FRAME_QUEUE_SIZE)
                        }
                        if (outputFrameQueue == null) {
                            outputFrameQueue = LinkedBlockingQueue(OUTPUT_FRAME_QUEUE_SIZE);
                        }
                        val format = codec.outputFormat
                        KLog.d(TAG, format)
                        onOutputFormatChanged(format, recycledPool!!)
                    }
                    MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED -> {

                    }
                    else -> {
                        val pool = this.recycledPool!!
                        val queue = this.outputFrameQueue!!

                        val srcBuffer = codec.getOutputBuffer(outputIndex)

                        var element: Element<MediaFrame>? = null
                        while (element == null && !isStop.get()) {
                            element = pool.take(50, TimeUnit.MILLISECONDS) ?: continue
                            val frame = element.value
                            val size = info.size
                            frame.pts = info.presentationTimeUs
//                            KLog.d(TAG, "output[pts=${frame.pts} size=${size}]")

                            if (srcBuffer != null) {
                                frame.buffer.put(srcBuffer)
                            }
                            queue.offer(element)
                        }
                        codec.releaseOutputBuffer(outputIndex, false)
                    }
                }

            } while (outputIndex >= 0 && configured && !isStop.get())
        } catch (e: Exception) {
            e.printStackTrace()
            isStop.set(true)
            realRelease()
        }
    }

    abstract fun onOutputFormatChanged(format: MediaFormat, pool: RecycledPool<MediaFrame>)

    override fun outputOneFrame(): Element<MediaFrame>? {
        return outputFrameQueue?.poll(50, TimeUnit.MILLISECONDS)
    }

    override fun release() {
        isStop.set(true)
        putMessage(MSG_UNINIT)
        quit()
    }

    private fun realRelease() {
        val codec = this.mediaCodec
        try {
            codec.apply {
                stop()
                release()
            }
        } catch (e: Exception) {
            e.printStackTrace()
        } finally {
            demuxer.release()
            outputFrameQueue?.clear()
            recycledPool?.clear()
        }
    }

    override fun handleMessage(msg: Int) {
        super.handleMessage(msg)
        when (msg) {
            MSG_INIT -> configureCodec()
            MSG_DECODE -> loop()
            MSG_UNINIT -> realRelease()
        }
    }
}