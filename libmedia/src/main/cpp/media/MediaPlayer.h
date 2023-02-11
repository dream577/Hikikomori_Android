//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_MEDIAPLAYER_H
#define HIKIKOMORI_MEDIAPLAYER_H

#include "jni.h"
#include "MediaDef.h"

class MediaPlayer {
public:
    MediaPlayer() {};

    virtual ~MediaPlayer() {};

    virtual int Init(JNIEnv *jniEnv, jobject obj, char *url, int decodeType,
                     int renderType, jobject surface) = 0;

    virtual int UnInit() = 0;

    virtual void Play() = 0;

    virtual void Pause() = 0;

    virtual void Resume() = 0;

    virtual void Stop() = 0;

    virtual void SeekToPosition(float position) = 0;

protected:
    JavaVM *m_JavaVM = nullptr;

    jobject m_JavaObj = nullptr;

    volatile PlayerState state = STATE_UNKNOWN;
};

#endif //HIKIKOMORI_MEDIAPLAYER_H
