//
// Created by bronyna on 2023/2/7.
//

#ifndef HIKIKOMORI_VIDEONATIVERENDER_H
#define HIKIKOMORI_VIDEONATIVERENDER_H

#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <string.h>

extern "C" {
#include "libavutil/imgutils.h"
#include "libavutil/frame.h"
#include "libswscale/swscale.h"
};

#include "VideoRender.h"

using namespace std;

class VideoNativeRender : public VideoRender {
public:
    VideoNativeRender(AVPixelFormat avPixelFormat, RenderCallback *callback)
            : VideoRender(callback) {
        m_PixelFormat = avPixelFormat;
    }

    virtual ~VideoNativeRender() {
    }

    int init() override;

    int unInit() override;

protected:
    virtual void onDrawFrame() override;

    virtual void onSurfaceCreated() override;

    virtual void onSurfaceChanged() override;

    virtual void onSurfaceDestroyed() override;

private:
    ANativeWindow_Buffer m_NativeWindowBuffer;
    AVPixelFormat m_PixelFormat;
    AVFrame *m_RGBAFrame = nullptr;
    uint8_t *m_FrameBuffer = nullptr;
    int m_BufferSize = 0;
    SwsContext *m_SwsContext = nullptr;
};


#endif //HIKIKOMORI_VIDEONATIVERENDER_H
