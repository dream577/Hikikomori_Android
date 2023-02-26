//
// Created by 郝留凯 on 2023/2/26.
//

#ifndef HIKIKOMORI_OPENGLSAMPLERENDER_H
#define HIKIKOMORI_OPENGLSAMPLERENDER_H

#include "VideoRender.h"
#include "VioletEGLSurface.h"
#include "GLUtils.h"
#include "BaseSample.h"
#include "LogUtil.h"

enum OpenGLSample {
    SAMPLE_TYPE_TRIANGLE = 0,
};

namespace opengles_example {

    enum OpenGLSampleRenderMessage {
        MESSAGE_LOAD_SAMPLE = 100,
    };

    class OpenGLSampleRender : public VideoRender {
    private:
        VioletEGLSurface *mSurface = nullptr;

        BaseSample *mSample = nullptr;
        int mCurrentSampleType = 0;
        int mAnotherSampleType = 0;

        void swapBuffers() {
            mSurface->swapBuffers();
        }

    protected:
        virtual int init() override {
            return 0;
        }

        virtual void loadSample() {
            LOGCATE("OpenGLSampleRender::loadSample %d", mAnotherSampleType);
            if (mAnotherSampleType != mCurrentSampleType && mSample) {
                mSample->UnInit();
                delete mSample;
            }
            mCurrentSampleType = mAnotherSampleType;
            switch (mAnotherSampleType) {
                case SAMPLE_TYPE_TRIANGLE:
                    mSample = new TriangleSample();
                    break;
            }
            if (mSample) mSample->Init();
        }

        virtual void onDrawFrame() override {
            LOGCATE("OpenGLSampleRender::onDrawFrame");
            if (mSample) {
                mSample->OnDrawFrame();
                swapBuffers();
            }
        }

        virtual void onSurfaceCreated() override {
            LOGCATE("OpenGLSampleRender::onSurfaceCreated");
            mSurface = new VioletEGLSurface();
            mSurface->createWindowSurface(m_NativeWindow);
            mSurface->makeCurrent();
        }

        virtual void onSurfaceChanged() override {
            LOGCATE("OpenGLSampleRender::onSurfaceChanged");
            glViewport(0, 0, mWindowWidth, mWindowHeight);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        }

        virtual void updateMVPMatrix() override {
            LOGCATE("OpenGLSampleRender::updateMVPMatrix");
        }

        virtual void onSurfaceDestroyed() override {
            LOGCATE("OpenGLSampleRender::onSurfaceDestroyed");
            if (mSample) {
                mSample->UnInit();
                delete mSample;
            }
            if (mSurface) {
                mSurface->releaseEglSurface();
                delete mSurface;
                mSurface = nullptr;
            }
        }

    public:
        OpenGLSampleRender() : VideoRender(nullptr) {}

        void LoadSample(int type) {
            mAnotherSampleType = type;
            post(MESSAGE_LOAD_SAMPLE, nullptr);
        }

        void OnDrawFrame() {
            post(MESSAGE_VIDEO_DRAW_FRAME, nullptr);
        }

        void handle(int what, void *data) override {
            VideoRender::handle(what, data);
            if (what == MESSAGE_LOAD_SAMPLE) {
                loadSample();
            }
        }

    };
}
// opengles_example

#endif //HIKIKOMORI_OPENGLSAMPLERENDER_H
