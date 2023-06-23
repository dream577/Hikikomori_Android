package com.violet.libmedia.core

import android.opengl.EGL14
import android.opengl.EGL15
import android.opengl.EGLConfig
import android.opengl.EGLExt
import android.view.Surface
import com.violet.libbasetools.util.KLog
import java.util.concurrent.atomic.AtomicBoolean
import javax.microedition.khronos.egl.EGLContext

class EglCore {
    companion object {
        private const val TAG = "EglCore"
    }

    private var hasInitialized = AtomicBoolean(false)
    private var mEGLContext = EGL14.EGL_NO_CONTEXT
    private var mEGLDisplay = EGL14.EGL_NO_DISPLAY
    private var mEGLSurface = EGL14.EGL_NO_SURFACE

    fun init(surface: Surface): Boolean {
        var result = false
        do {
            // 1.打开与EGL显示服务器的连接
            mEGLDisplay = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY)
            if (mEGLDisplay == EGL14.EGL_NO_DISPLAY) {
                KLog.d(TAG, "eglGetDisplay error")
                break
            }

            // 2.初始化EGL
            val versions = intArrayOf(0, 0)
            result = EGL14.eglInitialize(mEGLDisplay, versions, 0, versions, 1)
            if (!result) {
                KLog.d(TAG, "eglInitialize error")
                break
            }

            // 3.选择EGL配置
            val configAttriArray = intArrayOf(
                EGL14.EGL_BUFFER_SIZE, 32,
                EGL14.EGL_RED_SIZE, 8,
                EGL14.EGL_GREEN_SIZE, 8,
                EGL14.EGL_BLUE_SIZE, 8,
                EGL14.EGL_ALPHA_SIZE, 8,
                EGL14.EGL_DEPTH_SIZE, 4,
                EGL14.EGL_RENDERABLE_TYPE, EGL14.EGL_OPENGL_ES2_BIT,
                EGL14.EGL_NONE
            )
            val mConfigs = arrayOfNulls<EGLConfig>(1)
            val mConfigNum = intArrayOf(1)
            result = EGL14.eglChooseConfig(
                mEGLDisplay, configAttriArray, 0, mConfigs,
                0, mConfigs.size, mConfigNum, 0
            )
            if (!result) {
                KLog.d(TAG, "eglChooseConfig error")
                break
            }

            // 5.创建EGLSurface
            val surfaceAttriArray = intArrayOf(
                EGL14.EGL_NONE
            )
            mEGLSurface = EGL14.eglCreateWindowSurface(
                mEGLDisplay, mConfigs[0],
                surface, surfaceAttriArray, 0
            )
            if (mEGLSurface == EGL14.EGL_NO_SURFACE) {
                KLog.d(TAG, "eglCreateWindowSurface error")
                result = false
                break
            }

            // 5.创建EGLContext
            val contextAttriArray = intArrayOf(
                EGL14.EGL_CONTEXT_CLIENT_VERSION, 3,
                EGL14.EGL_NONE
            )
            mEGLContext = EGL14.eglCreateContext(
                mEGLDisplay, mConfigs[0], EGL14.EGL_NO_CONTEXT,
                contextAttriArray, 0
            )
            if (mEGLContext == EGL14.EGL_NO_CONTEXT) {
                KLog.d(TAG, "eglCreateContext error")
                result = false
                break
            }

            // 6.指定EGLContext为当前上下文
            result = EGL14.eglMakeCurrent(mEGLDisplay, mEGLSurface, mEGLSurface, mEGLContext)
        } while (false)
        hasInitialized.set(result)
        return result
    }

    fun swapBuffers() {
        if (!hasInitialized.get()) return
        EGL14.eglSwapBuffers(mEGLDisplay, mEGLSurface)
    }

    fun unInit() {
        if (mEGLSurface != EGL14.EGL_NO_SURFACE) {
            EGL14.eglMakeCurrent(
                mEGLDisplay,
                EGL14.EGL_NO_SURFACE,
                EGL14.EGL_NO_SURFACE,
                EGL14.EGL_NO_CONTEXT
            )
            EGL14.eglDestroySurface(mEGLDisplay, mEGLSurface)
            mEGLSurface = EGL14.EGL_NO_SURFACE
        }
        if (mEGLContext != EGL14.EGL_NO_CONTEXT) {
            EGL14.eglDestroyContext(mEGLDisplay, mEGLContext)
            mEGLContext = EGL14.EGL_NO_CONTEXT
        }
        if (mEGLDisplay != EGL14.EGL_NO_DISPLAY) {
            EGL14.eglTerminate(mEGLDisplay)
            mEGLDisplay = EGL14.EGL_NO_DISPLAY
        }
        hasInitialized.set(false)
    }
}