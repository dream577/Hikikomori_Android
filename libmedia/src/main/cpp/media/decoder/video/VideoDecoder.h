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
    AVPixelFormat DST_PIXEL_FORMAT = AV_PIX_FMT_RGBA;

    bool useNaiveWindow = true;

    int m_VideoWidth, m_VideoHeight;
    int m_RenderWidth, m_RenderHeight;

    AVFrame *m_RGBAFrame = nullptr;
    uint8_t *m_FrameBuffer = nullptr;
    int m_BufferSize = 0;

    SwsContext *m_SwsContext = nullptr;

    virtual void startDecodeThread() override;

    Frame *OnFrameAvailable() override;

    virtual void onDecoderReady() override;

public:
    VideoDecoder(char *path, bool useNativeWindow, DecoderCallback *callback)
            : FFBaseDecoder(path, AVMEDIA_TYPE_VIDEO, callback) {
        VideoDecoder::useNaiveWindow = useNativeWindow;
    }

    virtual ~VideoDecoder();

    virtual int init() override;

    virtual int unInit() override;

    int getVideoWidth() {
        return m_VideoWidth;
    }

    int getVideoHeight() {
        return m_VideoHeight;
    }

    void setRenderSize(int renderWidth, int renderHeight) {
        m_RenderWidth = renderWidth;
        m_RenderHeight = renderHeight;
    }
};


#endif //HIKIKOMORI_VIDEODECODER_H
