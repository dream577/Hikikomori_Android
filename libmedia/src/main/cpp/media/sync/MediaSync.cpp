//
// Created by bronyna on 2023/2/5.
//
#include "MediaSync.h"
#include "LogUtil.h"

void MediaSync::SyncAudio(Frame *frame) {
    m_CurrAudioPts = frame->pts;
}

void MediaSync::SyncVideo(Frame *frame) {
    long currentVideoPts = frame->pts;
    if (frame->seekFlag) {
        m_LastVideoPts = currentVideoPts;
    }

    long delay = currentVideoPts - m_LastVideoPts;
    long diff = currentVideoPts - m_CurrAudioPts;

    long sync_threshold = fmax(MIN_SYNC_THRESHOLD, fmin(MAX_SYNC_THRESHOLD, delay));

    if (diff <= -sync_threshold) {
        // 视频比音频慢，加快
        delay = fmax(0, delay + diff);
    } else if (diff >= sync_threshold && delay > SYNC_FRAMEDUP_THRESHOLD) {
        // 视频比音频快，差距较大，一步到位
        delay += diff;
    } else if (diff >= sync_threshold) {
        // 视频比音频快，差距较小，逐渐缩小
        delay *= 2;
    }
    av_usleep(delay * 1000);
    m_LastVideoPts = frame->pts;
}


