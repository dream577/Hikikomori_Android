//
// Created by 郝留凯 on 2023/7/2.
//

#ifndef HIKIKOMORI_CALLBACK_H
#define HIKIKOMORI_CALLBACK_H

#include "MediaDef.h"
#include "looper.h"
#include "LogUtil.h"

#include <jni.h>
#include <stdlib.h>

#define INIT_CALLBACK    10000000
#define UNINIT_CALLBACK  10000001

class RenderCallback {
public:
    virtual Frame *GetOneFrame(int type) = 0;

    virtual void FrameRendFinish(Frame* frame) = 0;
};

class DecoderCallback {
public:
    virtual void OnDecodeOneFrame(Frame *frame) = 0;

    virtual void OnSeekResult(int mediaType, bool result) = 0;

    virtual int GetPlayerState() = 0;

    virtual void SetPlayerState(PlayerState state) = 0;
};

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

#endif //HIKIKOMORI_CALLBACK_H
