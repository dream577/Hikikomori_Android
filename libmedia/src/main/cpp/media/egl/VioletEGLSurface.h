//
// Created by bronyna on 2023/2/12.
//

#ifndef EGL_HELPER_EGLSURFACEBASE_H
#define EGL_HELPER_EGLSURFACEBASE_H

#include "EglCore.h"

class VioletEGLSurface {
public:

    VioletEGLSurface() {
        mEglCore = new EglCore;
        mEglSurface = EGL_NO_SURFACE;
    };

    ~VioletEGLSurface() {
        if (mEglSurface != EGL_NO_SURFACE) {
            releaseEglSurface();
        }
        if (mEglCore) {
            delete mEglCore;
        }
    }

    // 获取宽度
    int getWidth();

    // 获取高度
    int getHeight();

    // 释放EGLSurface
    void releaseEglSurface();

    // 切换到当前上下文
    void makeCurrent();

    // 交换缓冲区，显示图像
    bool swapBuffers();

    // 设置显示时间戳
    void setPresentationTime(long nsecs);

    // 获取当前帧缓冲
    char *getCurrentFrame();

    // 创建窗口Surface
    void createWindowSurface(ANativeWindow *nativeWindow);

    // 创建离屏Surface
    void createOffscreenSurface(int width, int height);

protected:
    EglCore *mEglCore;
    EGLSurface mEglSurface;
    int mWidth, mHeight;
};


#endif //EGL_HELPER_EGLSURFACEBASE_H
