//
// Created by bronyna on 2023/2/12.
//

#include "WindowSurface.h"
#include "LogUtil.h"

void WindowSurface::release() {
    releaseEglSurface();
}

void WindowSurface::recreate(EglCore *eglCore) {
    if (mNativeWindow == nullptr) {
        LOGCATE("not yet implemented ANativeWindow");
        return;
    }
    mEglCore = eglCore;
    createWindowSurface(mNativeWindow);
}