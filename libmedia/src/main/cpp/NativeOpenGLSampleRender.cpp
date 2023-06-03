//
// Created by bronyna on 2023/2/26.
//

#include "TriangleSample.h"
#include "OpenGLSampleRender.h"

using namespace opengles_example;

extern "C"
JNIEXPORT jlong JNICALL
Java_com_violet_libmedia_opengles_OpenGLSampleClient_native_1initOpenGLESSampleRender(JNIEnv *env,
                                                                                      jobject thiz) {
    OpenGLSampleRender *render = new OpenGLSampleRender();
    return reinterpret_cast<jlong>(render);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_opengles_OpenGLSampleClient_native_1loadOpenGLSample(JNIEnv *env,
                                                                              jobject thiz,
                                                                              jlong address,
                                                                              jint type) {
    if (address != 0) {
        OpenGLSampleRender *render = reinterpret_cast<OpenGLSampleRender *>(address);
        render->LoadSample(type);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_opengles_OpenGLSampleClient_native_1onDrawFrame(JNIEnv *env, jobject thiz,
                                                                         jlong address) {
    if (address != 0) {
        OpenGLSampleRender *render = reinterpret_cast<OpenGLSampleRender *>(address);
        render->OnDrawFrame();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_opengles_OpenGLSampleClient_native_1onSurfaceCreated(JNIEnv *env,
                                                                              jobject thiz,
                                                                              jlong address,
                                                                              jobject surface) {
    if (address != 0) {
        OpenGLSampleRender *render = reinterpret_cast<OpenGLSampleRender *>(address);
        render->OnSurfaceCreated(env, surface);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_opengles_OpenGLSampleClient_native_1onSurfaceChanged(JNIEnv *env,
                                                                              jobject thiz,
                                                                              jlong address, jint w,
                                                                              jint h) {
    if (address != 0) {
        OpenGLSampleRender *render = reinterpret_cast<OpenGLSampleRender *>(address);
        render->OnSurfaceChanged(w, h);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_opengles_OpenGLSampleClient_native_1onSurfaceDestroyed(JNIEnv *env,
                                                                                jobject thiz,
                                                                                jlong address) {
    if (address != 0) {
        OpenGLSampleRender *render = reinterpret_cast<OpenGLSampleRender *>(address);
        render->OnSurfaceDestroyed();
    }
}