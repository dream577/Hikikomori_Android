#include <jni.h>

//
// Created by bronyna on 2023/2/25.
//

#include "CameraVideoRecorder.h"

extern "C"
JNIEXPORT jlong JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1Init(JNIEnv *env,
                                                                              jobject thiz) {
    auto *recorder = new CameraVideoRecorder();
    return reinterpret_cast<jlong>(recorder);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1startRecord(
        JNIEnv *env, jobject thiz, jlong record_handler, jstring path, jstring name) {
    if (record_handler != 0) {
        auto *recorder = reinterpret_cast<CameraVideoRecorder *>(record_handler);
        const char *mPath = env->GetStringUTFChars(path, nullptr);
        const char *mName = env->GetStringUTFChars(name, nullptr);

        auto builder = recorder->Rebuild(recorder);
        // TODO 此处的数据暂时先固定写死
        recorder = builder->EnableAudioRecord(true)
                ->EnableVideoRecord(true)
                ->ConfigAudioParam(44100, AV_SAMPLE_FMT_FLTP, AV_CH_LAYOUT_STEREO)
                ->ConfigVideoParam(1920, 1080, AV_PIX_FMT_YUV420P, 1920 * 1080 * 15 * 24 * 3 / 2, 15)
                ->InitFile(mPath, mName)
                ->Build();
        delete builder;
        recorder->Init();
        recorder->StartRecord();
        env->ReleaseStringUTFChars(path, mPath);
        env->ReleaseStringUTFChars(name, mName);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1stopRecord(
        JNIEnv *env, jobject thiz, jlong record_handler) {
    if (record_handler != 0) {
        auto *recorder = reinterpret_cast<CameraVideoRecorder *>(record_handler);
        recorder->StopRecord();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1InputVideoFrame(
        JNIEnv *env, jobject thiz, jlong record_handle, jbyteArray data, jint width, jint height,
        jint format, jlong timestamp) {
    if (record_handle != 0) {
        int len = env->GetArrayLength(data);
        auto *buf = new unsigned char[len];
        env->GetByteArrayRegion(data, 0, len, reinterpret_cast<jbyte *>(buf));

        auto *recorder = reinterpret_cast<CameraVideoRecorder *>(record_handle);
        recorder->OnDrawVideoFrame(buf, width, height, format, timestamp);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1InputAudioFrame(
        JNIEnv *env, jobject thiz, jlong record_handle, jbyteArray data, jint size, jlong timestamp,
        jint sample_rate, jint sample_format, jint channel_layout) {
    if (record_handle != 0) {
        int len = env->GetArrayLength(data);
        auto *buf = new unsigned char[len];
        env->GetByteArrayRegion(data, 0, size, reinterpret_cast<jbyte *>(buf));

        auto *recorder = reinterpret_cast<CameraVideoRecorder *>(record_handle);
        recorder->InputAudioData(buf, size, timestamp, sample_rate, sample_format, channel_layout);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1onSurfaceCreated(
        JNIEnv *env, jobject thiz, jlong record_handler, jobject surface) {
    if (record_handler != 0) {
        auto *recorder = reinterpret_cast<CameraVideoRecorder *>(record_handler);
        GLRenderWindow *render = recorder->GetVideoRender();
        render->OnSurfaceCreated(env, surface);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1onSurfaceChanged(
        JNIEnv *env, jobject thiz, jlong record_handler, jint width, jint height) {
    if (record_handler != 0) {
        auto *recorder = reinterpret_cast<CameraVideoRecorder *>(record_handler);
        GLRenderWindow *render = recorder->GetVideoRender();
        render->OnSurfaceChanged(width, height);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1onSurfaceDestroyed(
        JNIEnv *env, jobject thiz, jlong record_handler) {
    if (record_handler != 0) {
        auto *recorder = reinterpret_cast<CameraVideoRecorder *>(record_handler);
        GLRenderWindow *render = recorder->GetVideoRender();
        recorder->StopRecord();
        render->OnSurfaceDestroyed();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1SetTransformMatrix(
        JNIEnv *env, jobject thiz, jlong record_handle, jfloat translate_x, jfloat translate_y,
        jfloat scale_x, jfloat scale_y, jint degree, jint mirror) {
    if (record_handle != 0) {
        auto *recorder = reinterpret_cast<CameraVideoRecorder *>(record_handle);
        GLRenderWindow *render = recorder->GetVideoRender();
        render->UpdateMVPMatrix(translate_x, translate_y, scale_x, scale_y, degree, mirror);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_recoder_video_camera_CameraRecordClient_native_1destroy(JNIEnv *env,
                                                                                 jobject thiz,
                                                                                 jlong record_handle) {
    if (record_handle != 0) {
        auto *recorder = reinterpret_cast<CameraVideoRecorder *>(record_handle);
        delete recorder;
    }
}