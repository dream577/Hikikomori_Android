//
// Created by bronyna on 2023/2/12.
//

#ifndef HIKIKOMORI_EGLCORE_H
#define HIKIKOMORI_EGLCORE_H

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3platform.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#define    FLAG_RECORDABLE           0x01
#define    FLAG_TRY_GLES3            0x02
#define    EGL_RECORDABLE_ANDROID    0x3142

typedef EGLBoolean (EGLAPIENTRYP EGL_PRESENTATION_TIME_ANDROIDPROC)(EGLDisplay display,
                                                                    EGLSurface surface,
                                                                    khronos_stime_nanoseconds_t time);

class EglCore {
private:
    EGLDisplay mEGLDisplay = EGL_NO_DISPLAY;
    EGLConfig mEGLConfig = nullptr;
    EGLContext mEGLContext = EGL_NO_CONTEXT;

    int mGLVersion = -1;

    EGLConfig getConfig(int flags, int version);

    // 设置时间戳方法
    EGL_PRESENTATION_TIME_ANDROIDPROC eglPresentationTimeANDROID = nullptr;

public:

    EglCore();

    EglCore(EglCore *eglCore);  // 以EglCore为参数，新创建出来的对象与原对象共享EGL环境

    ~EglCore();

    EglCore(EGLContext sharedContext, int flags);

    bool init(EGLContext sharedContext, int flags);

    void release();

    EGLContext getEGLContext();

    EGLSurface createWindowSurface(ANativeWindow *window);

    EGLSurface createOffscreenSurface(int width, int height);

    bool makeCurrent(EGLSurface eglSurface);

    bool makeCurrent(EGLSurface eglSurface, EGLSurface readSurface);

    bool makeNothingCurrent();

    bool swapBuffers(EGLSurface eglSurface);

    void setPresentationTime(EGLSurface eglSurface, long nsecs);

    bool isCurrent(EGLSurface eglSurface);

    int querySurface(EGLSurface eglSurface, int what);

    const char *queryString(int what);

    int glVersion();

    void releaseSurface(EGLSurface eglSurface);

};


#endif //HIKIKOMORI_EGLCORE_H
