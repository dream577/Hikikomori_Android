//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_VIOLETMEDIAPLAYER_H
#define HIKIKOMORI_VIOLETMEDIAPLAYER_H

#include "AVInputEngine.h"
#include "MediaPlayer.h"
#include "MediaDef.h"
#include "GLRenderWindow.h"
#include "VideoNativeRender.h"
#include "OpenSLAudioRender.h"

#include "MediaSync.h"
#include "ThreadSafeQueue.h"

class VioletMediaPlayer : public MediaPlayer, public DecoderCallback, public RenderCallback {

public:
    VioletMediaPlayer() {};

    virtual ~VioletMediaPlayer() {}

    virtual int
    Init(JNIEnv *jniEnv, jobject obj, char *url, int decodeType, int renderType) override;

    virtual void OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) override;

    virtual void OnSurfaceChanged(int width, int height) override;

    virtual void OnSurfaceDestroyed() override;

    virtual int UnInit() override;

    virtual void Play() override;

    virtual void Pause() override;

    virtual void Resume() override;

    virtual void Stop() override;

    virtual void SeekToPosition(float position) override;

    virtual Frame *GetOneFrame(int type) override;

    void FrameRendFinish(Frame *frame) override;

    virtual void OnDecodeOneFrame(Frame *frame) override;

    virtual int GetPlayerState() override;

    virtual void SetPlayerState(PlayerState state) override;

private:
    volatile PlayerState state = STATE_UNKNOWN;

    AVInputEngine *m_InputEngine;
    GLRenderWindow *m_ImageRenderWindow;
    OpenSLAudioRender *m_AudioRender;
    MediaSync *m_AVSync;
    MediaEventCallback *m_EventCallback;

    mutex m_Mutex;
    condition_variable m_Cond;

    ThreadSafeQueue *m_VideoFrameQueue;
    ThreadSafeQueue *m_AudioFrameQueue;
};


#endif //HIKIKOMORI_VIOLETMEDIAPLAYER_H
