//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_MEDIAPLAYER_H
#define HIKIKOMORI_MEDIAPLAYER_H

#include "ImageRenderItf.h"

class MediaPlayer : public ImageRenderItf {
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
};

#endif //HIKIKOMORI_MEDIAPLAYER_H
