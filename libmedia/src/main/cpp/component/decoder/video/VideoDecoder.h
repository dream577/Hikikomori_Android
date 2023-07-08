//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_VIDEODECODER_H
#define HIKIKOMORI_VIDEODECODER_H

extern "C" {
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
};


#include "FFBaseDecoder.h"

class VideoDecoder : public FFBaseDecoder {
private:
    int m_VideoWidth, m_VideoHeight;

    Frame *onFrameAvailable() override;

    virtual int init() override;

    virtual int unInit() override;

public:
    VideoDecoder(char *path, MediaEventCallback *eventCallback, DecoderCallback *callback)
            : FFBaseDecoder(path, AVMEDIA_TYPE_VIDEO, callback) {
        m_EventCallback = eventCallback;
    }

    virtual ~VideoDecoder() {}

    int getVideoWidth() {
        return m_VideoWidth;
    }

    int getVideoHeight() {
        return m_VideoHeight;
    }
};


#endif //HIKIKOMORI_VIDEODECODER_H
