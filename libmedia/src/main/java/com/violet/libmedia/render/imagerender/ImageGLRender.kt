package com.violet.libmedia.render.imagerender

import android.opengl.GLES20.GL_NONE
import android.opengl.GLES30
import android.opengl.Matrix
import android.view.Surface
import com.violet.libbasetools.util.KLog
import com.violet.libmedia.core.GLUtils
import com.violet.libmedia.model.ImageFormat
import com.violet.libmedia.model.MediaFrame
import com.violet.libmedia.util.BufferUtil
import com.violet.libmedia.util.RecycledPool.Element
import com.violet.libmedia.util.ShaderSource
import java.nio.FloatBuffer
import java.nio.ShortBuffer

class ImageGLRender : GLRender {
    companion object {
        const val TAG = "ImageGLRender"

        private val vertexCoords = floatArrayOf(
            -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 0.0f
        )

        private val indices = shortArrayOf(0, 1, 2, 0, 2, 3)
    }

    private var program: Int = 0

    private val vertexCoordBuffer: FloatBuffer
    private val indicBuffer: ShortBuffer

    private val m_TextureIds: IntArray      // 纹理数组
    private val texturesLocation : IntArray

    private val matrixArray : FloatArray
    private var matrixLocation = 0

    private var imageTypeLocation = 0

    private val m_VboIds: IntArray          // 顶点数组缓冲区对象
    private val m_VaoIds: IntArray          // 顶点数组对象

    init {
        vertexCoordBuffer = BufferUtil.createBuffer(vertexCoords)
        indicBuffer = BufferUtil.createBuffer(indices)

        m_TextureIds = intArrayOf(GLES30.GL_NONE, GLES30.GL_NONE, GLES30.GL_NONE)
        texturesLocation = intArrayOf(0, 0, 0)

        m_VboIds = intArrayOf(GLES30.GL_NONE, GLES30.GL_NONE, GLES30.GL_NONE)
        m_VaoIds = intArrayOf(GLES30.GL_NONE)

        matrixArray = FloatArray(16)
    }

