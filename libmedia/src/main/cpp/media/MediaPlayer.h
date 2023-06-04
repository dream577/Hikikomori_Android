//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_MEDIAPLAYER_H
#define HIKIKOMORI_MEDIAPLAYER_H

#include "jni.h"
#include "MediaDef.h"
#include "VideoNativeRender.h"
#include "VideoGLRender.h"
#include "OpenSLAudioRender.h"
#include "AudioDecoder.h"
#include "VideoDecoder.h"
#include "MediaSync.h"
#include "ThreadSafeQueue.h"
#include "MediaEventCallback.h"

class MediaPlayer {
public:
    MediaPlayer() {};

    virtual ~MediaPlayer() {};

    virtual int Init(JNIEnv *jniEnv, jobject obj, char *url, int decodeType,
                     int renderType) = 0;

    virtual int UnInit() = 0;

    virtual void Play() = 0;

    virtual void Pause() = 0;

    virtual void Resume() = 0;

    virtual void Stop() = 0;

    virtual void SeekToPosition(float position) = 0;

    VideoRender *GetVideoRender() {
        return m_VideoRender;
    }

protected:
    volatile PlayerState state = STATE_UNKNOWN;

    VideoDecoder *m_VideoDecoder;
    AudioDecoder *m_AudioDecoder;
    VideoRender *m_VideoRender;
    AudioRender *m_AudioRender;
    MediaSync *m_AVSync;
    MediaEventCallback *m_EventCallback;

    mutex m_Mutex;
    condition_variable m_Cond;

    ThreadSafeQueue *m_VideoFrameQueue;
    ThreadSafeQueue *m_AudioFrameQueue;
};

#endif //HIKIKOMORI_MEDIAPLAYER_H
