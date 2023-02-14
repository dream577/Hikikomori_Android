//
// Created by bronyna on 2023/2/12.
//

#ifndef EGL_HELPER_WINDOWSURFACE_H
#define EGL_HELPER_WINDOWSURFACE_H

#include "EglSurfaceBase.h"

class WindowSurface : public EglSurfaceBase {
public:
    WindowSurface(int vWidth, int vHeight, RenderCallback *callback)
            : EglSurfaceBase(vWidth, vHeight, callback) {};

    ~WindowSurface() {
        release();
    }

    // 释放资源
    void release();

    // 重新创建
    void recreate(EglCore *eglCore);
};


#endif //EGL_HELPER_WINDOWSURFACE_H
