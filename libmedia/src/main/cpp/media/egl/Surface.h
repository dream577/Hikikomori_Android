//
// Created by bronyna on 2023/2/14.
//

#ifndef HIKIKOMORI_SURFACE_H
#define HIKIKOMORI_SURFACE_H

#include "looper.h"
#include "jni.h"
#include <android/native_window.h>
#include "VideoRender.h"

enum GLMessage {
    MESSAGE_ON_SURFACE_CREATED = 0,
    MESSAGE_ON_SURFACE_CHANGED,
    MESSAGE_ON_SURFACE_DESTROY,
    MESSAGE_ON_DRAW_FRAME,
};

class Surface : public looper {
protected:
    ANativeWindow *mNativeWindow;
    int mVideoWidth, mVideoHeight;
    int *mWindowSize;
    VideoRender *mVideoRender;

public:
    Surface() : looper() {
        mWindowSize = new int[2];
    }

    virtual ~Surface() {
        if (mVideoRender) {
            delete mVideoRender;
            mVideoRender = nullptr;
        }
        if (mNativeWindow) {
            ANativeWindow_release(mNativeWindow);
            mNativeWindow = nullptr;
        }
        if (mWindowSize) {
            delete[] mWindowSize;
        }
    }

    void setVideoSize(int width, int height) {
        mVideoWidth = width;
        mVideoHeight = height;
    }

    virtual void OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) = 0;

    virtual void OnSurfaceChanged(int w, int h) = 0;

    virtual void OnDrawFrame() = 0;

    virtual void OnSurfaceDestroyed() = 0;
};

#endif //HIKIKOMORI_SURFACE_H
