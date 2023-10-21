//
// Created by bronyna on 2023/2/13.
//

#include "GLUtils.h"
#include "ImageGLRender.h"

static char vShaderStr[] =
        "#version 300 es\n"
        "layout(location = 0) in vec4 a_position;\n"
        "layout(location = 1) in vec2 a_texCoord;\n"
        "uniform mat4 u_MVPMatrix;\n"
        "out vec2 v_texCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = u_MVPMatrix * a_position;\n"
        "    v_texCoord = a_texCoord;\n"
        "}";

static char fShaderStr[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec2 v_texCoord;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "uniform sampler2D s_texture0;\n"
        "uniform sampler2D s_texture1;\n"
        "uniform sampler2D s_texture2;\n"
        "uniform int u_nImgType;// 1:RGBA, 2:NV21, 3:NV12, 4:I420\n"
        "\n"
        "void main()\n"
        "{\n"
        "\n"
        "    if(u_nImgType == 1) //RGBA\n"
        "    {\n"
        "        outColor = texture(s_texture0, v_texCoord);\n"
        "    }\n"
        "    else if(u_nImgType == 2) //NV21\n"
        "    {\n"
        "        vec3 yuv;\n"
        "        yuv.x = texture(s_texture0, v_texCoord).r;\n"
        "        yuv.y = texture(s_texture1, v_texCoord).a - 0.5;\n"
        "        yuv.z = texture(s_texture1, v_texCoord).r - 0.5;\n"
        "        highp vec3 rgb = mat3(1.0,      1.0,     1.0,\n"
        "                              0.0,     -0.344,   1.770,\n"
        "                              1.403,   -0.714,   0.0) * yuv;\n"
        "        outColor = vec4(rgb, 1.0);\n"
        "\n"
        "    }\n"
        "    else if(u_nImgType == 3) //NV12\n"
        "    {\n"
        "        vec3 yuv;\n"
        "        yuv.x = texture(s_texture0, v_texCoord).r;\n"
        "        yuv.y = texture(s_texture1, v_texCoord).r - 0.5;\n"
        "        yuv.z = texture(s_texture1, v_texCoord).a - 0.5;\n"
        "        highp vec3 rgb = mat3(1.0,      1.0,     1.0,\n"
        "                              0.0,     -0.344,   1.770,\n"
        "                              1.403,   -0.714,   0.0) * yuv;\n"
        "        outColor = vec4(rgb, 1.0);\n"
        "    }\n"
        "    else if(u_nImgType == 4) //I420\n"
        "    {\n"
        "        vec3 yuv;\n"
        "        yuv.x = texture(s_texture0, v_texCoord).r;\n"
        "        yuv.y = texture(s_texture1, v_texCoord).r - 0.5;\n"
        "        yuv.z = texture(s_texture2, v_texCoord).r - 0.5;\n"
        "        highp vec3 rgb = mat3(1.0,     1.0,     1.0,\n"
        "                              0.0,    -0.344,   1.770,\n"
        "                              1.403,  -0.714,   0.0) * yuv;\n"
        "        outColor = vec4(rgb, 1.0);\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        outColor = vec4(1.0);\n"
        "    }\n"
        "}";

GLushort indices[] = {0, 1, 2, 0, 2, 3};

GLfloat verticesCoords[] = {
        -1.0f,   1.0f,  0.0f,  0.0f,  0.0f,
        -1.0f,  -1.0f,  0.0f,  0.0f,  1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 0.0f
};

GLfloat textureCoords[] = {
        0.0f, 0.0f,        // TexCoord 0
        0.0f, 1.0f,        // TexCoord 1
        1.0f, 1.0f,        // TexCoord 2
        1.0f, 0.0f         // TexCoord 3
};

void ImageGLRender::OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) {
    LOGCATE("ImageGLRender::onSurfaceCreated");
    m_Program = GLUtils::CreateProgram(vShaderStr, fShaderStr);

    if (!m_Program) {
        LOGCATE("ImageGLRender::OnSurfaceCreated create program fail")
        return;
    }

    glGenTextures(TEXTURE_NUM, m_TextureId);
    for (int i = 0; i < TEXTURE_NUM; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_TextureId[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(m_TextureId[i], GL_NONE);
    }

    // Generate VBO Ids and load the VBOs with data
    glGenBuffers(2, m_VboIds);
    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCoords), verticesCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Generate VAO Id
    glGenVertexArrays(1, &m_VaoId);
    glBindVertexArray(m_VaoId);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void *) 0);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                          (const void *) (3 * (sizeof(GLfloat))));
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[1]);

    glBindVertexArray(GL_NONE);
}

