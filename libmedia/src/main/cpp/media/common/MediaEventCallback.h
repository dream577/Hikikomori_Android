//
// Created by 郝留凯 on 2023/6/4.
//

#ifndef HIKIKOMORI_MEDIAEVENTCALLBACK_H
#define HIKIKOMORI_MEDIAEVENTCALLBACK_H

#include "looper.h"
#include "LogUtil.h"
#include <jni.h>

#define INIT_CALLBACK    10000000
#define UNINIT_CALLBACK  10000001

class MediaEventCallback : looper {
public:
    MediaEventCallback(JNIEnv *jniEnv, jobject obj) : looper() {
        jniEnv->GetJavaVM(&m_JavaVM);
        m_JavaObj = jniEnv->NewGlobalRef(obj);
        post(INIT_CALLBACK, nullptr);
    }

    ~MediaEventCallback() override {
        post(UNINIT_CALLBACK, nullptr);
    }

public:

    void PostMessage(int type, long value) {
        long *v = (long *) malloc(sizeof(long));
        *v = value;
        post(type, v);
    }

    void handle(int what, void *data) override {
        switch (what) {
            case INIT_CALLBACK:
                init();
                break;
            case UNINIT_CALLBACK:
                unInit();
                break;
            default:
                long *value = (long *) data;
                postMessage(what, *value);
                free(value);
        }
    }

private:
    JNIEnv *globalEnv = nullptr;
    JavaVM *m_JavaVM = nullptr;
    jobject m_JavaObj = nullptr;

    jmethodID mid = nullptr;

    bool isAttached = false;

    void init() {
        do {
            int status = m_JavaVM->GetEnv((void **) &globalEnv, JNI_VERSION_1_4);
            if (status != JNI_OK) {
                status = m_JavaVM->AttachCurrentThread(&globalEnv, nullptr);
                if (status != JNI_OK) {
                    LOGCATE("VioletMediaPlayer::getJNIEnv error")
                    break;
                }
                isAttached = true;
            }

            jclass myClass = globalEnv->GetObjectClass(m_JavaObj);
            if (myClass == nullptr) {
                LOGCATE("VioletMediaPlayer::postMessage noSuchClass")
            }
            mid = globalEnv->GetMethodID(myClass, "onReceiveMediaEvent", "(IJ)V");
        } while (false);
    }

    void unInit() {
        if (globalEnv && m_JavaObj) {
            globalEnv->DeleteGlobalRef(m_JavaObj);
        }
        if (isAttached && m_JavaVM) {
            m_JavaVM->DetachCurrentThread();
        }
    }

    void postMessage(int type, long value) {
        if (mid != nullptr) {
            globalEnv->CallVoidMethod(m_JavaObj, mid, type, value);
        }
    }

};

#endif //HIKIKOMORI_MEDIAEVENTCALLBACK_H