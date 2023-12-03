//
// Created by bronyna on 2023/2/13.
//

#ifndef HIKIKOMORI_IMAGEGLRENDER_H
#define HIKIKOMORI_IMAGEGLRENDER_H

#include <vec2.hpp>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <GLES3/gl3.h>

#include "MediaDef.h"
#include "AVSurface.h"
#include "NativeAssertManager.h"

#define MATH_PI 3.1415926535897932384626433832802
#define TEXTURE_NUM 3

using namespace std;
using namespace glm;

class ImageGLRender : public AVSurface {
private:
    GLuint m_Program = GL_NONE;
    GLuint m_TextureId[3];

    GLuint m_VaoId;
    GLuint m_VboIds[2];

    TransformMatrix m_transformMatrix;
    mat4 m_MVPMatrix;

    void _DrawGRBA(MediaFrame *frame);

    void _DrawI420(MediaFrame *frame);

    void _DrawNV12orNV21(MediaFrame *frame);

public:
    void OnDrawFrame(MediaFrame *frame);

    void OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) override;

    void OnSurfaceChanged(int width, int height) override;

    void UpdateMVPMatrix(int angleX, int angleY, float scaleX, float scaleY);

    void UpdateMVPMatrix(float translateX, float translateY, float scaleX,
                         float scaleY, int degree, int mirror);

    void OnSurfaceDestroyed() override;

    ~ImageGLRender() {}
};


#endif //HIKIKOMORI_IMAGEGLRENDER_H
