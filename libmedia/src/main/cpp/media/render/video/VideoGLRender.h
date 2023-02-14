//
// Created by bronyna on 2023/2/13.
//

#ifndef HIKIKOMORI_VIDEOGLRENDER_H
#define HIKIKOMORI_VIDEOGLRENDER_H

#include "VideoRender.h"
#include <vec2.hpp>
#include <GLES3/gl3.h>

#define MATH_PI 3.1415926535897932384626433832802
#define TEXTURE_NUM 3

using namespace std;
using namespace glm;

class VideoGLRender : public VideoRender {
private:
    GLuint m_Program = GL_NONE;
    GLuint m_TextureId[3];

    GLuint m_VaoId;
    GLuint m_VboIds[3];

public:
    VideoGLRender(RenderCallback *callback) : VideoRender(callback) {}

    virtual void OnSurfaceCreated() override;

    virtual void OnSurfaceChanged(int w, int h) override;

    virtual void OnSurfaceDestroyed() override;

    virtual void OnDrawFrame() override;
};


#endif //HIKIKOMORI_VIDEOGLRENDER_H
