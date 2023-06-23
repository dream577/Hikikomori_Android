package com.violet.libmedia.render.imagerender

import android.opengl.GLES30
import android.view.Surface
import com.violet.libmedia.core.GLUtils
import com.violet.libmedia.util.ShaderSource
import com.violet.libmedia.util.BufferUtil
import java.nio.ByteBuffer
import java.nio.FloatBuffer
import java.nio.ShortBuffer

class ImageGLRender: GLRender {
    companion object {
        const val TAG = "TriangleSampleRender"

        val vertexCoords = floatArrayOf(
            -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 0.0f
        )

        val indices = shortArrayOf(0, 1, 2, 0, 2, 3)

        const val IMAGE_FORMAT_GRB_888 = 0x00
        const val IMAGE_FORMAT_I420   = 0x01
        const val IMAGE_FORMAT_NV12   = 0x10
        const val IMAGE_FORMAT_NV21   = 0x11
    }

    private var program: Int = 0

    private val vertexCoordBuffer: FloatBuffer
    private val indicBuffer: ShortBuffer
    private val dataBuffer: ByteBuffer

    private val textureArray: IntArray      // 纹理数组
    private val m_VboIds: IntArray          // 顶点数组缓冲区对象
    private val m_VaoIds: IntArray          // 顶点数组对象

    init {
        vertexCoordBuffer = BufferUtil.createBuffer(vertexCoords)
        indicBuffer = BufferUtil.createBuffer(indices)

        dataBuffer = ByteBuffer.allocateDirect(1024 * 1024)

        textureArray = intArrayOf(GLES30.GL_NONE, GLES30.GL_NONE, GLES30.GL_NONE)
        m_VboIds = intArrayOf(GLES30.GL_NONE, GLES30.GL_NONE, GLES30.GL_NONE)
        m_VaoIds = intArrayOf(GLES30.GL_NONE)
    }

    override fun onSurfaceCreated(surface: Surface) {
        // 初始化EGL环境相关
        program = GLUtils.createProgram(
            ShaderSource.IMAGE_VERTEX_SHADER,
            ShaderSource.IMAGE_FRAGMENT_SHADER
        )

        // 创建3个纹理对象
        GLES30.glGenTextures(3, textureArray, 0)
        for (i in 0 until 3) {
            // 激活纹理
            GLES30.glActiveTexture(GLES30.GL_TEXTURE0 + i)
            // 绑定纹理
            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textureArray[i])
            // 设置过滤方式
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_LINEAR)
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_LINEAR)
            // 设置纹理坐标系s轴和t轴的映射规则
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_CLAMP_TO_EDGE)
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_CLAMP_TO_EDGE)
            // 纹理解绑
            GLES30.glBindTexture(textureArray[i], GLES30.GL_NONE)
        }

        // 创建顶点竖组缓冲区对象
        GLES30.glGenBuffers(2, m_VboIds, 0)

        // 顶点数组缓冲区对象0用来保存顶点属性数组
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, m_VboIds[0])
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, vertexCoords.size * 4, vertexCoordBuffer, GLES30.GL_STATIC_DRAW)

        // 元素数组缓存区对象用于保存图元索引
        GLES30.glBindBuffer(GLES30.GL_ELEMENT_ARRAY_BUFFER, m_VboIds[1])
        GLES30.glBufferData(GLES30.GL_ELEMENT_ARRAY_BUFFER, indices.size * 4, indicBuffer, GLES30.GL_STATIC_DRAW)

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

    }

    override fun onDrawFrame() {
//        val image = ImageFrame()
//        dataBuffer.clear()
//        when (image.format) {
//            IMAGE_FORMAT_GRB_888 -> {
//                GLES30.glActiveTexture(textureArray[0])
//                GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textureArray[0])
//                GLES30.glTexImage2D(GLES30.GL_TEXTURE_2D, 0, GLES30.GL_RGBA, image.width,
//                    image.height, 0, GLES30.GL_RGBA, GLES30.GL_UNSIGNED_BYTE, dataBuffer[ByteArray(3), 0 , 3])
//                GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, GLES30.GL_NONE)
//            }
//            IMAGE_FORMAT_I420 -> {
//                GLES30.glActiveTexture(textureArray[1])
//                GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textureArray[0])
//
//            }
//            IMAGE_FORMAT_NV12 -> {
//
//            }
//            IMAGE_FORMAT_NV21 -> {
//
//            }
//        }
    }

    override fun onSurfaceDestroyed(surface: Surface) {
        GLES30.glDeleteVertexArrays(1, m_VaoIds, 0)
        GLES30.glDeleteBuffers(2, m_VboIds, 0)
        GLES30.glDeleteTextures(3, textureArray, 0)
        GLES30.glDeleteProgram(program)
    }
}