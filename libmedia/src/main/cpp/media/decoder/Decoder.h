//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_DECODER_H
#define HIKIKOMORI_DECODER_H

#define MAX_PATH_LENGTH 2048

#include "MediaDef.h"

class DecoderCallback {
public:
    virtual void OnDecoderReady(int type) = 0;

    virtual void OnDecodeOneFrame(Frame *frame) = 0;

    virtual int GetPlayerState() = 0;

    virtual void SetPlayerState(PlayerState state) = 0;
};

class Decoder {
public:
    Decoder(AVMediaType mediaType, DecoderCallback *callback) {
        m_Callback = callback;
        m_MediaType = mediaType;
    }

    virtual ~Decoder() {
        m_Callback = nullptr;
    }

    virtual int init() = 0;

    virtual int unInit() = 0;

    virtual void DecodingLoop() = 0;

    virtual void startDecodeThread() = 0;

protected:
    DecoderCallback *m_Callback;

    AVMediaType m_MediaType = AVMEDIA_TYPE_UNKNOWN;

    virtual int decode() = 0;

    virtual Frame *OnFrameAvailable() = 0;

    virtual void onDecoderReady() {};

private:
};

#endif //HIKIKOMORI_DECODER_H
