#include "WindowSurface.h"
#include "LogUitls"

WindowSurface::WindowSurface(ANativeWindow *window, bool releaseSurface) {
    mSurface = window;
    createWindowSurface(mSurface);
    mReleaseSurface = releaseSurface;
}

WindowSurface::WindowSurface(ANativeWindow *window) {
    createWindowSurface(window);
    mSurface = window;
}

void WindowSurface::release() {
    releaseEglSurface();
    if (mSurface != nullptr) {
        ANativeWindow_release(mSurface);
        mSurface = nullptr;
    }

}

void WindowSurface::recreate(EglCore *eglCore) {
    if (mSurface == nullptr) {
        LOGCATE("not yet implemented ANativeWindow");
        return;
    }
    mEglCore = eglCore;
    createWindowSurface(mSurface);
}