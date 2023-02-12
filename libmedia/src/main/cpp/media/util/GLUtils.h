//
// Created by bronyna on 2023/2/12.
//

#ifndef HIKIKOMORI_GLUTILS_H
#define HIKIKOMORI_GLUTILS_H

#include "LogUtil.h"

#include <GLES3/gl3.h>
#include <string>
#include <glm.hpp>


class GLUtils {

public:

    static GLuint LoadShader(GLenum shaderType, const char *pSource);

    static GLuint CreateProgram(const char *pVertexShaderSource, const char *pFragShaderSource,
                                GLuint &vertexShaderHandle,
                                GLuint &fragShaderHandle);

    static void DeleteProgram(GLuint &program);

    static void CheckGLError(const char *pGLOperation);
};


#endif //HIKIKOMORI_GLUTILS_H
