//
// Created by bronyna on 2023/2/12.
//

#include "EglSurfaceBase.h"
#include "LogUtil.h"

/**
 * 创建显示的Surface
 * @param nativeWindow
 */
void EglSurfaceBase::createWindowSurface(ANativeWindow *nativeWindow) {
    if (mEglSurface != EGL_NO_SURFACE) {
        LOGCATE("surface already created\n");
        return;
    } else {
        LOGCATE("surface create success\n");
    }
    mEglSurface = mEglCore->createWindowSurface(nativeWindow);
}

/**
 * 创建离屏surface
 * @param width
 * @param height
 */
void EglSurfaceBase::createOffscreenSurface(int width, int height) {
    if (mEglSurface != EGL_NO_SURFACE) {
        LOGCATE("surface already created\n");
        return;
    }
    mEglSurface = mEglCore->createOffscreenSurface(width, height);
    mWidth = width;
    mHeight = height;
}

/**
 * 获取宽度
 * @return
 */
int EglSurfaceBase::getWidth() {
    if (mWidth < 0) {
        return mEglCore->querySurface(mEglSurface, EGL_WIDTH);
    } else {
        return mWidth;
    }
}

/**
 * 获取高度
 * @return
 */
int EglSurfaceBase::getHeight() {
    if (mHeight < 0) {
        return mEglCore->querySurface(mEglSurface, EGL_HEIGHT);
    } else {
        return mHeight;
    }
}

/**
 * 释放EGLSurface
 */
void EglSurfaceBase::releaseEglSurface() {
    mEglCore->releaseSurface(mEglSurface);
    mEglSurface = EGL_NO_SURFACE;
    mWidth = mHeight = -1;
}

/**
 * 切换到当前EGLContext
 */
void EglSurfaceBase::makeCurrent() {
    mEglCore->makeCurrent(mEglSurface);
}

/**
 * 交换到前台显示
 * @return
 */
bool EglSurfaceBase::swapBuffers() {
    bool result = mEglCore->swapBuffers(mEglSurface);
    if (!result) {
        LOGCATE("WARNING: swapBuffers() failed");
    }
    return result;
}

/**
 * 设置当前时间戳
 * @param nsecs
 */
void EglSurfaceBase::setPresentationTime(long nsecs) {
    mEglCore->setPresentationTime(mEglSurface, nsecs);
}

/**
 * 获取当前像素
 * @return
 */
char *EglSurfaceBase::getCurrentFrame() {
    char *pixels = nullptr;
    glReadPixels(0, 0, getWidth(), getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    return pixels;
}

void EglSurfaceBase::OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) {
    LOGCATE("EglSurfaceBase::OnSurfaceCreated")
    mNativeWindow = ANativeWindow_fromSurface(jniEnv, surface);
    mVideoRender = new VideoGLRender(m_Callback);
    mVideoRender->setVideoSize(mVideoWidth, mVideoHeight);
    createWindowSurface(mNativeWindow);
    post(MESSAGE_ON_SURFACE_CREATED, nullptr);
}

void EglSurfaceBase::OnSurfaceChanged(int w, int h) {
    LOGCATE("EglSurfaceBase::OnSurfaceChanged");
    mWindowSize[0] = w;
    mWindowSize[1] = h;
    post(MESSAGE_ON_SURFACE_CHANGED, mWindowSize);
}

void EglSurfaceBase::OnDrawFrame() {
    LOGCATE("EglSurfaceBase::OnDrawFrame");
    post(MESSAGE_ON_DRAW_FRAME, nullptr);
}

void EglSurfaceBase::OnSurfaceDestroyed() {
    LOGCATE("EglSurfaceBase::OnSurfaceDestroyed");
    post(MESSAGE_ON_SURFACE_DESTROY, nullptr, true);
}

void EglSurfaceBase::handle(int what, void *data) {
    switch (what) {
        case MESSAGE_ON_SURFACE_CREATED: {
            makeCurrent();
            mVideoRender->OnSurfaceCreated();
            break;
        }
        case MESSAGE_ON_SURFACE_CHANGED: {
            int *size = (int *) data;
            mVideoRender->OnSurfaceChanged(size[0], size[1]);
            break;
        }
        case MESSAGE_ON_DRAW_FRAME: {
            mVideoRender->OnDrawFrame();
            swapBuffers();
            post(MESSAGE_ON_DRAW_FRAME, nullptr);
            break;
        }
        case MESSAGE_ON_SURFACE_DESTROY: {
            mVideoRender->OnSurfaceDestroyed();
            break;
        }
        default:;
    }
}
