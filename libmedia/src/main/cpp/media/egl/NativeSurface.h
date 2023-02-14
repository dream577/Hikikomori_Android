//
// Created by bronyna on 2023/2/14.
//

#ifndef HIKIKOMORI_NATIVESURFACE_H
#define HIKIKOMORI_NATIVESURFACE_H

#include "Surface.h"
#include "VideoNativeRender.h"

class NativeSurface : public Surface {
private:
    AVPixelFormat m_AVPixelFormat;
    RenderCallback *m_Callback;

public:
    NativeSurface(AVPixelFormat avPixelFormat, RenderCallback *callback) : Surface() {
        m_AVPixelFormat = avPixelFormat;
        m_Callback = callback;
    }

    ~NativeSurface() {
        m_Callback = nullptr;
    }

    virtual void OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) override;

    virtual void OnSurfaceChanged(int w, int h) override;

    virtual void OnDrawFrame() override;

    virtual void OnSurfaceDestroyed() override;

    virtual void handle(int what, void *data) override;
};


#endif //HIKIKOMORI_NATIVESURFACE_H
