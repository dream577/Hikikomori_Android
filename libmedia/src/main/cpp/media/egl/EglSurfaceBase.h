//
// Created by bronyna on 2023/2/12.
//

#ifndef EGL_HELPER_EGLSURFACEBASE_H
#define EGL_HELPER_EGLSURFACEBASE_H

#include "EglCore.h"
#include "Surface.h"
#include "VideoGLRender.h"

class EglSurfaceBase : public Surface {
public:

    EglSurfaceBase(int vWidth, int vHeight, RenderCallback *callback) : Surface() {
        mEglCore = new EglCore;
        mEglSurface = EGL_NO_SURFACE;
        m_Callback = callback;
        mVideoWidth = mWidth;
        mVideoHeight = mHeight;
    };

    ~EglSurfaceBase() {
        m_Callback = nullptr;
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

    virtual void OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) override;

    virtual void OnSurfaceChanged(int w, int h) override;

    virtual void OnDrawFrame() override;

    virtual void OnSurfaceDestroyed() override;

    virtual void handle(int what, void *data) override;

protected:
    EglCore *mEglCore;
    EGLSurface mEglSurface;
    int mWidth, mHeight;
    RenderCallback *m_Callback;

    // 创建窗口Surface
    void createWindowSurface(ANativeWindow *nativeWindow);

    // 创建离屏Surface
    void createOffscreenSurface(int width, int height);
};


#endif //EGL_HELPER_EGLSURFACEBASE_H
