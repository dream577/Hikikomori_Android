//
// Created by bronyna on 2023/2/11.
//

#include <jni.h>
#include "VioletMediaPlayer.h"
#include "LogUtil.h"

extern "C"
JNIEXPORT jstring JNICALL
Java_com_violet_libmedia_VioletMediaClient_native_1GetFFmpegVersion(JNIEnv *env, jclass clazz) {
    char strBuffer[1024 * 4] = {0};
    strcat(strBuffer, "libavcodec : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVCODEC_VERSION));
    strcat(strBuffer, "\nlibavformat : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVFORMAT_VERSION));
    strcat(strBuffer, "\nlibavutil : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVUTIL_VERSION));
    strcat(strBuffer, "\nlibavfilter : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVFILTER_VERSION));
    strcat(strBuffer, "\nlibswresample : ");
    strcat(strBuffer, AV_STRINGIFY(LIBSWRESAMPLE_VERSION));
    strcat(strBuffer, "\nlibswscale : ");
    strcat(strBuffer, AV_STRINGIFY(LIBSWSCALE_VERSION));
    strcat(strBuffer, "\navcodec_configure : \n");
    strcat(strBuffer, avcodec_configuration());
    strcat(strBuffer, "\navcodec_license : ");
    strcat(strBuffer, avcodec_license());
//    LOGCATE("GetFFmpegVersion\n%s", strBuffer);

    return env->NewStringUTF(strBuffer);
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_violet_libmedia_VioletMediaClient_native_1Init(JNIEnv *env, jobject thiz, jstring url,
                                                        jint player_type, jint render_type,
                                                        jobject surface) {
    const char *path = env->GetStringUTFChars(url, nullptr);
    MediaPlayer *player = new VioletMediaPlayer();
    player->Init(env, thiz, const_cast<char *>(path), 0, render_type, surface);
    env->ReleaseStringUTFChars(url, path);
    return reinterpret_cast<jlong>(player);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_VioletMediaClient_native_1Play(JNIEnv *env, jobject thiz,
                                                        jlong player_handle) {
    if (player_handle != 0) {
        MediaPlayer *player = reinterpret_cast<MediaPlayer *>(player_handle);
        player->Play();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_VioletMediaClient_native_1pause(JNIEnv *env, jobject thiz,
                                                         jlong player_handle) {
    if (player_handle != 0) {
        MediaPlayer *player = reinterpret_cast<MediaPlayer *>(player_handle);
        player->Pause();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_VioletMediaClient_native_1resume(JNIEnv *env, jobject thiz,
                                                          jlong player_handle) {
    if (player_handle != 0) {
        MediaPlayer *player = reinterpret_cast<MediaPlayer *>(player_handle);
        player->Resume();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_VioletMediaClient_native_1stop(JNIEnv *env, jobject thiz,
                                                        jlong player_handle) {
    if (player_handle != 0) {
        MediaPlayer *player = reinterpret_cast<MediaPlayer *>(player_handle);
        player->Stop();
        delete player;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_VioletMediaClient_native_1seekToPosition(JNIEnv *env, jobject thiz,
                                                                  jlong player_handle,
                                                                  jfloat position) {
    if (player_handle != 0) {
        MediaPlayer *player = reinterpret_cast<MediaPlayer *>(player_handle);
        player->SeekToPosition(position);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_VioletMediaClient_native_1pnSurfaceCreated(JNIEnv *env, jobject thiz,
                                                                    jlong player_handle) {
    if (player_handle != 0) {
        MediaPlayer *player = reinterpret_cast<MediaPlayer *>(player_handle);
        VideoRender *render = player->GetVideoRender();
        render->OnSurfaceCreated();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_VioletMediaClient_native_1onSurfaceChanged(JNIEnv *env, jobject thiz,
                                                                    jlong player_handle, jint w,
                                                                    jint h) {
    if (player_handle != 0) {
        MediaPlayer *player = reinterpret_cast<MediaPlayer *>(player_handle);
        VideoRender *videoRender = player->GetVideoRender();
        if (videoRender) {
            videoRender->OnSurfaceChanged(w, h);
        }
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_VioletMediaClient_native_1onSurfaceDestroyed(JNIEnv *env, jobject thiz,
                                                                      jlong player_handle) {
    if (player_handle != 0) {
        MediaPlayer *player = reinterpret_cast<MediaPlayer *>(player_handle);
        VideoRender *videoRender = player->GetVideoRender();
        if (videoRender) {
            videoRender->OnSurfaceDestroyed();
        }
    }
}