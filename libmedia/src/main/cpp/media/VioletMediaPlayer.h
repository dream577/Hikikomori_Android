//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_VIOLETMEDIAPLAYER_H
#define HIKIKOMORI_VIOLETMEDIAPLAYER_H

#include "MediaPlayer.h"

class VioletMediaPlayer : public MediaPlayer, public DecoderCallback, public RenderCallback {

public:
    VioletMediaPlayer() {};

    virtual ~VioletMediaPlayer();

    virtual int Init(JNIEnv *jniEnv, jobject obj, char *url, int decodeType,
                     int renderType, jobject surface) override;

    virtual void Play() override;

    virtual void Pause() override;

    virtual void Resume() override;

    virtual void Stop() override;

    virtual void SeekToPosition(float position) override;

    virtual Frame *GetOneFrame(int type) override;

    virtual void OnDecodeOneFrame(Frame *frame) override;

    virtual void OnSeekResult(int mediaType, bool result) override;

    virtual int GetPlayerState() override;

    virtual void SetPlayerState(PlayerState state) override;

protected:

    virtual int UnInit() override;

private:

    int initAudioPlayer(char *url);

    int initVideoPlayer(JNIEnv *jniEnv, jobject surface, char *url);

    void unInitAudioPlayer();

    void unInitVideoPlayer();

};


#endif //HIKIKOMORI_VIOLETMEDIAPLAYER_H
