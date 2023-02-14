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
    VideoNativeRender(ANativeWindow *nativeWindow, AVPixelFormat avPixelFormat,
                      RenderCallback *callback) : VideoRender(callback) {
        m_NativeWindow = nativeWindow;
        m_PixelFormat = avPixelFormat;
    }

    VideoNativeRender(RenderCallback *callback) : VideoRender(callback) {}

    virtual ~VideoNativeRender();

    virtual void OnSurfaceCreated() override;

    virtual void OnSurfaceChanged(int w, int h) override;

    virtual void OnDrawFrame() override;

    virtual void OnSurfaceDestroyed() override;

    void SetNativeWindow(ANativeWindow *window) {
        m_NativeWindow = window;
    }

    void SetAvPixelFormat(AVPixelFormat format) {
        m_PixelFormat = format;
    }

private:
    ANativeWindow_Buffer m_NativeWindowBuffer;
    ANativeWindow *m_NativeWindow = nullptr;

    AVPixelFormat m_PixelFormat;

    AVFrame *m_RGBAFrame = nullptr;
    uint8_t *m_FrameBuffer = nullptr;
    int m_BufferSize = 0;
    SwsContext *m_SwsContext = nullptr;
};


#endif //HIKIKOMORI_VIDEONATIVERENDER_H
