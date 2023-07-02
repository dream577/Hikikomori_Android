//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_MEDIASYNC_H
#define HIKIKOMORI_MEDIASYNC_H

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/time.h"
}

#include "MediaDef.h"

#include <stdint.h>
#include <math.h>

#define MIN_SYNC_THRESHOLD       10
#define MAX_SYNC_THRESHOLD       100
#define SYNC_FRAMEDUP_THRESHOLD  40

class MediaSync {

public:
    void SyncAudio(Frame *frame);

    void SyncVideo(Frame *frame);

private:
    long m_CurrAudioPts = 0; // 当前音频帧的时间戳

    long m_LastVideoPts = 0; // 上一帧视频的时间戳
};


#endif //HIKIKOMORI_MEDIASYNC_H
