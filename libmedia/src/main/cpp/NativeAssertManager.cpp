//
// Created by 郝留凯 on 2023/2/25.
//

#include <jni.h>
#include "NativeAssertManager.h"
#include "LogUtil.h"


extern "C"
JNIEXPORT jlong JNICALL
Java_com_violet_libmedia_util_NativeAssertManager_setAssertManager(JNIEnv *env, jobject thiz,
                                                                   jobject manager) {
    AAssetManager *aAssetManager = AAssetManager_fromJava(env, manager);
    VioletAssertManager *instance = VioletAssertManager::GetInstance();
    instance->SetAssertManager(aAssetManager);
    return reinterpret_cast<jlong>(instance);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_violet_libmedia_util_NativeAssertManager_destroyNativeAssertManager(JNIEnv *env,
                                                                             jobject thiz,
                                                                             jlong assert_manager_handle) {
}