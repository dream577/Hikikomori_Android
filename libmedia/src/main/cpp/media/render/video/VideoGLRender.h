//
// Created by 郝留凯 on 2023/2/13.
//

#ifndef HIKIKOMORI_VIDEOGLRENDER_H
#define HIKIKOMORI_VIDEOGLRENDER_H

#include "VideoRender.h"
#include "WindowSurface.h"

#include <thread>

#define MATH_PI 3.1415926535897932384626433832802
#define TEXTURE_NUM 3

using namespace std;

class VideoGLRender : public VideoRender {

private:
    ANativeWindow *m_NativeWindow = nullptr;

    WindowSurface *m_WindowSurface = nullptr;

    GLuint m_Program = GL_NONE;
    GLuint m_TextureId[3];

    GLuint m_VaoId;
    GLuint m_VboIds[3];

    thread *m_thread = nullptr;

    static void StartRenderLoop(VideoGLRender *render);

    void doRenderLoop();

public:
    VideoGLRender(JNIEnv *env, jobject surface, RenderCallback *callback) : VideoRender(callback) {
            m_NativeWindow = ANativeWindow_fromSurface(env, surface);
            m_WindowSurface = new WindowSurface(m_NativeWindow);
    }

    virtual int init() override;

    virtual int unInit() override;

    int destroy() override;

    virtual void OnSurfaceCreated() override;

    virtual void OnSurfaceChanged(int w, int h) override;

    virtual void OnSurfaceDestroyed() override;

    virtual void renderVideoFrame(Frame *frame) override;

    virtual void startRenderThread() override;
};


#endif //HIKIKOMORI_VIDEOGLRENDER_H
