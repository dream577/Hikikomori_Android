//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_VIOLETMEDIAPLAYER_H
#define HIKIKOMORI_VIOLETMEDIAPLAYER_H

#include "FFMediaInputEngine.h"
#include "MediaPlayer.h"
#include "MediaDef.h"
#include "GLRenderWindow.h"
#include "VideoNativeRender.h"
#include "OpenSLAudioRender.h"
#include "LinkedBlockingQueue.h"

#include "MediaSync.h"
#include "LinkedBlockingQueue.h"

class VioletMediaPlayer : public MediaPlayer, public DecoderCallback,
                          public AVSurface, public RenderCallback {

public:
    VioletMediaPlayer() {};

    virtual ~VioletMediaPlayer() {}

    /* ****************  播放API  **************** */
    virtual int
    Init(JNIEnv *jniEnv, jobject obj, char *url, int decodeType, int renderType) override;
    virtual void Play() override;
    virtual void Pause() override;
    virtual void Resume() override;
    virtual void Stop() override;
    virtual void SeekToPosition(float position) override;
    virtual void StartRecord() override;
    virtual void StopRecord() override;
    virtual int UnInit() override;

    virtual void OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) override;
    virtual void OnSurfaceChanged(int width, int height) override;
    virtual void OnSurfaceDestroyed() override;
    virtual void UpdateMVPMatrix(float translateX, float translateY, float scaleX, float scaleY, int degree,
                         int mirror) override;

    /* 解码相关 */
    virtual void OnFrameReady(std::shared_ptr<MediaFrame> frame) override;
    virtual int GetPlayerState() override;
    virtual void SetPlayerState(PlayerState state) override;

    /* 渲染相关 */
    virtual shared_ptr<MediaFrame> GetOneFrame(int type) override;
    void FrameRendFinish(shared_ptr<MediaFrame> frame) override;

private:
    volatile PlayerState state = STATE_UNKNOWN;

    shared_ptr<FFMediaInputEngine> m_InputEngine;
    shared_ptr<GLRenderWindow> m_ImageRenderWindow;
    shared_ptr<OpenSLAudioRender> m_AudioRender;
    shared_ptr<MediaSync> m_AVSync;
    shared_ptr<MediaEventCallback> m_EventCallback;

    mutex m_Mutex;
    condition_variable m_Cond;

    shared_ptr<LinkedBlockingQueue<MediaFrame>> m_VideoFrameQueue;
    shared_ptr<LinkedBlockingQueue<MediaFrame>> m_AudioFrameQueue;
};


#endif //HIKIKOMORI_VIOLETMEDIAPLAYER_H
