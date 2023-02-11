//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_VIDEORENDER_H
#define HIKIKOMORI_VIDEORENDER_H

#include "Render.h"

class VideoRender : public Render {
protected:
    int m_RenderType = VIDEO_RENDER_UNKONWN;
    int m_VideoWidth, m_VideoHeight;
    int m_RenderWidth, m_RenderHeight;

public:
    VideoRender(RenderCallback *callback) : Render(callback) {}

    virtual ~VideoRender() {}

    virtual int init() = 0;

    virtual void renderVideoFrame(Frame *frame) = 0;

    void setVideoSize(int videoWidth, int videoHeight) {
        m_VideoWidth = videoWidth;
        m_VideoHeight = videoHeight;
    }

    int getRenderWidth() {
        return m_RenderWidth;
    }

    int getRenderHeight() {
        return m_RenderHeight;
    }
};


#endif //HIKIKOMORI_VIDEORENDER_H