void ImageGLRender::OnSurfaceChanged(int width, int height) {
}

void ImageGLRender::UpdateMVPMatrix(int angleX, int angleY, float scaleX, float scaleY) {
    LOGCATE("ImageGLRender::updateMVPMatrix [%d,%d,%f,%f]", angleX, angleY, scaleX, scaleY)


    angleX = angleX % 360;
    angleY = angleY % 360;

    //转化为弧度角
    float radiansX = static_cast<float>(MATH_PI / 180.0f * angleX);
    float radiansY = static_cast<float>(MATH_PI / 180.0f * angleY);
    // Projection matrix
    glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
    //glm::mat4 Projection = glm::frustum(-ratio, ratio, -1.0f, 1.0f, 4.0f, 100.0f);
    //glm::mat4 Projection = glm::perspective(45.0f,ratio, 0.1f,100.f);

    // View matrix
    glm::mat4 View = glm::lookAt(
            glm::vec3(0, 0, 4), // Camera is at (0,0,1), in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // Model matrix
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::scale(Model, glm::vec3(scaleX, scaleY, 1.0f));
    Model = glm::rotate(Model, radiansX, glm::vec3(1.0f, 0.0f, 0.0f));
    Model = glm::rotate(Model, radiansY, glm::vec3(0.0f, 1.0f, 0.0f));
    Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));

    m_MVPMatrix = Projection * View * Model;
}

void ImageGLRender::UpdateMVPMatrix(float translateX, float translateY, float scaleX,
                                    float scaleY, int degree, int mirror) {
    LOGCATE("ImageGLRender::updateMVPMatrix [angleX=%d,angleY=%d,scaleX=%f,scaleY=%f,degree=%d,mirror=%d]",
            m_transformMatrix.angleX, m_transformMatrix.angleY, m_transformMatrix.scaleX,
            m_transformMatrix.scaleY, m_transformMatrix.degree, m_transformMatrix.mirror)

    m_transformMatrix.translateX = translateX;
    m_transformMatrix.translateY = translateY;
    m_transformMatrix.scaleX = scaleX;
    m_transformMatrix.scaleY = scaleY;
    m_transformMatrix.degree = degree;
    m_transformMatrix.mirror = mirror;

    //转化为弧度角
    float radiansX = static_cast<float>(MATH_PI / 180.0f * m_transformMatrix.angleX);
    float radiansY = static_cast<float>(MATH_PI / 180.0f * m_transformMatrix.angleY);

    float fFactorX = 1.0f;
    float fFactorY = 1.0f;

    if (m_transformMatrix.mirror == 1) {
        fFactorX = -1.0f;
    } else if (m_transformMatrix.mirror == 2) {
        fFactorY = -1.0f;
    }

    float fRotate = MATH_PI * m_transformMatrix.degree * 1.0f / 180;
    if (m_transformMatrix.mirror == 0) {
        if (m_transformMatrix.degree == 270) {
            fRotate = MATH_PI * 0.5;
        } else if (m_transformMatrix.degree == 180) {
            fRotate = MATH_PI;
        } else if (m_transformMatrix.degree == 90) {
            fRotate = MATH_PI * 1.5;
        }
    } else if (m_transformMatrix.mirror == 1) {
        if (m_transformMatrix.degree == 90) {
            fRotate = MATH_PI * 0.5;
        } else if (m_transformMatrix.degree == 180) {
            fRotate = MATH_PI;
        } else if (m_transformMatrix.degree == 270) {
            fRotate = MATH_PI * 1.5;
        }
    }

    glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
    glm::mat4 View = glm::lookAt(
            glm::vec3(0, 0, 1), // Camera is at (0,0,1), in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0) // Head is up (set to 0,-1,0 to look upside-down)
    );

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::scale(Model, glm::vec3(fFactorX * m_transformMatrix.scaleX,
                                        fFactorY * m_transformMatrix.scaleY, 1.0f));
    Model = glm::rotate(Model, fRotate, glm::vec3(0.0f, 0.0f, 1.0f));
    Model = glm::rotate(Model, radiansX, glm::vec3(1.0f, 0.0f, 0.0f));
    Model = glm::rotate(Model, radiansY, glm::vec3(0.0f, 1.0f, 0.0f));
    Model = glm::translate(Model,
                           glm::vec3(m_transformMatrix.translateX, m_transformMatrix.translateY,
                                     0.0f));

    LOGCATE("ImageGLRender::updateMVPMatrix rotate %d,%.2f,%0.5f,%0.5f,%0.5f,%0.5f,",
            m_transformMatrix.degree, fRotate,
            m_transformMatrix.translateX, m_transformMatrix.translateY,
            fFactorX * m_transformMatrix.scaleX, fFactorY * m_transformMatrix.scaleY);

    m_MVPMatrix = Projection * View * Model;
}