    override fun onSurfaceCreated(surface: Surface) {
        // 初始化EGL环境相关
        program = GLUtils.createProgram(
            ShaderSource.IMAGE_VERTEX_SHADER,
            ShaderSource.IMAGE_FRAGMENT_SHADER
        )

        matrixLocation = GLES30.glGetUniformLocation(program, "u_MVPMatrix")
        for (i in 0 until 3) {
            texturesLocation[i] = GLES30.glGetUniformLocation(program, "s_texture$i")
        }
        imageTypeLocation = GLES30.glGetUniformLocation(program, "u_nImgType")
        KLog.d(TAG, "matLoc=$matrixLocation imageTypeLoc=$imageTypeLocation texture0=${texturesLocation[0]} texture1=${texturesLocation[1]} texture2=${texturesLocation[2]}")

        // 创建3个纹理对象
        GLES30.glGenTextures(3, m_TextureIds, 0)
        for (i in 0 until 3) {
            // 激活纹理
            GLES30.glActiveTexture(GLES30.GL_TEXTURE0 + i)
            // 绑定纹理
            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, m_TextureIds[i])
            // 设置过滤方式
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_LINEAR)
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_LINEAR)
            // 设置纹理坐标系s轴和t轴的映射规则
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_CLAMP_TO_EDGE)
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_CLAMP_TO_EDGE)
            // 纹理解绑
            GLES30.glBindTexture(m_TextureIds[i], GLES30.GL_NONE)
        }

        // 创建顶点数组缓冲区对象
        GLES30.glGenBuffers(2, m_VboIds, 0)

        // 顶点数组缓冲区对象0用来保存顶点属性数组
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, m_VboIds[0])
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, vertexCoords.size * 4, vertexCoordBuffer, GLES30.GL_STATIC_DRAW)

        // 元素数组缓存区对象用于保存图元索引
        GLES30.glBindBuffer(GLES30.GL_ELEMENT_ARRAY_BUFFER, m_VboIds[1])
        GLES30.glBufferData(GLES30.GL_ELEMENT_ARRAY_BUFFER, indices.size * 2, indicBuffer, GLES30.GL_STATIC_DRAW)

        // 创建顶点数组对象
        GLES30.glGenVertexArrays(1, m_VaoIds, 0)
        GLES30.glBindVertexArray(m_VaoIds[0])

        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, m_VboIds[0])
        GLES30.glEnableVertexAttribArray(0)
        GLES30.glVertexAttribPointer(0, 3, GLES30.GL_FLOAT, false, 5 * 4, 0)
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, GLES30.GL_NONE)

        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, m_VboIds[0])
        GLES30.glEnableVertexAttribArray(1)
        GLES30.glVertexAttribPointer(1, 2, GLES30.GL_FLOAT, false, 5 * 4, 3 * 4)
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, GLES30.GL_NONE)

        GLES30.glBindBuffer(GLES30.GL_ELEMENT_ARRAY_BUFFER, m_VboIds[1])
        GLES30.glBindVertexArray(GLES30.GL_NONE)
    }

    override fun onSurfaceChanged(width: Int, height: Int) {
        Matrix.orthoM(matrixArray, 0, -1f, 1f, -1f, 1f, 0f, 1f)
    }

    override fun onDrawFrame(frame: MediaFrame) {
        when (frame.format) {
            ImageFormat.IMAGE_FORMAT_YV21 -> {
            }
            ImageFormat.IMAGE_FORMAT_YV12 -> {
            }
            ImageFormat.IMAGE_FORMAT_NV12, ImageFormat.IMAGE_FORMAT_NV21 -> {
                drawNV12orNV21(frame)
            }
            else -> {
                drawRGBA(frame)
            }
        }

        GLES30.glUseProgram(program)
        GLES30.glBindVertexArray(m_VaoIds[0])
        // 设置变换矩阵
        GLES30.glUniformMatrix4fv(matrixLocation, 1, false, matrixArray, 0)

        for (i in 0 until 3) {
            GLES30.glActiveTexture(GLES30.GL_TEXTURE0 + i)
            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, m_TextureIds[i])
            GLES30.glUniform1i(texturesLocation[i], i)
        }

        // 设置图片格式
        GLES30.glUniform1i(imageTypeLocation, frame.format.format)

        GLES30.glDrawElements(GLES30.GL_TRIANGLES, 6, GLES30.GL_UNSIGNED_SHORT, 0)
        GLES30.glBindVertexArray(GLES30.GL_NONE)

        frame.buffer.clear()
    }

    private fun drawNV12orNV21(frame: MediaFrame) {
        val buffer = frame.buffer
        val uvIndex = frame.width * frame.height

        //update Y plane data
        buffer.apply {
            position(0)
            limit(uvIndex)
        }
        GLES30.glActiveTexture(GLES30.GL_TEXTURE0)
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, m_TextureIds[0])
        GLES30.glTexImage2D(
            GLES30.GL_TEXTURE_2D, 0, GLES30.GL_LUMINANCE, frame.width, frame.height,
            0, GLES30.GL_LUMINANCE, GLES30.GL_UNSIGNED_BYTE, buffer.slice()
        )
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, GLES30.GL_NONE)

        //update UV plane data
        buffer.apply {
            position(uvIndex)
            limit(buffer.capacity())
        }
        GLES30.glActiveTexture(GLES30.GL_TEXTURE1)
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, m_TextureIds[1])
        GLES30.glTexImage2D(
            GLES30.GL_TEXTURE_2D, 0, GLES30.GL_LUMINANCE_ALPHA, frame.width / 2,
            frame.height / 2, 0, GLES30.GL_LUMINANCE_ALPHA, GLES30.GL_UNSIGNED_BYTE,
            frame.buffer.slice()
        )
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, GL_NONE)
    }

    private fun drawRGBA(frame: MediaFrame) {
        GLES30.glActiveTexture(GLES30.GL_TEXTURE0)
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, m_TextureIds[0])
        GLES30.glTexImage2D(GLES30.GL_TEXTURE_2D, 0, GLES30.GL_RGBA, frame.width, frame.height,
            0, GLES30.GL_RGBA, GLES30.GL_UNSIGNED_BYTE, frame.buffer)
        GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, GLES30.GL_NONE)
    }

    override fun onSurfaceDestroyed(surface: Surface) {
        GLES30.glDeleteVertexArrays(1, m_VaoIds, 0)
        GLES30.glDeleteBuffers(2, m_VboIds, 0)
        GLES30.glDeleteTextures(3, m_TextureIds, 0)
        GLES30.glDeleteProgram(program)
    }
}