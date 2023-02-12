//
// Created by bronyna on 2023/2/7.
//

#ifndef HIKIKOMORI_NATIVEVIDEORENDER_H
#define HIKIKOMORI_NATIVEVIDEORENDER_H

#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <string.h>
#include <thread>

#include "VideoRender.h"

using namespace std;

class NativeVideoRender : public VideoRender {
public:
    NativeVideoRender(JNIEnv *env, jobject surface, RenderCallback *callback) : VideoRender(
            callback) {
        m_NativeWindow = ANativeWindow_fromSurface(env, surface);
        m_RenderType = VIDEO_RENDER_ANWINDOW;
    }

    virtual ~NativeVideoRender();

    virtual int init() override;

    virtual int destroy() override;

    virtual void startRenderThread() override;

protected:
    virtual int unInit() override;

private:
    ANativeWindow_Buffer m_NativeWindowBuffer;
    ANativeWindow *m_NativeWindow = nullptr;

    virtual void renderVideoFrame(Frame *frame) override;

    static void StartRenderLoop(NativeVideoRender *render);

    void doRenderLoop();

    thread *m_thread = nullptr;
};


#endif //HIKIKOMORI_NATIVEVIDEORENDER_H
