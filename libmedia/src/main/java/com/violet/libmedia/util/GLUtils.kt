package com.violet.libmedia.util

import android.opengl.GLES30
import com.violet.libbasetools.util.KLog

object GLUtils {
    const val TAG = "GLUtils"

    fun loadShader(type: Int, shaderSource: String): Int {
        val array = IntArray(1)
        // 1.创建一个着色器
        val shader = GLES30.glCreateShader(type)
        if (shader != 0) {
            // 2.提供着色器源码
            GLES30.glShaderSource(shader, shaderSource)
            // 3.编译着色器
            GLES30.glCompileShader(shader)
            // 4.获取着色器是否编译成功
            GLES30.glGetShaderiv(shader, GLES30.GL_COMPILE_STATUS, array, 0)
            if (array[0] == 0) {
                // 5.如果编译失败，获取编译失败的日志信息并删除着色器
                GLES30.glGetShaderiv(shader, GLES30.GL_INFO_LOG_LENGTH, array, 0)
                if (array[0] != 0) {
                    val info = GLES30.glGetShaderInfoLog(shader)
                    KLog.d(TAG, info)
                }
                GLES30.glDeleteShader(shader)
            }
        }
        return shader
    }

    fun createProgram(vertexShaderSource: String, fragmentShaderSource: String): Int {
        var program = 0
        var error = 0
        val array = IntArray(1)
        do {
            // 1.加载顶点着色器
            val vertexShader = loadShader(GLES30.GL_VERTEX_SHADER, vertexShaderSource)
            if (vertexShader == 0) {
                KLog.d(TAG, "load vertex shader failure")
                break
            }
            // 2.加载片段着色器
            val fragmentShader = loadShader(GLES30.GL_FRAGMENT_SHADER, fragmentShaderSource)
            if (fragmentShader == 0) {
                KLog.d(TAG, "load fragment shader failure")
                break
            }
            // 3.创建program对象
            program = GLES30.glCreateProgram()
            if (program == 0) {
                KLog.d(TAG, "glCreateProgram error")
                break
            }
            // 4.将着色器与program对象链接
            GLES30.glAttachShader(program, vertexShader)
            error = GLES30.glGetError()
            if (error != GLES30.GL_NO_ERROR) {
                KLog.d(TAG, "attach vertex shader error")
                break
            }
            GLES30.glAttachShader(program, fragmentShader)
            error = GLES30.glGetError()
            if (error != GLES30.GL_NO_ERROR) {
                KLog.d(TAG, "attach fragment shader error")
                break
            }

            GLES30.glLinkProgram(program)
            GLES30.glGetProgramiv(program, GLES30.GL_LINK_STATUS, array, 0)

            GLES30.glDetachShader(program, vertexShader)
            GLES30.glDeleteShader(vertexShader)
            GLES30.glDetachShader(program, fragmentShader)
            GLES30.glDeleteShader(fragmentShader)
            if (array[0] != GLES30.GL_TRUE) {
                GLES30.glGetProgramiv(program, GLES30.GL_INFO_LOG_LENGTH, array, 0)
                if (array[0] != 0) {
                    val info = GLES30.glGetProgramInfoLog(program)
                    KLog.d(TAG, info)
                }
            }

        } while (false)
        return program
    }
}