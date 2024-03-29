//
// Created by bronyna on 2023/2/12.
//

#ifndef HIKIKOMORI_GLUTILS_H
#define HIKIKOMORI_GLUTILS_H

#include "LogUtil.h"

#include <GLES3/gl3.h>
#include <string>
#include "glm.hpp"

class GLUtils {

public:

    static GLuint LoadShader(GLenum shaderType, const char *pSource) {
        GLuint shader = 0;
        shader = glCreateShader(shaderType);
        if (shader) {
            glShaderSource(shader, 1, &pSource, NULL);
            glCompileShader(shader);
            GLint compiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            if (!compiled) {
                GLint infoLen = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                if (infoLen) {
                    char *buf = (char *) malloc((size_t) infoLen);
                    if (buf) {
                        glGetShaderInfoLog(shader, GL_INFO_LOG_LENGTH, &infoLen, buf);
                        LOGCATE("GLUtils::LoadShader Could not compile shader %d:\n%s\n",
                                shaderType, buf);
                        free(buf);
                    }
                    glDeleteShader(shader);
                    shader = 0;
                }
            }
        }
        return shader;
    }

    static GLuint CreateProgram(const char *pVertexShaderSource, const char *pFragShaderSource,
                                  GLuint &vertexShaderHandle, GLuint &fragShaderHandle) {
        GLuint program = 0;
        vertexShaderHandle = LoadShader(GL_VERTEX_SHADER, pVertexShaderSource);
        if (!vertexShaderHandle) return program;
        fragShaderHandle = LoadShader(GL_FRAGMENT_SHADER, pFragShaderSource);
        if (!fragShaderHandle) return program;
        program = glCreateProgram();
        if (program) {
            glAttachShader(program, vertexShaderHandle);
            CheckGLError("glAttachShader");
            glAttachShader(program, fragShaderHandle);
            CheckGLError("glAttachShader");

            glLinkProgram(program);
            GLint linkStatus = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

            glDetachShader(program, vertexShaderHandle);
            glDeleteShader(vertexShaderHandle);
            vertexShaderHandle = 0;
            glDetachShader(program, fragShaderHandle);
            glDeleteShader(fragShaderHandle);
            fragShaderHandle = 0;

            if (linkStatus != GL_TRUE) {
                GLint bufLength = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
                if (bufLength) {
                    char *buf = (char *) malloc((size_t) bufLength);
                    if (buf) {
                        glGetProgramInfoLog(program, bufLength, NULL, buf);
                        LOGCATE("GLUtils::CreateProgram Could not link program:\n%s\n", buf);
                        free(buf);
                    }
                }
            }
        }
        LOGCATE("GLUtils::CreateProgram program = %d", program);
        return program;
    }

    static void DeleteProgram(GLuint &program) {
        LOGCATE("GLUtils::DeleteProgram");
        if (program) {
            glUseProgram(0);
            glDeleteProgram(program);
            program = 0;
        }
    }

    static void CheckGLError(const char *pGLOperation) {
        for (GLint error = glGetError(); error; error = glGetError()) {
            LOGCATE("GLUtils::CheckGLError GL Operation %s() glError (0x%x)\n", pGLOperation, error);
        }
    }

    static GLuint CreateProgram(const char *pVertexShaderSource, const char *pFragShaderSource) {
        GLuint vertexShaderHandle, fragShaderHandle;
        return CreateProgram(pVertexShaderSource, pFragShaderSource, vertexShaderHandle,
                             fragShaderHandle);
    }

    static void setBool(GLuint programId, const std::string &name, bool value) {
        glUniform1i(glGetUniformLocation(programId, name.c_str()), (int) value);
    }

    static void setInt(GLuint programId, const std::string &name, int value) {
        glUniform1i(glGetUniformLocation(programId, name.c_str()), value);
    }

    static void setFloat(GLuint programId, const std::string &name, float value) {
        glUniform1f(glGetUniformLocation(programId, name.c_str()), value);
    }

    static void setVec2(GLuint programId, const std::string &name, const glm::vec2 &value) {
        glUniform2fv(glGetUniformLocation(programId, name.c_str()), 1, &value[0]);
    }

    static void setVec2(GLuint programId, const std::string &name, float x, float y) {
        glUniform2f(glGetUniformLocation(programId, name.c_str()), x, y);
    }

    static void setVec3(GLuint programId, const std::string &name, const glm::vec3 &value) {
        glUniform3fv(glGetUniformLocation(programId, name.c_str()), 1, &value[0]);
    }

    static void setVec3(GLuint programId, const std::string &name, float x, float y, float z) {
        glUniform3f(glGetUniformLocation(programId, name.c_str()), x, y, z);
    }

    static void setVec4(GLuint programId, const std::string &name, const glm::vec4 &value) {
        glUniform4fv(glGetUniformLocation(programId, name.c_str()), 1, &value[0]);
    }

    static void
    setVec4(GLuint programId, const std::string &name, float x, float y, float z, float w) {
        glUniform4f(glGetUniformLocation(programId, name.c_str()), x, y, z, w);
    }

    static void setMat2(GLuint programId, const std::string &name, const glm::mat2 &mat) {
        glUniformMatrix2fv(glGetUniformLocation(programId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    static void setMat3(GLuint programId, const std::string &name, const glm::mat3 &mat) {
        glUniformMatrix3fv(glGetUniformLocation(programId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    static void setMat4(GLuint programId, const std::string &name, const glm::mat4 &mat) {
        glUniformMatrix4fv(glGetUniformLocation(programId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    static glm::vec3 texCoordToVertexCoord(glm::vec2 texCoord) {
        return glm::vec3(2 * texCoord.x - 1, 1 - 2 * texCoord.y, 0);
    }
};


#endif //HIKIKOMORI_GLUTILS_H
