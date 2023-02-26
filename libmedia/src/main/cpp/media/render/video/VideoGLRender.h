//
// Created by bronyna on 2023/2/13.
//

#ifndef HIKIKOMORI_VIDEOGLRENDER_H
#define HIKIKOMORI_VIDEOGLRENDER_H

#include <vec2.hpp>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "VideoRender.h"
#include "VioletEGLSurface.h"
#include "NativeAssertManager.h"

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

    mat4 m_MVPMatrix;

    void updateMVPMatrix(int angleX, int angleY, float scaleX, float scaleY);

protected:


    virtual int init() override;

    virtual void onDrawFrame() override;

    virtual void onSurfaceCreated() override;

    virtual void onSurfaceChanged() override;

    virtual void updateMVPMatrix() override;

    virtual void onSurfaceDestroyed() override;

    virtual int unInit() override;

public:
    VideoGLRender(RenderCallback *callback) : VideoRender(callback) {}

    ~VideoGLRender() {}
};


#endif //HIKIKOMORI_VIDEOGLRENDER_H
