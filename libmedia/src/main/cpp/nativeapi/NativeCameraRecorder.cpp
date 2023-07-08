#include <jni.h>

//
// Created by bronyna on 2023/2/25.
//

#include "CameraVideoRecorder.h"

extern "C"
JNIEXPORT jlong JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1Init(JNIEnv *env,
                                                                              jobject thiz) {
    CameraVideoRecorder *recorder = new CameraVideoRecorder();
    return reinterpret_cast<jlong>(recorder);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1startRecord(
        JNIEnv *env, jobject thiz, jlong record_handler, jstring path) {
    if (record_handler != 0) {
        CameraVideoRecorder *recorder = reinterpret_cast<CameraVideoRecorder *>(record_handler);
        recorder->StartRecord();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1stopRecord(
        JNIEnv *env, jobject thiz, jlong record_handler) {
    if (record_handler != 0) {
        CameraVideoRecorder *recorder = reinterpret_cast<CameraVideoRecorder *>(record_handler);
        recorder->StopRecord();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1rendPreviewVideoFrame(
        JNIEnv *env, jobject thiz, jlong record_handler, jbyteArray data, jint width, jint height,
        jint format, jlong timestamp) {
    if (record_handler != 0) {
        int len = env->GetArrayLength(data);
        auto *buf = new unsigned char[len];
        env->GetByteArrayRegion(data, 0, len, reinterpret_cast<jbyte *>(buf));

        CameraVideoRecorder *recorder = reinterpret_cast<CameraVideoRecorder *>(record_handler);
        recorder->OnDrawPreviewFrame(buf, width, height, format, timestamp);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1onSurfaceCreated(
        JNIEnv *env, jobject thiz, jlong record_handler, jobject surface) {
    if (record_handler != 0) {
        CameraVideoRecorder *recorder = reinterpret_cast<CameraVideoRecorder *>(record_handler);
        GLRenderWindow *render = recorder->GetVideoRender();
        render->OnSurfaceCreated(env, surface);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1onSurfaceChanged(
        JNIEnv *env, jobject thiz, jlong record_handler, jint width, jint height) {
    if (record_handler != 0) {
        CameraVideoRecorder *recorder = reinterpret_cast<CameraVideoRecorder *>(record_handler);
        GLRenderWindow *render = recorder->GetVideoRender();
        render->OnSurfaceChanged(width, height);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1onSurfaceDestroyed(
        JNIEnv *env, jobject thiz, jlong record_handler) {
    if (record_handler != 0) {
        CameraVideoRecorder *recorder = reinterpret_cast<CameraVideoRecorder *>(record_handler);
        recorder->UnInit();
        delete recorder;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1SetTransformMatrix(
        JNIEnv *env, jobject thiz, jlong record_handle, jfloat translate_x, jfloat translate_y,
        jfloat scale_x, jfloat scale_y, jint degree, jint mirror) {
    if (record_handle != 0) {
        CameraVideoRecorder *recorder = reinterpret_cast<CameraVideoRecorder *>(record_handle);
        GLRenderWindow *render = recorder->GetVideoRender();
        render->UpdateMVPMatrix(translate_x, translate_y, scale_x, scale_y, degree, mirror);
    }
}