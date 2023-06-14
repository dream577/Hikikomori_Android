package com.violet.libmedia.render.imagerender

import android.opengl.GLES30
import android.view.Surface
import com.violet.libmedia.model.ImageFrame
import com.violet.libmedia.model.ShaderSource
import com.violet.libmedia.util.GLUtils
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.FloatBuffer
import java.nio.IntBuffer

class ImageGLRender: GLRender {
    companion object {
        const val TAG = "TriangleSampleRender"

        val vertexCoords = floatArrayOf(
            -1.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f
        )

        val fragmentCoords = floatArrayOf(
            0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, 0.0f
        )

        val indices = intArrayOf(0, 1, 2, 0, 2, 3)

        const val IMAGE_FORMAT_GRB_888 = 0x00
        const val IMAGE_FORMAT_I420   = 0x01
        const val IMAGE_FORMAT_NV12   = 0x10
        const val IMAGE_FORMAT_NV21   = 0x11
    }

    private var vertexCoordBuffer: FloatBuffer
    private var fragmentCoordBuffer: FloatBuffer
    private var indicBuffer: IntBuffer
    private var program: Int = 0

    private var textureArray: IntArray      // 纹理数组
    private var vboArray: IntArray          // 顶点数组缓冲区对象
    private var vaoArray: IntArray          // 顶点数组对象

    init {
        val buffer0 = ByteBuffer.allocateDirect(vertexCoords.size * 4)
        buffer0.order(ByteOrder.nativeOrder())
        vertexCoordBuffer = buffer0.asFloatBuffer()
        vertexCoordBuffer.put(vertexCoords)
        buffer0.position(0)
        vertexCoordBuffer.position(0)

        val buffer1 = ByteBuffer.allocateDirect(fragmentCoords.size * 4)
        buffer1.order(ByteOrder.nativeOrder())
        fragmentCoordBuffer = buffer1.asFloatBuffer()
        fragmentCoordBuffer.put(fragmentCoords)
        buffer1.position(0)
        fragmentCoordBuffer.position(0)

        val buffer2 = ByteBuffer.allocateDirect(indices.size * 4)
        buffer2.order(ByteOrder.nativeOrder())
        indicBuffer = buffer2.asIntBuffer()
        indicBuffer.put(indices)
        buffer2.position(0)
        indicBuffer.position(0)

        textureArray = intArrayOf(GLES30.GL_NONE, GLES30.GL_NONE, GLES30.GL_NONE)
        vboArray = intArrayOf(GLES30.GL_NONE, GLES30.GL_NONE, GLES30.GL_NONE)
        vaoArray = intArrayOf(GLES30.GL_NONE)
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
        GLES30.glGenBuffers(3, vboArray, 0)

        // 顶点数组缓冲区对象0用来保存顶点数据
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vboArray[0])
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, vertexCoords.size * 4, vertexCoordBuffer, GLES30.GL_STATIC_DRAW)
        // 顶点数组缓冲区对象1用来保存纹理顶点数据
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vboArray[1])
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, fragmentCoords.size * 4, fragmentCoordBuffer, GLES30.GL_STATIC_DRAW)
        // 元素数组缓存区对象用于保存图元索引
        GLES30.glBindBuffer(GLES30.GL_ELEMENT_ARRAY_BUFFER, vboArray[2])
        GLES30.glBufferData(GLES30.GL_ELEMENT_ARRAY_BUFFER, indices.size * 4, indicBuffer, GLES30.GL_STATIC_DRAW)

        // 创建顶点数组对象
        GLES30.glGenVertexArrays(1, vaoArray, 0)
        GLES30.glBindVertexArray(vaoArray[0])

        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vboArray[0])
        GLES30.glEnableVertexAttribArray(0)
        GLES30.glVertexAttribPointer(0, 3, GLES30.GL_FLOAT, false, 3 * 4, 0)
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, GLES30.GL_NONE)

        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vboArray[1])
        GLES30.glEnableVertexAttribArray(1)
        GLES30.glVertexAttribPointer(1, 2, GLES30.GL_FLOAT, false, 2 * 4, 0)
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, GLES30.GL_NONE)

        GLES30.glBindBuffer(GLES30.GL_ELEMENT_ARRAY_BUFFER, vboArray[2])
        GLES30.glBindVertexArray(GLES30.GL_NONE)
    }

    override fun onSurfaceChanged(width: Int, height: Int) {

    }

    override fun onDrawFrame() {
        val image = ImageFrame()
        when (image.format) {
            IMAGE_FORMAT_GRB_888 -> {
                GLES30.glActiveTexture(textureArray[0])
                GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textureArray[0])
                GLES30.glTexImage2D(GLES30.GL_TEXTURE_2D, 0, GLES30.GL_RGBA, image.width,
                    image.height, 0, GLES30.GL_RGBA, GLES30.GL_UNSIGNED_BYTE, image.buffer)
                GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, GLES30.GL_NONE)
            }
            IMAGE_FORMAT_I420 -> {
//                GLES30.glActiveTexture(textureArray[1])
//                GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textureArray[0])

            }
            IMAGE_FORMAT_NV12 -> {

            }
            IMAGE_FORMAT_NV21 -> {

            }
        }
    }

    override fun onSurfaceDestroyed(surface: Surface) {
        GLES30.glDeleteVertexArrays(1, vaoArray, 0)
        GLES30.glDeleteBuffers(3, vboArray, 0)
        GLES30.glDeleteTextures(3, textureArray, 0)
        GLES30.glDeleteProgram(program)
    }
}