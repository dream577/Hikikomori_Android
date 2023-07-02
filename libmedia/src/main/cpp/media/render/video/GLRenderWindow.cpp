//
// Created by 郝留凯 on 2023/7/2.
//

#include "GLRenderWindow.h"

void GLRenderWindow::OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) {
    this->m_JniEnv = jniEnv;
    this->m_JavaSurface = surface;
    m_NativeWindow = ANativeWindow_fromSurface(jniEnv, surface);
    post(MESSAGE_ON_SURFACE_CREATED, nullptr);
};

void GLRenderWindow::OnSurfaceChanged(int width, int height) {
    mWindowWidth = width;
    mWindowHeight = height;
    post(MESSAGE_ON_SURFACE_CHANGED, nullptr);
}

void GLRenderWindow::UpdateMVPMatrix(float translateX, float translateY, float scaleX,
                                     float scaleY, int degree, int mirror) {

    this->mTranslateX = translateX;
    this->mTranslateY = translateY;
    this->mScaleX = scaleX;
    this->mScaleY = scaleY;
    this->mDegree = degree;
    this->mMirror = mirror;
    post(MESSAGE_UPDATE_MATRIX, nullptr);
}

void GLRenderWindow::OnDrawFrame(VideoFrame *frame) {
    if (glRender) {
        glRender->OnDrawFrame(frame);
    }
    if (m_Surface) {
        m_Surface->swapBuffers();
    }
}

void GLRenderWindow::OnSurfaceDestroyed() {
    post(MESSAGE_ON_SURFACE_DESTROY, nullptr, true);
}

void GLRenderWindow::StartRender() {
    enableAutoLoop(&mLoopMsg);
}

void GLRenderWindow::onSurfaceCreated() {
    m_Surface = new VioletEGLSurface();
    m_Surface->createWindowSurface(m_NativeWindow);
    m_Surface->makeCurrent();
    if (glRender == nullptr) {
        glRender = new ImageGLRender();
    }
    glRender->OnSurfaceCreated(m_JniEnv, m_JavaSurface);
}

void GLRenderWindow::onSurfaceChanged() {
    LOGCATE("GLRenderWindow::onSurfaceChanged [w,h]=%d, %d", mWindowWidth, mWindowHeight);
    int x, y;
    int rWidth, rHeight;
    int iWidth, iHeight;
    int wWidth, wHeight;

    if (mImageWidth == -1) mImageWidth = mWindowWidth;
    if (mImageHeight == -1) mImageHeight = mWindowHeight;
    if ((mDegree / 90) % 2 != 0 && mDegree % 90 == 0) {
        iWidth = mImageHeight;
        iHeight = mImageWidth;
    } else {
        iWidth = mImageWidth;
        iHeight = mImageHeight;
    }
    wWidth = mWindowWidth;
    wHeight = mWindowHeight;

    if (wWidth < wHeight * iWidth / iHeight) {
        rWidth = wWidth;
        rHeight = wWidth * iHeight / iWidth;
    } else {
        rWidth = wHeight * iWidth / iHeight;
        rHeight = wHeight;
    }

    LOGCATE("GLRenderWindow::onSurfaceChanged window[w,h]=[%d, %d], image[w, h]=[%d, %d], real[w, h]=[%d, %d]",
            mWindowWidth, mWindowHeight, mImageWidth, mImageHeight, rWidth, rHeight);

    x = (mWindowWidth - rWidth) / 2;
    y = (mWindowHeight - rHeight) / 2;

    // 自适应画面居中
    glViewport(x, y, rWidth, rHeight);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    if (glRender) {
        glRender->OnSurfaceChanged(rWidth, rHeight);
    }
}

void GLRenderWindow::updateMVPMatrix(float translateX, float translateY, float scaleX, float scaleY,
                                     int degree, int mirror) {
    if (glRender) {
        glRender->UpdateMVPMatrix(translateX, translateY, scaleX, scaleY, degree, mirror);
    }
}

void GLRenderWindow::onDrawFrame() {
    VideoFrame *frame = nullptr;
    if (m_Callback) {
        frame = (VideoFrame *) m_Callback->GetOneFrame(MEDIA_TYPE_VIDEO);
    }
    if (frame) {
        if (frame->width != mImageWidth || frame->height != mImageHeight) {
            mImageWidth = frame->width;
            mImageHeight = frame->height;
            onSurfaceChanged();
        }
        if (glRender) {
            glRender->OnDrawFrame(frame);
        }
        if (m_Surface) {
            m_Surface->swapBuffers();
        }
    }
}

void GLRenderWindow::onSurfaceDestroyed() {
    LOGCATE("GLRenderWindow::onSurfaceDestroyed")
    if (glRender) {
        glRender->OnSurfaceDestroyed();
        glRender = nullptr;
    }
    if (m_Surface) {
        m_Surface->releaseEglSurface();
        delete m_Surface;
        m_Surface = nullptr;
    }
    if (m_NativeWindow) {
        ANativeWindow_release(m_NativeWindow);
        m_NativeWindow = nullptr;
    }
}

void GLRenderWindow::Destroy() {
    // 1. 终止自动循环
    // 2. 执行卸载程序
    // 3. 退出循环
    disableAutoLoop();
    post(MESSAGE_ON_SURFACE_DESTROY, nullptr);
    quit();
}

void GLRenderWindow::handle(int what, void *data) {
    looper::handle(what, data);
    switch (what) {
        case MESSAGE_ON_SURFACE_CREATED: {
            onSurfaceCreated();
            break;
        }
        case MESSAGE_ON_SURFACE_CHANGED: {
            onSurfaceChanged();
            break;
        }
        case MESSAGE_IMAGE_RENDER_LOOP: {
            onDrawFrame();
            break;
        }
        case MESSAGE_IMAGE_DRAW_FRAME: {
            onDrawFrame();
            break;
        }
        case MESSAGE_UPDATE_MATRIX: {
            updateMVPMatrix(mTranslateX, mTranslateY, mScaleX, mScaleY, mDegree, mMirror);
            break;
        }
        case MESSAGE_ON_SURFACE_DESTROY: {
            onSurfaceDestroyed();
            break;
        }
        default:;
    }
}

