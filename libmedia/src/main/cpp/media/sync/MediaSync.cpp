//
// Created by bronyna on 2023/2/5.
//
#include "MediaSync.h"
#include "LogUtil.h"

void MediaSync::audioSyncToSystemClock(long pts) {
    m_CurrAudioTimeStamp = pts;

    if (m_AudioStartTime == SYNC_INITIAL_TIMESTAMP) {
        m_VideoStartTime = GetSysCurrentTime() - pts;
    }
    long elapsedTime = GetSysCurrentTime() - m_AudioStartTime;
    long delay = 0;
    if (m_CurrAudioTimeStamp > elapsedTime) {
        auto sleepTime = m_CurrAudioTimeStamp - elapsedTime;
        sleepTime = sleepTime > SYNC_DELAY_THRESHOLD ? SYNC_DELAY_THRESHOLD : sleepTime;
        av_usleep(sleepTime * 1000);
    }
    delay = elapsedTime - m_CurrAudioTimeStamp;
    LOGCATE("MediaSync::audioSyncToSystemClock pts=%ld, 音频播放时长:%ld", pts, m_CurrAudioTimeStamp)
}

void MediaSync::videoSyncToSystemClock(long pts) {
    m_CurrVideoTimeStamp = pts;

    if (m_VideoStartTime == SYNC_INITIAL_TIMESTAMP) {
        m_VideoStartTime = GetSysCurrentTime() - pts;
    }
    long elapsedTime = GetSysCurrentTime() - m_VideoStartTime;
    long delay = 0;
    if (m_CurrVideoTimeStamp > elapsedTime) {
        auto sleepTime = m_CurrVideoTimeStamp - elapsedTime;
        sleepTime = sleepTime > SYNC_DELAY_THRESHOLD ? SYNC_DELAY_THRESHOLD : sleepTime;
        av_usleep(sleepTime * 1000);
    }
    delay = elapsedTime - m_CurrVideoTimeStamp;
    LOGCATE("MediaSync::videoSyncToSystemClock pts=%ld, 视频播放时长:%ld", pts, m_CurrVideoTimeStamp)
}

void MediaSync::videoSynToAudioClock() {

}

void MediaSync::syncTimeStampWhenResume() {
    m_AudioStartTime = GetSysCurrentTime() - m_CurrAudioTimeStamp;
    m_VideoStartTime = GetSysCurrentTime() - m_CurrVideoTimeStamp;
    if (m_AudioStartTime != SYNC_INITIAL_TIMESTAMP && m_VideoStartTime != SYNC_INITIAL_TIMESTAMP) {
        m_VideoStartTime = m_AudioStartTime;
    }
}

void MediaSync::audioSeekToPositionSuccess() {
    m_AudioStartTime = SYNC_INITIAL_TIMESTAMP;
}

void MediaSync::videoSeekToPositionSuccess() {
    m_VideoStartTime = SYNC_INITIAL_TIMESTAMP;
}

MediaSync::~MediaSync() {
    LOGCATE("MediaSync::~MediaSync")
}


