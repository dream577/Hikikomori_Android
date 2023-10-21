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
#include "LogUtil.h"

#include <stdint.h>
#include <math.h>
#include <memory>

#define MIN_SYNC_THRESHOLD       10
#define MAX_SYNC_THRESHOLD       100
#define SYNC_FRAMEDUP_THRESHOLD  40

class MediaSync {

public:
    void SyncAudio(long pts, int flag);

    void SyncVideo(long pts, int flag);

private:
    long m_CurrAudioPts = 0; // 当前音频帧的时间戳

    long m_LastVideoPts = 0; // 上一帧视频的时间戳
};

class SyncClock {
private:
    bool useSystemClock = false;
    long referenceClock = -1L;
public:
    SyncClock(bool useSystemClock) {
        this->useSystemClock = useSystemClock;
    }

    void setRefClock(long timestamp) {
        if (useSystemClock) {
            referenceClock = GetSysCurrentTime() - timestamp;
        } else {
            referenceClock = timestamp;
        }
    }

    long getRefClock() {
        if (useSystemClock) {
            return GetSysCurrentTime();
        } else {
            return referenceClock;
        }
    }
};

class Synchronizer {
protected:
    std::shared_ptr<SyncClock> clock;

    long lastFramePts;

public:
    Synchronizer(std::shared_ptr<SyncClock> clock) {
        this->clock = clock;
        this->lastFramePts = 0l;
    }

    virtual void sync(std::shared_ptr<MediaFrame> frame) = 0;
};

class AudioSync : public Synchronizer {

};

class VideoSync : public Synchronizer {

};


#endif //HIKIKOMORI_MEDIASYNC_H
