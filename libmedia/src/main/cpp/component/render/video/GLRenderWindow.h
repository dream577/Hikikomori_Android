//
// Created by 郝留凯 on 2023/7/2.
//

#ifndef HIKIKOMORI_GLRENDERWINDOW_H
#define HIKIKOMORI_GLRENDERWINDOW_H

#include "looper.h"
#include "MediaDef.h"
#include "LogUtil.h"
#include "VioletEGLSurface.h"
#include "ImageGLRender.h"
#include "Callback.h"

#include <vec2.hpp>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <jni.h>

using namespace std;
using namespace glm;

enum ImageRenderMessage {
    MESSAGE_ON_SURFACE_CREATED,
    MESSAGE_ON_SURFACE_CHANGED,
    MESSAGE_ON_SURFACE_DESTROY,
    MESSAGE_UPDATE_MATRIX,
    MESSAGE_IMAGE_RENDER_LOOP,
    MESSAGE_IMAGE_DRAW_FRAME,
};

class GLRenderWindow : public ImageRenderItf, public looper {
public:
    GLRenderWindow(RenderCallback *callback);

    ~GLRenderWindow();

    void OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) override;

    void OnSurfaceChanged(int width, int height) override;

    void UpdateMVPMatrix(float translateX, float translateY, float scaleX,
                         float scaleY, int degree, int mirror);

    void OnDrawFrame(MediaFrame *frame);

    void OnSurfaceDestroyed() override;

    void Destroy();

    void StartRender();

protected:

private:
    int mLoopMsg = MESSAGE_IMAGE_RENDER_LOOP;
    RenderCallback *m_Callback = nullptr;

    int mImageWidth, mImageHeight;     // 图片宽高
    int mWindowWidth, mWindowHeight;   // 渲染窗口宽高

    float mTranslateX, mTranslateY;
    float mScaleX, mScaleY;
    int mDegree, mMirror;

    JNIEnv *m_JniEnv = nullptr;
    jobject m_JavaSurface = nullptr;
    ANativeWindow *m_NativeWindow = nullptr;
    VioletEGLSurface *m_Surface = nullptr;
    ImageGLRender *glRender = nullptr;

    void onSurfaceCreated();

    void onSurfaceChanged();

    void updateMVPMatrix(float translateX, float translateY, float scaleX, float scaleY, int degree,
                         int mirror);

    void onDrawFrame();

    void onSurfaceDestroyed();

    void handle(int what, void *data) override;
};


#endif //HIKIKOMORI_GLRENDERWINDOW_H
