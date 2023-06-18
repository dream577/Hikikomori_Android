package com.violet.libmedia.demuxer

import android.media.MediaExtractor
import android.media.MediaFormat
import com.violet.libbasetools.util.KLog
import com.violet.libmedia.model.AudioFrame
import com.violet.libmedia.model.Frame
import com.violet.libmedia.model.ImageFrame
import com.violet.libmedia.util.VThread
import java.nio.ByteBuffer

class MediaDemuxer(name: String) : VThread(name) {
    companion object {
        const val TAG = "MediaDemuxer"
        const val MSG_INIT = 0
        const val MSG_LOOP = 1
        const val MSG_UNINIT = 2
    }

    private val mediaExtractor: MediaExtractor = MediaExtractor()
    private val mediaBuffer = ByteBuffer.allocateDirect(1024 * 1024)

    private var videoIndex: Int = -1
    private var videoFrame = ImageFrame(mediaBuffer)
    private var audioIndex: Int = -1
    private var audioFrame = AudioFrame(mediaBuffer)

    private var audioPts: Long = 0
    private var videoPts: Long = 0
    private var isVideoEnd = false
    private var isAudioEnd = false

    private var path: String? = null
    private var mCallback: DemuxerCallback? = null

    constructor() : this(TAG)

    constructor(callback: DemuxerCallback) : this() {
        mCallback = callback
    }

    init {
        start()
    }

    fun initDemuxer(path: String) {
        this.path = path
        putMessage(MSG_INIT)
    }

    fun release() {
        putMessage(MSG_UNINIT)
        quit()
    }

    private fun init(): Boolean {
        var result = false
        if (path == null) return false
        mediaExtractor.setDataSource(path!!)

        // 获取解封装后流的数目
        val count = mediaExtractor.trackCount
        var duration: Long = 0

        var videoMineType: String? = null
        var videoWidth = 0
        var videoHeight = 0

        var audioMineType: String? = null
        var sampleRate = 0
        var audioChannels = 0

        for (index in 0 until count) {
            val mediaFormat = mediaExtractor.getTrackFormat(index)
            val mine = mediaFormat.getString(MediaFormat.KEY_MIME)
            if (mine?.startsWith("video/") == true && videoIndex == -1) {
                videoIndex = index
                videoMineType = mine
                videoWidth = mediaFormat.getInteger(MediaFormat.KEY_WIDTH)
                videoHeight = mediaFormat.getInteger(MediaFormat.KEY_HEIGHT)
                duration = mediaFormat.getLong(MediaFormat.KEY_DURATION)

                videoFrame.width = videoWidth
                videoFrame.height = videoHeight

                KLog.e(
                    TAG,
                    "mineType:${mine}, width:${videoWidth}, height:${videoHeight}, duration:${duration}"
                )
            }
            if (mine?.startsWith("audio/") == true && audioIndex == -1) {
                audioIndex = index
                audioMineType = mine
                sampleRate = mediaFormat.getInteger(MediaFormat.KEY_SAMPLE_RATE)
                audioChannels = mediaFormat.getInteger(MediaFormat.KEY_CHANNEL_COUNT)
                duration = mediaFormat.getLong(MediaFormat.KEY_DURATION)

                audioFrame.sampleRate = sampleRate
                audioFrame.channels = audioChannels

                KLog.e(
                    TAG,
                    "mineType:${mine}, sampleRate:${sampleRate}, audioChannels:${audioChannels}, duration:${duration}"
                )
            }

            if (audioIndex != -1 && videoIndex != -1) {
                result = true
                mCallback?.onDemuxerReady(
                    videoMineType!!, videoWidth, videoHeight,
                    audioMineType!!, sampleRate, audioChannels,
                    duration
                )
                break
            }
        }
        startLoop(MSG_LOOP)
        return result
    }

    private fun loopOnce() {
        // 解复用结束释放资源
        mediaBuffer.clear()
        if (isVideoEnd && isAudioEnd) {
            release()
            return
        }

        // 选取视频流或者音频流
        val selectTrackIndex: Int
        val frame: Frame
        if (videoPts <= audioPts && !isVideoEnd) {
            selectTrackIndex = videoIndex
            frame = videoFrame
        } else if (!isAudioEnd) {
            selectTrackIndex = audioIndex
            frame = audioFrame
        } else {
            release()
            return
        }

        // 读取数据以及时间戳等
        mediaExtractor.selectTrack(selectTrackIndex)
        val size = mediaExtractor.readSampleData(mediaBuffer, 0)
        val pts = mediaExtractor.sampleTime

        if (size <= 0) return
        when (selectTrackIndex) {
            videoIndex -> {
                frame.pts = pts
                videoPts = pts
                mCallback?.onDataAvailable(frame)
                if (!mediaExtractor.advance()) isVideoEnd = true
            }
            audioIndex -> {
                frame.pts = pts
                audioPts = pts
                mCallback?.onDataAvailable(frame)
                if (!mediaExtractor.advance()) isAudioEnd = true
            }
            else -> {

            }
        }
    }

    private fun unInit() {
        mediaExtractor.release()
    }

    override fun handleMessage(msg: Int) {
        super.handleMessage(msg)
        when (msg) {
            MSG_INIT -> init()
            MSG_LOOP -> loopOnce()
            MSG_UNINIT -> unInit()
        }
    }
}