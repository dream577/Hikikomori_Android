//
// Created by bronyna on 2023/2/13.
//

#ifndef HIKIKOMORI_VIDEOGLRENDER_H
#define HIKIKOMORI_VIDEOGLRENDER_H

#include "VideoRender.h"
#include <vec2.hpp>
#include "VioletEGLSurface.h"

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

    VioletEGLSurface *m_Surface;
protected:
    virtual void onDrawFrame() override;

    virtual void onSurfaceCreated() override;

    virtual void onSurfaceChanged() override;

    virtual void onSurfaceDestroyed() override;

public:
    VideoGLRender(RenderCallback *callback) : VideoRender(callback) {}

    ~VideoGLRender() {
        quit();
    }
};


#endif //HIKIKOMORI_VIDEOGLRENDER_H
