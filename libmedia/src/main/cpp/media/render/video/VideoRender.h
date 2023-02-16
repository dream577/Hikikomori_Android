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

enum RenderMessage {
    MESSAGE_ON_SURFACE_CREATED = 0,
    MESSAGE_ON_SURFACE_CHANGED,
    MESSAGE_ON_SURFACE_DESTROY,
    MESSAGE_RENDER_LOOP,
};

class VideoRender : public Render, public looper {
private:
    int mLoopMsg = MESSAGE_RENDER_LOOP;

protected:
    int mVideoWidth, mVideoHeight;
    int mRenderWidth, mRenderHeight;
    int mWindowWidth, mWindowHeight;

    ANativeWindow *m_NativeWindow = nullptr;

    virtual void onDrawFrame() = 0;

    virtual void onSurfaceCreated() = 0;

    virtual void onSurfaceChanged() = 0;

    virtual void onSurfaceDestroyed() = 0;

public:
    VideoRender(RenderCallback *callback) : Render(callback) {

    }

    virtual ~VideoRender() {
        if (m_NativeWindow) {
            ANativeWindow_release(m_NativeWindow);
        }
    }

    virtual void SetVideoSize(int videoWidth, int videoHeight) {
        mVideoWidth = videoWidth;
        mVideoHeight = videoHeight;
    }

    virtual void OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) {
        m_NativeWindow = ANativeWindow_fromSurface(jniEnv, surface);
        post(MESSAGE_ON_SURFACE_CREATED, nullptr);
    };

    virtual void OnSurfaceChanged(int w, int h) {
        mWindowWidth = w;
        mWindowHeight = h;
        post(MESSAGE_ON_SURFACE_CHANGED, nullptr);
    };

    virtual void StartRenderLoop() {
        enableAutoLoop(&mLoopMsg);
    }

    virtual void OnSurfaceDestroyed() {
        post(MESSAGE_ON_SURFACE_DESTROY, nullptr, true);
    };

    void handle(int what, void *data) override {
        looper::handle(what, data);
        switch (what) {
            case MESSAGE_ON_SURFACE_CREATED: {
                onSurfaceCreated();
                break;
            }
            case MESSAGE_ON_SURFACE_CHANGED: {
                onSurfaceChanged();
                break;
            }
            case MESSAGE_RENDER_LOOP: {
                onDrawFrame();
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
