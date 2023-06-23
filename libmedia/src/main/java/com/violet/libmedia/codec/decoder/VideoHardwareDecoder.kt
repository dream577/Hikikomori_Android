package com.violet.libmedia.codec.decoder

import android.media.MediaCodecInfo
import android.media.MediaFormat
import android.view.Surface
import com.violet.libbasetools.util.KLog
import com.violet.libmedia.demuxer.Demuxer
import com.violet.libmedia.demuxer.MediaDemuxer
import com.violet.libmedia.model.ImageFormat
import com.violet.libmedia.model.MediaFrame
import com.violet.libmedia.util.RecycledPool
import java.nio.ByteBuffer

class VideoHardwareDecoder(surface: Surface?) : HardwareDecoder() {

    companion object {
        const val TAG = "VideoHardwareDecoder"
    }

    init {
        this.surface = surface
    }

    constructor() : this(null)

    override fun prepareDemuxer(): Demuxer {
        val demuxer = MediaDemuxer(true)
        demuxer.configDemuxer("/storage/emulated/0/视频/[Airota][Fate stay night Heaven's Feel III.spring song][Movie][BDRip 1080p AVC AAC][CHS].mp4")
        return demuxer
    }

    override fun onOutputFormatChanged(format: MediaFormat, pool: RecycledPool<MediaFrame>) {
        val width = format.getInteger(MediaFormat.KEY_WIDTH)
        val height = format.getInteger(MediaFormat.KEY_HEIGHT)
        val colorFormat = format.getInteger(MediaFormat.KEY_COLOR_FORMAT)
        val stride = format.getInteger("stride")

        val capacity: Int
        val planeArray: IntArray
        val imageFormat: ImageFormat = when (colorFormat) {
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar -> {  // YV21、I420、YUV420SP
                /*
                 * Y Y Y Y
                 * Y Y Y Y
                 * Y Y Y Y
                 * Y Y Y Y
                 * U U U U
                 * V V V V
                 */
                capacity = stride * height + stride * height / 4 + stride * height / 4
                planeArray = intArrayOf(stride, stride / 2, stride / 2)
                ImageFormat.IMAGE_FORMAT_YV21
            }
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420PackedPlanar -> { // YV12
                /*
                 * Y Y Y Y
                 * Y Y Y Y
                 * Y Y Y Y
                 * Y Y Y Y
                 * V V V V
                 * U U U U
                 */
                capacity = stride * height + stride * height / 4 + stride * height / 4
                planeArray = intArrayOf(stride, stride, 0)
                ImageFormat.IMAGE_FORMAT_YV12
            }
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar -> { // NV12
                /*
                 * Y Y Y Y
                 * Y Y Y Y
                 * Y Y Y Y
                 * Y Y Y Y
                 * U V U V
                 * U V U V
                 */
                capacity = stride * height + stride * height / 2
                planeArray = intArrayOf(stride, stride, 0)
                ImageFormat.IMAGE_FORMAT_NV12
            }
            MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420PackedSemiPlanar -> {  //NV21
                /*
                 * Y Y Y Y
                 * Y Y Y Y
                 * Y Y Y Y
                 * Y Y Y Y
                 * V U V U
                 * V U V U
                 */
                capacity = stride * height + stride * height / 2
                planeArray = intArrayOf(stride, stride, 0)
                ImageFormat.IMAGE_FORMAT_NV21
            }
            else -> {
                // 默认RGBA
                capacity = stride * height * 3
                planeArray = intArrayOf(stride, stride, stride)
                ImageFormat.IMAGE_FORMAT_RGBA
            }
        }

        KLog.d(HardwareDecoder.TAG, "VideoOutputFormat[format:$format")

        pool.initRecycledPool {
            MediaFrame(
                0,
                0,
                imageFormat,
                ByteBuffer.allocateDirect(capacity),
                planeArray,
                false,
                width,
                height,
                0,
                0,
                0,
                true
            )
        }
    }
}