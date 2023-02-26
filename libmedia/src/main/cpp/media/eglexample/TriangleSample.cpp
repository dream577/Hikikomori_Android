//
// Created by bronyna on 2023/2/26.
//

#include "TriangleSample.h"
#include "LogUtil.h"

namespace opengles_example {

    static GLfloat vVertices[] = {
            -0.5, -0.5, 0.0,
            0.5, -0.5, 0.0,
            0.0, 0.5, 0.0
    };

    void TriangleSample::Init() {
        char *vShader = VioletAssertManager::GetInstance()->GetAssertFile(
                "vshader/TriangleVShader.glsl");
        char *fShader = VioletAssertManager::GetInstance()->GetAssertFile(
                "fshader/TriangleFShader.glsl");
        m_Program = GLUtils::CreateProgram(vShader, fShader);
        delete vShader;
        delete fShader;
    }

    void TriangleSample::OnDrawFrame() {
        glUseProgram(m_Program);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
        glEnableVertexAttribArray(0);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    void TriangleSample::UnInit() {
        if (m_Program != GL_NONE) {
            glDeleteProgram(m_Program);
        }
    }
}