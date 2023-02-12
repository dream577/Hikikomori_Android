#include "OffscreenSurface.h"

OffscreenSurface::OffscreenSurface(int width, int height){
    createOffscreenSurface(width, height);
}

void OffscreenSurface::release() {
    releaseEglSurface();
}


