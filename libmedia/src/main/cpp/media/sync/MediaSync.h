//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_MEDIASYNC_H
#define HIKIKOMORI_MEDIASYNC_H

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/time.h"
};

#include <stdint.h>

#define SYNC_DELAY_THRESHOLD 100    //100ms

#define SYNC_INITIAL_TIMESTAMP      -1

class MediaSync {

public:
    MediaSync() {};

    ~MediaSync();

    void audioSyncToSystemClock(long pts);

    void videoSyncToSystemClock(long pts);

    void videoSynToAudioClock();

    void syncTimeStampWhenResume();

    void audioSeekToPositionSuccess();

    void videoSeekToPositionSuccess();

private:
    long m_CurrAudioTimeStamp = SYNC_INITIAL_TIMESTAMP;         // 当前音频帧的时间戳
    long m_AudioStartTime = SYNC_INITIAL_TIMESTAMP;             // 音频开始播放时的时间戳

    long m_CurrVideoTimeStamp = SYNC_INITIAL_TIMESTAMP;         // 当前视频帧的时间戳
    long m_VideoStartTime = SYNC_INITIAL_TIMESTAMP;             // 视频始播放时的时间戳
};


#endif //HIKIKOMORI_MEDIASYNC_H
