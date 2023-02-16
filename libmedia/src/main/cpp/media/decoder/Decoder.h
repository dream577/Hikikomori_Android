//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_DECODER_H
#define HIKIKOMORI_DECODER_H

#define MAX_PATH_LENGTH 2048

#include "MediaDef.h"
#include "looper.h"

enum DecoderMsg {
    MESSAGE_INIT = 0,
    MESSAGE_REINIT,
    MESSAGE_DECODE_LOOP,
    MESSAGE_UNINIT,
    MESSAGE_SEEK
};

class DecoderCallback {
public:
    virtual void OnDecodeOneFrame(Frame *frame) = 0;

    virtual void OnSeekResult(int mediaType, bool result) = 0;

    virtual int GetPlayerState() = 0;

    virtual void SetPlayerState(PlayerState state) = 0;
};

class Decoder : public looper {
private:
    int mLoopMsg = MESSAGE_DECODE_LOOP;

protected:
    DecoderCallback *m_Callback = nullptr;

    virtual void decodeLoop() = 0;

    virtual Frame *onFrameAvailable() = 0;

    virtual void seekPosition(float timestamp) = 0;

    volatile float m_SeekPosition = -1;               // seek position

public:
    Decoder(DecoderCallback *callback) {
        m_Callback = callback;
    }

    virtual ~Decoder() {
        m_Callback = nullptr;
    }

    virtual int Init() = 0;

    virtual int UnInit() = 0;

    virtual void StartDecodeLoop() {
        enableAutoLoop(&mLoopMsg);
    };

    int SeekPosition(float timestamp) {
        seekPosition(timestamp);
    }

    virtual void StartDecoderThread() {};

    virtual void handle(int what, void *data) override {
        looper::handle(what, data);
        switch (what) {
//            case MESSAGE_INIT:
//                Init();
//                break;
            case MESSAGE_DECODE_LOOP:
                decodeLoop();
                break;
//            case MESSAGE_SEEK: {
//                float *ts = (float *) data;
//                seekPosition(*ts);
//                break;
//            }
//            case MESSAGE_UNINIT:
//                UnInit();
//                break;
        }
    }
};

#endif //HIKIKOMORI_DECODER_H
