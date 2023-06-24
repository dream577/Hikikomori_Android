package com.violet.libmedia.render.sync

import com.violet.libmedia.model.MediaFrame
import java.lang.Long.max
import kotlin.math.min

/**
 * 当前帧延迟从而使上一帧显示特定的时间
 */
class MediaSync {

    companion object {
        private const val MIN_SYNC_THRESHOLD = 10L
        private const val MAX_SYNC_THRESHOLD = 100L
        private const val SYNC_FRAMEDUP_THRESHOLD = 40L

        private var delayZeroCount = 0
    }


    private var audioPts: Long = 0L
    private var lastVideoPts: Long = 0L

    fun syncMediaFrame(frame: MediaFrame) {
        if (!frame.isVideo) {
            audioPts = frame.pts / 1000
        } else {
            val currentPts = frame.pts / 1000
            // 上一帧视频应该显示的时长
            var delay = currentPts - lastVideoPts
            // 当前视频帧PTS与参考时钟比较，得到音视频差距diff
            val diff = currentPts - audioPts

            val sync_threshold = max(MIN_SYNC_THRESHOLD, min(MAX_SYNC_THRESHOLD, delay))

            if (diff <= -sync_threshold) {
                // 视频比音频慢，加快
                delay = max(0, delay + diff)
            } else if (diff >= sync_threshold && delay > SYNC_FRAMEDUP_THRESHOLD) {
                // 视频比音频快，差距较大，一步到位
                delay += diff
            } else if (diff >= sync_threshold) {
                // 视频比音频快，差距较小，逐渐缩小
                delay *= 2
            }
            Thread.sleep(delay)
            lastVideoPts = currentPts;
        }
    }
}