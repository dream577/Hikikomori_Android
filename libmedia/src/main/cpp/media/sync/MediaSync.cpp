//
// Created by bronyna on 2023/2/5.
//
#include "MediaSync.h"
#include "LogUtil.h"

void MediaSync::audioSyncToSystemClock(long dts, long pts) {
    m_CurrAudioTimeStamp = pts;

    if (m_AudioStartTime == -1) {
        m_AudioStartTime = GetSysCurrentTime();
    }
    long elapsedTime = GetSysCurrentTime() - m_AudioStartTime;
    long delay = 0;
    if (m_CurrAudioTimeStamp > elapsedTime) {
        auto sleepTime = m_CurrAudioTimeStamp - elapsedTime;
        sleepTime = sleepTime > DELAY_THRESHOLD ? DELAY_THRESHOLD : sleepTime;
        av_usleep(sleepTime * 1000);
    }
    delay = elapsedTime - m_CurrAudioTimeStamp;
    LOGCATE("MediaSync::audioSyncToSystemClock [dts, pts]=[%ld,%ld], 音频播放时长:%ld", dts, pts,
            m_CurrAudioTimeStamp)
}

void MediaSync::videoSyncToSystemClock(long dts, long pts) {
    m_CurrVideoTimeStamp = pts;

    if (m_VideoStartTime == -1) {
        m_VideoStartTime = GetSysCurrentTime();
    }
    long elapsedTime = GetSysCurrentTime() - m_VideoStartTime;
    long delay = 0;
    if (m_CurrVideoTimeStamp > elapsedTime) {
        auto sleepTime = m_CurrVideoTimeStamp - elapsedTime;
        sleepTime = sleepTime > DELAY_THRESHOLD ? DELAY_THRESHOLD : sleepTime;
        av_usleep(sleepTime * 1000);
    }
    delay = elapsedTime - m_CurrVideoTimeStamp;
    LOGCATE("MediaSync::videoSyncToSystemClock [dts, pts]=[%ld,%ld], 视频播放时长:%ld", dts, pts,
            m_CurrVideoTimeStamp)
}

void MediaSync::videoSynToAudioClock() {

}

