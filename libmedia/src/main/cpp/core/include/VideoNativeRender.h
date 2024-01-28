//
// Created by bronyna on 2023/2/7.
//

#ifndef HIKIKOMORI_VIDEONATIVERENDER_H
#define HIKIKOMORI_VIDEONATIVERENDER_H

#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <string.h>

extern "C" {
#include "libavutil/imgutils.h"
#include "libavutil/frame.h"
#include "libswscale/swscale.h"
};

#include "Callback.h"

using namespace std;

class VideoNativeRender {
public:
    VideoNativeRender(ANativeWindow *nativeWindow, AVPixelFormat avPixelFormat, AVRenderCallback *callback) {
        m_NativeWindow = nativeWindow;
        m_PixelFormat = avPixelFormat;
        m_Callback = callback;
    }

    virtual ~VideoNativeRender() {
    }

    int init();

    int unInit();

protected:
    virtual void onDrawFrame();

    virtual void onSurfaceCreated();

    virtual void onSurfaceChanged(int width, int height);

    virtual void onSurfaceDestroyed();

    virtual void updateMVPMatrix();

private:
    int mImageWidth, mImageHeight;     // 图片宽高
    int mWindowWidth, mWindowHeight;   // 渲染窗口宽高
    int mRenderWidth, mRenderHeight;

    ANativeWindow *m_NativeWindow = nullptr;
    ANativeWindow_Buffer m_NativeWindowBuffer;
    AVPixelFormat m_PixelFormat;
    AVFrame *m_RGBAFrame = nullptr;
    uint8_t *m_FrameBuffer = nullptr;
    int m_BufferSize = 0;
    SwsContext *m_SwsContext = nullptr;

    AVRenderCallback *m_Callback = nullptr;
};


#endif //HIKIKOMORI_VIDEONATIVERENDER_H
