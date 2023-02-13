//
// Created by bronyna on 2023/2/7.
//

#ifndef HIKIKOMORI_VIDEONATIVERENDER_H
#define HIKIKOMORI_VIDEONATIVERENDER_H

#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <string.h>
#include <thread>

extern "C" {
#include "libavutil/imgutils.h"
#include "libavutil/frame.h"
#include "libswscale/swscale.h"
};

#include "VideoRender.h"

using namespace std;

class VideoNativeRender : public VideoRender {
public:
    VideoNativeRender(JNIEnv *env, jobject surface, AVPixelFormat avPixelFormat,
                      RenderCallback *callback) : VideoRender(callback) {
        m_NativeWindow = ANativeWindow_fromSurface(env, surface);
        m_PixelFormat = avPixelFormat;
    }

    virtual ~VideoNativeRender();

    virtual int init() override;

    virtual int unInit() override;

    virtual int destroy() override;

    virtual void startRenderThread() override;

    virtual void OnSurfaceCreated() override;

    virtual void OnSurfaceChanged(int w, int h) override;

    virtual void OnSurfaceDestroyed() override;

private:
    ANativeWindow_Buffer m_NativeWindowBuffer;
    ANativeWindow *m_NativeWindow = nullptr;

    AVPixelFormat m_PixelFormat;

    AVFrame *m_RGBAFrame = nullptr;
    uint8_t *m_FrameBuffer = nullptr;
    int m_BufferSize = 0;
    SwsContext *m_SwsContext = nullptr;

    virtual void renderVideoFrame(Frame *frame) override;

    static void StartRenderLoop(VideoNativeRender *render);

    void doRenderLoop();

    thread *m_thread = nullptr;
};


#endif //HIKIKOMORI_VIDEONATIVERENDER_H
