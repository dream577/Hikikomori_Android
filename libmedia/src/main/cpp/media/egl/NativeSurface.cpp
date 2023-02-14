//
// Created by bronyna on 2023/2/14.
//

#include "NativeSurface.h"
#include "LogUtil.h"

void NativeSurface::OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) {
    LOGCATE("NativeSurface::OnSurfaceCreated");
    mNativeWindow = ANativeWindow_fromSurface(jniEnv, surface);
    mVideoRender = new VideoNativeRender(mNativeWindow, m_AVPixelFormat, m_Callback);
    mVideoRender->setVideoSize(mVideoWidth, mVideoHeight);
    post(MESSAGE_ON_SURFACE_CREATED, nullptr);
}

void NativeSurface::OnSurfaceChanged(int w, int h) {
    LOGCATE("NativeSurface::OnSurfaceChanged");
    mWindowSize[0] = w;
    mWindowSize[1] = h;
    post(MESSAGE_ON_SURFACE_CHANGED, mWindowSize);
}

void NativeSurface::OnDrawFrame() {
    LOGCATE("NativeSurface::OnDrawFrame");
    post(MESSAGE_ON_DRAW_FRAME, nullptr);
}

void NativeSurface::OnSurfaceDestroyed() {
    LOGCATE("NativeSurface::OnSurfaceDestroyed");
    post(MESSAGE_ON_SURFACE_DESTROY, nullptr, true);
}

void NativeSurface::handle(int what, void *data) {
    switch (what) {
        case MESSAGE_ON_SURFACE_CREATED:
            mVideoRender->OnSurfaceCreated();
            break;
        case MESSAGE_ON_SURFACE_CHANGED: {
            int *size = (int *) data;
            mVideoRender->OnSurfaceChanged(size[0], size[1]);
            break;
        }
        case MESSAGE_ON_DRAW_FRAME: {
            mVideoRender->OnDrawFrame();
            post(MESSAGE_ON_DRAW_FRAME, nullptr);
            break;
        }
        case MESSAGE_ON_SURFACE_DESTROY:
            mVideoRender->OnSurfaceDestroyed();
            break;
        default:;
    }
}
