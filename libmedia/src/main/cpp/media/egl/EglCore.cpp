//
// Created by bronyna on 2023/2/12.
//

#include "EglCore.h"
#include "LogUtil.h"

EglCore::EglCore() {

}

EglCore::~EglCore() {

}

bool EglCore::init(EGLContext sharedContext, int flags) {
    LOGCATE("EglCore::init")
    if (sharedContext == nullptr) {
        sharedContext = EGL_NO_CONTEXT;
    }
    mEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (mEGLDisplay == EGL_NO_DISPLAY) {
        LOGCATE("EglCore::init eglGetDisplay error=%d", eglGetError());
        return false;
    }
    if (!eglInitialize(mEGLDisplay, 0, 0)) {
        LOGCATE("EglCore::init eglInitialize error=%d", eglGetError())
        return false;
    }

    if ((flags & FLAG_TRY_GLES3) != 0) {
        EGLConfig config = getConfig(flags, 3);
        if (config != nullptr) {
            int attrib3_list[] = {
                    EGL_CONTEXT_CLIENT_VERSION, 3,
                    EGL_NONE
            };
            EGLContext context = eglCreateContext(mEGLDisplay, config, sharedContext,
                                                  attrib3_list);
            if (eglGetError() == EGL_SUCCESS) {
                mEGLConfig = config;
                mEGLContext = context;
                mGLVersion = 3;
            }
        }
    }

    if (mEGLContext == EGL_NO_CONTEXT) {
        EGLConfig config = getConfig(flags, 2);
        int attrib2_list[] = {
                EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL_NONE
        };
        EGLContext context = eglCreateContext(mEGLDisplay, config, sharedContext,
                                              attrib2_list);
        if (eglGetError() == EGL_SUCCESS) {
            mEGLConfig = config;
            mEGLContext = context;
            mGLVersion = 2;
        }
    }

    // 获取eglPresentationTimeANDROID方法的地址
    eglPresentationTimeANDROID = (EGL_PRESENTATION_TIME_ANDROIDPROC)
            eglGetProcAddress("eglPresentationTimeANDROID");
    if (!eglPresentationTimeANDROID) {
        LOGCATE("eglPresentationTimeANDROID is not available!");
    }

    int values[1] = {0};
    eglQueryContext(mEGLDisplay, mEGLContext, EGL_CONTEXT_CLIENT_VERSION, values);
    LOGCATE("EGLContext created, client version %d", values[0]);

    return true;
}

EGLConfig EglCore::getConfig(int flags, int version) {
    int renderableType = EGL_OPENGL_ES2_BIT;
    if (version >= 3) {
        renderableType |= EGL_OPENGL_ES3_BIT_KHR;
    }
    int attribList[] = {
            EGL_BUFFER_SIZE, 32,
            EGL_RED_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 16,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, renderableType,
            EGL_NONE, 0,      // placeholder for recordable [@-3]
            EGL_NONE
    };
    int length = sizeof(attribList) / sizeof(attribList[0]);
    if ((flags & FLAG_RECORDABLE) != 0) {
        attribList[length - 3] = EGL_RECORDABLE_ANDROID;
        attribList[length - 2] = 1;
    }
    EGLConfig configs = nullptr;
    int numConfigs;
    if (!eglGetConfigs(mEGLDisplay, &configs, 1, &numConfigs)) {
        LOGCATE("unable to find RGB8888 / %d  EGLConfig", version);
        return nullptr;
    }
    return configs;
}

void EglCore::release() {
    if (mEGLDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(mEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(mEGLDisplay, mEGLContext);
        eglReleaseThread();
        eglTerminate(mEGLDisplay);
    }
    mEGLDisplay = EGL_NO_DISPLAY;
    mEGLContext = EGL_NO_CONTEXT;
    mEGLConfig = nullptr;
}

EGLContext EglCore::getEGLContext() {

    return nullptr;
}

EGLSurface EglCore::createWindowSurface(ANativeWindow *window) {
    EGLSurface surface = nullptr;
    if (window == nullptr) {
        LOGCATE("EglCore::createWindowSurface window is nullptr", eglGetError());
        return surface;
    }
    EGLint format;
    if (!eglGetConfigAttrib(mEGLDisplay, mEGLConfig, EGL_NATIVE_VISUAL_ID, &format)) {
        LOGCATE("EglCore::createWindowSurface returned error %d", eglGetError());
        return surface;
    }
    ANativeWindow_setBuffersGeometry(window, 0, 0, format);
    int surfaceAttribs[] = {EGL_NONE};
    if (!(surface = eglCreateWindowSurface(mEGLDisplay, mEGLConfig, window, surfaceAttribs))) {
        LOGCATE("EglCore::createWindowSurface returned error %d", eglGetError());
        return surface;
    }
    return surface;
}

EGLSurface EglCore::createOffscreenSurface(int width, int height) {
    EGLSurface surface = nullptr;
    EGLint PUbufferAttributes[] = {EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE, EGL_NONE};
    if (!(surface = eglCreatePbufferSurface(mEGLDisplay, mEGLConfig, PUbufferAttributes))) {
        LOGCATE("EglCore::createOffscreenSurface returned error %d", eglGetError());
        return surface;
    }
    return surface;
}

bool EglCore::makeCurrent(EGLSurface eglSurface) {
    if (mEGLDisplay == EGL_NO_DISPLAY) {
        LOGCATE("Note: makeCurrent w/o display.\n");
        return false;
    }
    if (!eglMakeCurrent(mEGLDisplay, eglSurface, eglSurface, mEGLContext)) {
        LOGCATE("EglCore::makeCurrent returned error %d", eglGetError());
        return false;
    }
    return true;
}

bool EglCore::makeCurrent(EGLSurface eglSurface, EGLSurface readSurface) {
    if (mEGLDisplay == EGL_NO_DISPLAY) {
        LOGCATE("Note: makeCurrent w/o display.\n");
        return false;
    }
    if (!eglMakeCurrent(mEGLDisplay, eglSurface, readSurface, mEGLContext)) {
        LOGCATE("EglCore::makeCurrent returned error %d", eglGetError());
        return false;
    }
    return true;
}

bool EglCore::makeNothingCurrent() {
    if (!eglMakeCurrent(mEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
        return false;
    }
    return true;
}

bool EglCore::swapBuffers(EGLSurface eglSurface) {
    return eglSwapBuffers(mEGLDisplay, eglSurface);
}

void EglCore::setPresentationTime(EGLSurface eglSurface, long nsecs) {
    eglPresentationTimeANDROID(mEGLDisplay, eglSurface, nsecs);
}

bool EglCore::isCurrent(EGLSurface eglSurface) {
    return mEGLContext == eglGetCurrentContext() &&
           eglSurface == eglGetCurrentSurface(EGL_DRAW);
}

int EglCore::querySurface(EGLSurface eglSurface, int what) {
    int value;
    eglQuerySurface(mEGLContext, eglSurface, what, &value);
    return value;
}

const char *EglCore::queryString(int what) {
    return eglQueryString(mEGLDisplay, what);
}

int EglCore::glVersion() {
    return mGLVersion;
}

void EglCore::releaseSurface(EGLSurface eglSurface) {
    eglMakeCurrent(mEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(mEGLDisplay, eglSurface);
}
