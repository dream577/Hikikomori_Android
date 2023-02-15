//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_VIDEORENDER_H
#define HIKIKOMORI_VIDEORENDER_H

#include "Render.h"
#include "looper.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <jni.h>

enum GLMessage {
    MESSAGE_ON_SURFACE_CREATED = 0,
    MESSAGE_ON_SURFACE_CHANGED,
    MESSAGE_ON_SURFACE_DESTROY,
    MESSAGE_START_DRAW_FRAME_LOOP,
};

class VideoRender : public Render, public looper {
private:


protected:
    int *m_VideoSize = nullptr;     // 视频宽高
    int *m_RenderSize = nullptr;    // 渲染画面宽高
    int *m_WindowSize = nullptr;    // 播放窗口宽高

    ANativeWindow *m_NativeWindow = nullptr;

    virtual void onDrawFrame() = 0;

    virtual void onSurfaceCreated() = 0;

    virtual void onSurfaceChanged() = 0;

    virtual void onSurfaceDestroyed() = 0;

public:
    VideoRender(RenderCallback *callback) : Render(callback) {
        m_VideoSize = new int[2];
        m_RenderSize = new int[2];
        m_WindowSize = new int[2];
    }

    virtual ~VideoRender() {
        if (m_VideoSize) {
            delete m_VideoSize;
            m_VideoSize = nullptr;
        }
        if (m_WindowSize) {
            delete m_WindowSize;
            m_WindowSize = nullptr;
        }
        if (m_RenderSize) {
            delete m_RenderSize;
            m_RenderSize = nullptr;
        }
        if (m_NativeWindow) {
            ANativeWindow_release(m_NativeWindow);
        }
    }

    virtual void SetVideoSize(int videoWidth, int videoHeight) {
        m_VideoSize[0] = videoWidth;
        m_VideoSize[1] = videoHeight;
    }

    virtual void OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) {
        m_NativeWindow = ANativeWindow_fromSurface(jniEnv, surface);
        post(MESSAGE_ON_SURFACE_CREATED, nullptr);
    };

    virtual void OnSurfaceChanged(int w, int h) {
        m_WindowSize[0] = w;
        m_WindowSize[1] = h;
        post(MESSAGE_ON_SURFACE_CHANGED, nullptr);
    };

    virtual void OnDrawFrameLoop() {
        post(MESSAGE_START_DRAW_FRAME_LOOP, nullptr);
    }

    virtual void OnSurfaceDestroyed() {
        post(MESSAGE_ON_SURFACE_DESTROY, nullptr, true);
    };

    void handle(int what, void *data) override {
        switch (what) {
            case MESSAGE_ON_SURFACE_CREATED: {
                onSurfaceCreated();
                break;
            }
            case MESSAGE_ON_SURFACE_CHANGED: {
                onSurfaceChanged();
                break;
            }
            case MESSAGE_START_DRAW_FRAME_LOOP: {
                onDrawFrame();
                if (m_Callback->GetPlayerState() != STATE_STOP &&
                    m_Callback->GetPlayerState() != STATE_UNKNOWN &&
                    m_Callback->GetPlayerState() != STATE_ERROR) {
                    post(MESSAGE_START_DRAW_FRAME_LOOP, nullptr, true);
                }
                break;
            }
            case MESSAGE_ON_SURFACE_DESTROY: {
                onSurfaceDestroyed();
                break;
            }
            default:;
        }
    }
};


#endif //HIKIKOMORI_VIDEORENDER_H
