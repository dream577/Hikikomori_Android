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

#define DELAY_THRESHOLD 100 //100ms

class MediaSync {

public:
    void audioSyncToSystemClock(long dts, long pts);

    void videoSyncToSystemClock(long dts, long pts);

    void videoSynToAudioClock();

private:
    long m_CurrAudioTimeStamp = -1;         // 当前音频帧的时间戳
    long m_AudioStartTime = -1;             // 音频开始播放时的时间戳

    long m_CurrVideoTimeStamp = -1;         // 当前视频帧的时间戳
    long m_VideoStartTime = -1;             // 视频始播放时的时间戳
};


#endif //HIKIKOMORI_MEDIASYNC_H