void ImageGLRender::OnDrawFrame(MediaFrame *frame) {
//    LOGCATE("ImageGLRender::onDrawFrame");
    switch (frame->format) {
        case IMAGE_FORMAT_RGBA:
            DrawGRBA(frame);
            break;
        case IMAGE_FORMAT_I420:
            DrawI420(frame);
            break;
        case IMAGE_FORMAT_NV12:
        case IMAGE_FORMAT_NV21:
            DrawNV12orNV21(frame);
            break;
        default:;
    }

    // Use the program object
    glUseProgram(m_Program);
    glBindVertexArray(m_VaoId);
    GLUtils::setMat4(m_Program, "u_MVPMatrix", m_MVPMatrix);

    for (int i = 0; i < TEXTURE_NUM; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_TextureId[i]);
        char samplerName[64] = {0};
        sprintf(samplerName, "s_texture%d", i);
        GLUtils::setInt(m_Program, samplerName, i);
    }

    GLUtils::setInt(m_Program, "u_nImgType", frame->format);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *) 0);
}

void ImageGLRender::DrawGRBA(MediaFrame *videoFrame) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureId[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, videoFrame->width, videoFrame->height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, videoFrame->plane[0]);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
}

void ImageGLRender::DrawI420(MediaFrame *videoFrame) {
    //upload Y plane data
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureId[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, videoFrame->width,
                 videoFrame->height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                 videoFrame->plane[0]);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    //update U plane data
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_TextureId[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, videoFrame->width >> 1,
                 videoFrame->height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                 videoFrame->plane[1]);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    //update V plane data
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_TextureId[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, videoFrame->width >> 1,
                 videoFrame->height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                 videoFrame->plane[2]);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
}

void ImageGLRender::DrawNV12orNV21(MediaFrame *videoFrame) {
    // update Y plane data
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureId[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, videoFrame->width,
                 videoFrame->height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                 videoFrame->plane[0]);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    // update UV plane data
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_TextureId[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, videoFrame->width >> 1,
                 videoFrame->height >> 1, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
                 videoFrame->plane[1]
    );
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
}

void ImageGLRender::OnSurfaceDestroyed() {
    LOGCATE("ImageGLRender::onSurfaceDestroyed")
    glDeleteVertexArrays(1, &m_VaoId);
    glDeleteBuffers(2, m_VboIds);
    glDeleteTextures(3, m_TextureId);
    if (m_Program != GL_NONE) {
        glDeleteProgram(m_Program);
    }
}


