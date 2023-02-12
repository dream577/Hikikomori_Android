//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_VIDEORENDER_H
#define HIKIKOMORI_VIDEORENDER_H

#include "Render.h"

class VideoRender : public Render {
protected:
    int m_VideoWidth, m_VideoHeight;
    int m_RenderWidth, m_RenderHeight;
    int m_WindowWidth, m_WindowHeight;

public:
    VideoRender(RenderCallback *callback) : Render(callback) {}

    virtual ~VideoRender() {}

    virtual void OnSurfaceCreated() = 0;

    virtual void OnSurfaceChanged(int w, int h) = 0;

    virtual void OnSurfaceDestroyed() = 0;

    virtual void renderVideoFrame(Frame *frame) = 0;

    void setVideoSize(int videoWidth, int videoHeight) {
        m_VideoWidth = videoWidth;
        m_VideoHeight = videoHeight;
    }
};


#endif //HIKIKOMORI_VIDEORENDER_H
