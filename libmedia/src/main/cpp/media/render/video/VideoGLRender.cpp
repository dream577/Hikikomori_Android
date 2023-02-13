//
// Created by 郝留凯 on 2023/2/13.
//

#include "GLUtils.h"
#include "VideoGLRender.h"
#include "LogUtil.h"

static char vShaderStr[] =
        "#version 300 es\n"
        "layout(location = 0) in vec4 a_position;\n"
        "layout(location = 1) in vec2 a_texCoord;\n"
        "out vec2 v_texCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = a_position;\n"
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
        "void main()\n"
        "{\n"
        "   vec3 yuv;\n"
        "   yuv.x = texture2D(s_texture0, v_texCoord).r;\n"
        "   yuv.y = texture2D(s_texture1, v_texCoord).r - 0.5;\n"
        "   yuv.z = texture2D(s_texture2, v_texCoord).r - 0.5;\n"
        "   highp vec3 rgb = mat3(1.0,     1.0,     1.0,\n"
        "                         0.0,     -0.344,  1.770,\n"
        "                         1.403,   -0.714,  0.0) * yuv;\n"
        "   outColor = vec4(rgb, 1.0);\n"
        "}";

GLushort indices[] = {0, 1, 2, 0, 2, 3};

GLfloat verticesCoords[] = {
        -1.0f, 1.0f, 0.0f,  // Position 0
        -1.0f, -1.0f, 0.0f,  // Position 1
        1.0f, -1.0f, 0.0f,  // Position 2
        1.0f, 1.0f, 0.0f,  // Position 3
};

GLfloat textureCoords[] = {
        0.0f, 0.0f,        // TexCoord 0
        0.0f, 1.0f,        // TexCoord 1
        1.0f, 1.0f,        // TexCoord 2
        1.0f, 0.0f         // TexCoord 3
};

void VideoGLRender::OnSurfaceCreated() {
    LOGCATE("VideoGLRender::OnSurfaceCreated");
    m_Program = GLUtils::CreateProgram(vShaderStr, fShaderStr);

    if (!m_Program) {
        LOGCATE("VideoGLRender::OnSurfaceCreated create program fail");
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
    glGenBuffers(3, m_VboIds);
    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCoords), verticesCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Generate VAO Id
    glGenVertexArrays(1, &m_VaoId);
    glBindVertexArray(m_VaoId);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *) 0);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *) 0);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[2]);

    glBindVertexArray(GL_NONE);
}

void VideoGLRender::OnSurfaceChanged(int w, int h) {
    LOGCATE("VideoGLRender::OnSurfaceChanged [w,h]=%d, %d", w, h);
    glViewport(0, 0, w, h);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void VideoGLRender::renderVideoFrame(Frame *frame) {

    auto *videoFrame = (VideoFrame *) frame;
    //upload Y plane data
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureId[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, videoFrame->width,
                 videoFrame->height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                 videoFrame->yuvBuffer[0]);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    //update U plane data
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_TextureId[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, videoFrame->width >> 1,
                 videoFrame->height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                 videoFrame->yuvBuffer[1]);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    //update V plane data
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_TextureId[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, videoFrame->width >> 1,
                 videoFrame->height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                 videoFrame->yuvBuffer[2]);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    // Use the program object
    glUseProgram(m_Program);
    glBindVertexArray(m_VaoId);

    for (int i = 0; i < TEXTURE_NUM; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_TextureId[i]);
        char samplerName[64] = {0};
        sprintf(samplerName, "s_texture%d", i);
        GLUtils::setInt(m_Program, samplerName, i);
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *) 0);

    m_WindowSurface->swapBuffers();
}

void VideoGLRender::OnSurfaceDestroyed() {
    LOGCATE("VideoGLRender::OnSurfaceDestroyed");
    m_Callback->SetPlayerState(STATE_STOP);
    if (m_thread) {
        m_thread->join();
        delete m_thread;
        m_thread = nullptr;
    }
}

void VideoGLRender::startRenderThread() {
    LOGCATE("VideoGLRender::startRenderThread");
    m_thread = new thread(StartRenderLoop, this);
}

void VideoGLRender::StartRenderLoop(VideoGLRender *render) {
    LOGCATE("VideoGLRender::StartRenderLoop");
    render->doRenderLoop();
    render->unInit();
}

void VideoGLRender::doRenderLoop() {
    LOGCATE("VideoGLRender::doRenderLoop");
    m_WindowSurface->makeCurrent();
    while (m_Callback->GetPlayerState() != STATE_STOP) {
        Frame *frame = m_Callback->GetOneFrame(MEDIA_TYPE_VIDEO);
        renderVideoFrame(frame);
    }
}

int VideoGLRender::init() {
    return 0;
}

int VideoGLRender::unInit() {
    return 0;
}

int VideoGLRender::destroy() {
    m_Callback->SetPlayerState(STATE_STOP);
    if (m_thread) {
        m_thread->join();
        delete m_thread;
        m_thread = nullptr;
    }
    return 0;
}
