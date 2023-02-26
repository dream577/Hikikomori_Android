//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_VIDEORENDER_H
#define HIKIKOMORI_VIDEORENDER_H

#include "Render.h"
#include "looper.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <jni.h>

enum VideoRenderMessage {
    MESSAGE_VIDEO_RENDER_INIT = 0,
    MESSAGE_ON_SURFACE_CREATED,
    MESSAGE_ON_SURFACE_CHANGED,
    MESSAGE_ON_SURFACE_DESTROY,
    MESSAGE_UPDATE_MATRIX,
    MESSAGE_VIDEO_RENDER_LOOP,
    MESSAGE_VIDEO_RENDER_UNINIT
};

struct TransformMatrix {
    int degree;
    int mirror;
    float translateX;
    float translateY;
    float scaleX;
    float scaleY;
    int angleX;
    int angleY;

    TransformMatrix() : translateX(0), translateY(0), scaleX(1.0), scaleY(1.0),
                        degree(0), mirror(0), angleX(0), angleY(0) {}

    void Reset() {
        translateX = 0;
        translateY = 0;
        scaleX = 1.0;
        scaleY = 1.0;
        degree = 0;
        mirror = 0;
    }
};

class VideoRender : public Render, public looper {
private:
    int mLoopMsg = MESSAGE_VIDEO_RENDER_LOOP;

protected:
    int mVideoWidth, mVideoHeight;
    int mRenderWidth, mRenderHeight;
    int mWindowWidth, mWindowHeight;

    ANativeWindow *m_NativeWindow = nullptr;
    TransformMatrix m_transformMatrix;

    int result = -1;
    sem_t runBlock;

    virtual int init() = 0;

    virtual void onDrawFrame() = 0;

    virtual void onSurfaceCreated() = 0;

    virtual void onSurfaceChanged() = 0;

    virtual void updateMVPMatrix() = 0;

    virtual void onSurfaceDestroyed() = 0;

    virtual int unInit() {
        if (m_NativeWindow) {
            ANativeWindow_release(m_NativeWindow);
            m_NativeWindow = nullptr;
        }
        return 0;
    }

public:
    VideoRender(RenderCallback *callback) : Render(callback) {
        sem_init(&runBlock, 0, 0);
        mVideoWidth = mVideoHeight = -1;
    }

    virtual ~VideoRender() {
        sem_destroy(&runBlock);
    }

    virtual void SetVideoSize(int videoWidth, int videoHeight) {
        mVideoWidth = videoWidth;
        mVideoHeight = videoHeight;
    }

    virtual int Init() override {
        post(MESSAGE_VIDEO_RENDER_INIT, nullptr);
        sem_wait(&runBlock);
        return result;
    }

    virtual void OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) {
        m_NativeWindow = ANativeWindow_fromSurface(jniEnv, surface);
        post(MESSAGE_ON_SURFACE_CREATED, nullptr);
    };

    virtual void OnSurfaceChanged(int w, int h) {
        mWindowWidth = w;
        mWindowHeight = h;
        post(MESSAGE_ON_SURFACE_CHANGED, nullptr);
    };

    virtual void UpdateMVPMatrix(float translateX, float translateY, float scaleX,
                                 float scaleY, int degree, int mirror) {

        m_transformMatrix.translateX = translateX;
        m_transformMatrix.translateY = translateY;
        m_transformMatrix.scaleX = scaleX;
        m_transformMatrix.scaleY = scaleY;
        m_transformMatrix.degree = degree;
        m_transformMatrix.mirror = mirror;
        post(MESSAGE_UPDATE_MATRIX, nullptr);
    }

    virtual void OnSurfaceDestroyed() {
        post(MESSAGE_ON_SURFACE_DESTROY, nullptr, true);
    };

    virtual void StartRenderLoop() override {
        enableAutoLoop(&mLoopMsg);
    }

    virtual int UnInit() override {
        // 1. 终止自动循环
        // 2. 执行卸载程序
        // 3. 退出循环
        // 4. 父类卸载
        disableAutoLoop();
        post(MESSAGE_ON_SURFACE_DESTROY, nullptr);
        post(MESSAGE_VIDEO_RENDER_UNINIT, nullptr);
        quit();
        return 0;
    }

    void handle(int what, void *data) override {
        looper::handle(what, data);
        switch (what) {
            case MESSAGE_VIDEO_RENDER_INIT:
                result = init();
                break;
            case MESSAGE_ON_SURFACE_CREATED: {
                onSurfaceCreated();
                break;
            }
            case MESSAGE_ON_SURFACE_CHANGED: {
                onSurfaceChanged();
                break;
            }
            case MESSAGE_VIDEO_RENDER_LOOP: {
                onDrawFrame();
                break;
            }
            case MESSAGE_UPDATE_MATRIX: {
                updateMVPMatrix();
                break;
            }
            case MESSAGE_ON_SURFACE_DESTROY: {
                onSurfaceDestroyed();
                break;
            }
            case MESSAGE_VIDEO_RENDER_UNINIT:
                unInit();
                break;
            default:;
        }
    }
};


#endif //HIKIKOMORI_VIDEORENDER_H
