//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_FFMEDIAPLAYER_H
#define HIKIKOMORI_FFMEDIAPLAYER_H

#include "MediaPlayer.h"
#include "decoder/video/VideoDecoder.h"
#include "decoder/audio/AudioDecoder.h"
#include "OpenSLAudioRender.h"
#include "NativeVideoRender.h"
#include "MediaSync.h"
#include "ThreadSafeQueue.h"

class FFMediaPlayer : public MediaPlayer, public DecoderCallback, public RenderCallback {

public:
    FFMediaPlayer() {};

    virtual ~FFMediaPlayer();

    virtual int Init(JNIEnv *jniEnv, jobject obj, char *url, int decodeType,
                     int renderType, jobject surface) override;

    virtual void Play() override;

    virtual void Pause() override;

    virtual void Resume() override;

    virtual void Stop() override;

    virtual void SeekToPosition(float position) override;

    virtual Frame *GetOneFrame(int type) override;

    virtual void OnDecoderReady(int type) override;

    virtual void OnDecodeOneFrame(Frame *frame) override;

    virtual void OnSeekResult(int mediaType, bool result) override;

    virtual int GetPlayerState() override;

    virtual void SetPlayerState(PlayerState state) override;

protected:

    virtual int UnInit() override;

private:
    Decoder *m_VideoDecoder;
    Decoder *m_AudioDecoder;
    Render *m_VideoRender;
    Render *m_AudioRender;
    MediaSync *m_AVSync;

    mutex m_Mutex;
    condition_variable m_Cond;

    ThreadSafeQueue *m_VideoFrameQueue;
    ThreadSafeQueue *m_AudioFrameQueue;

    void unInitAudioPlayer();

    void unInitVideoPlayer();

};


#endif //HIKIKOMORI_FFMEDIAPLAYER_H
