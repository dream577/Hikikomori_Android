//
// Created by bronyna on 2023/2/12.
//

#include "VioletEGLSurface.h"
#include "LogUtil.h"

/**
 * 创建显示的Surface
 * @param nativeWindow
 */
void VioletEGLSurface::createWindowSurface(ANativeWindow *nativeWindow) {
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
void VioletEGLSurface::createOffscreenSurface(int width, int height) {
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
int VioletEGLSurface::getWidth() {
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
int VioletEGLSurface::getHeight() {
    if (mHeight < 0) {
        return mEglCore->querySurface(mEglSurface, EGL_HEIGHT);
    } else {
        return mHeight;
    }
}

/**
 * 释放EGLSurface
 */
void VioletEGLSurface::releaseEglSurface() {
    mEglCore->releaseSurface(mEglSurface);
    mEglSurface = EGL_NO_SURFACE;
    mWidth = mHeight = -1;
}

/**
 * 切换到当前EGLContext
 */
void VioletEGLSurface::makeCurrent() {
    mEglCore->makeCurrent(mEglSurface);
}

/**
 * 交换到前台显示
 * @return
 */
bool VioletEGLSurface::swapBuffers() {
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
void VioletEGLSurface::setPresentationTime(long nsecs) {
    mEglCore->setPresentationTime(mEglSurface, nsecs);
}

/**
 * 获取当前像素
 * @return
 */
char *VioletEGLSurface::getCurrentFrame() {
    char *pixels = nullptr;
    glReadPixels(0, 0, getWidth(), getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    return pixels;
}
