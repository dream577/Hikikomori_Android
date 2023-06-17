package com.violet.libmedia.render.imagerender

import android.opengl.GLES30
import android.opengl.Matrix
import android.view.Surface
import com.violet.libmedia.model.ShaderSource
import com.violet.libmedia.util.BufferUtil
import com.violet.libmedia.util.GLUtils
import java.nio.FloatBuffer
import java.nio.ShortBuffer

class TriangleSampleRender : GLRender {
    companion object {
        const val TAG = "TriangleSampleRender"

        private const val MARTIX = "u_MVPMatrix"

        /*
         * 顶点属性数组，每一行代表一个顶点
         * 其中每一行中的0---2代表顶点坐标属性， 3---6代表顶点颜色属性
         */
        val VERTEX_COORD = floatArrayOf(
            -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
             0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
             0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        )

        /*
         * 图元索引数组
         */
        val INDICES_COORD = shortArrayOf(
            0, 1, 2,
            0, 2, 3
        )

        val MATRIX_COORD = FloatArray(16)
    }

    private val vertexBuffer = BufferUtil.createBuffer(VERTEX_COORD)
    private val indicesBuffer = BufferUtil.createBuffer(INDICES_COORD)

    private val m_VboIds = IntArray(2)
    private val m_VaoIds = IntArray(1)
    private var program: Int = GLES30.GL_NONE

    private var matrixLocation: Int = 0

    override fun onSurfaceCreated(surface: Surface) {
        program = GLUtils.createProgram(
            ShaderSource.TRIANGLE_VERTEX_SHADER,
            ShaderSource.TRIANGLE_FRAGMENT_SHADER
        )
        matrixLocation = GLES30.glGetUniformLocation(program, MARTIX)

        // 创建顶点数组缓冲区对象
        GLES30.glGenBuffers(2, m_VboIds, 0)

        // 缓冲区0用来保存顶点属性数据
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, m_VboIds[0])
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, VERTEX_COORD.size * 4, vertexBuffer, GLES30.GL_STATIC_DRAW)

        // 缓冲区1用来保存图元索引数据
        GLES30.glBindBuffer(GLES30.GL_ELEMENT_ARRAY_BUFFER, m_VboIds[1])
        GLES30.glBufferData(GLES30.GL_ELEMENT_ARRAY_BUFFER, INDICES_COORD.size * 2, indicesBuffer, GLES30.GL_STATIC_DRAW)

        // 创建顶点数组对象
        GLES30.glGenVertexArrays(1, m_VaoIds, 0)
        GLES30.glBindVertexArray(m_VaoIds[0])

        // 指定每一个顶点的坐标属性
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, m_VboIds[0])
        GLES30.glEnableVertexAttribArray(0)
        GLES30.glVertexAttribPointer(0, 3, GLES30.GL_FLOAT, false, 7 * 4, 0)
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, GLES30.GL_NONE)

        // 指定每一个顶点的颜色属性
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, m_VboIds[0])
        GLES30.glEnableVertexAttribArray(1)
        GLES30.glVertexAttribPointer(1,4, GLES30.GL_FLOAT, false, 7 * 4, 12)
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, GLES30.GL_NONE)

        GLES30.glBindBuffer(GLES30.GL_ELEMENT_ARRAY_BUFFER, m_VboIds[1])
        GLES30.glBindVertexArray(GLES30.GL_NONE)
    }

    override fun onSurfaceChanged(width: Int, height: Int) {
        val aspectRatio = height.toFloat() / width
        Matrix.orthoM(MATRIX_COORD, 0, -1f, 1f, -aspectRatio, aspectRatio, -1f, 1f)
    }

    override fun onDrawFrame() {
        GLES30.glUseProgram(program)
        GLES30.glUniformMatrix4fv(matrixLocation, 1, false, MATRIX_COORD, 0)

        GLES30.glBindVertexArray(m_VaoIds[0])
        GLES30.glDrawElements(GLES30.GL_TRIANGLES, INDICES_COORD.size, GLES30.GL_UNSIGNED_SHORT, 0)
        GLES30.glBindVertexArray(GLES30.GL_NONE)
    }

    override fun onSurfaceDestroyed(surface: Surface) {
        GLES30.glDeleteProgram(program)
    }

}