//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_DECODER_H
#define HIKIKOMORI_DECODER_H

#define MAX_PATH_LENGTH 2048

#include "MediaDef.h"
#include "looper.h"

enum DecoderMsg {
    MESSAGE_DECODER_INIT = 0,
    MESSAGE_REINIT,
    MESSAGE_DECODER_LOOP,
    MESSAGE_DECODER_UNINIT,
    MESSAGE_DECODER_SEEK
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
    int mLoopMsg = MESSAGE_DECODER_LOOP;
    int result = -1;
    float mTempStamp = -1;
    sem_t runBlock;

protected:
    DecoderCallback *m_Callback = nullptr;

    virtual int init() = 0;

    virtual void decodeLoop() = 0;

    virtual Frame *onFrameAvailable() = 0;

    virtual void seekPosition(float timestamp) = 0;

    virtual int unInit() = 0;

public:
    Decoder(DecoderCallback *callback) {
        m_Callback = callback;
        sem_init(&runBlock, 0, 0);
    }

    virtual ~Decoder() {
        m_Callback = nullptr;
        sem_destroy(&runBlock);
    }

    virtual int Init() {
        post(MESSAGE_DECODER_INIT, nullptr);
        sem_wait(&runBlock);
        return result;
    };

    virtual int UnInit() {
        disableAutoLoop();
        post(MESSAGE_DECODER_UNINIT, nullptr);
        quit();
        return 0;
    }

    virtual int SeekPosition(float timestamp) {
        mTempStamp = timestamp;
        post(MESSAGE_DECODER_SEEK, &mTempStamp);
        return 0;
    }

    virtual void StartDecodeLoop() {
        enableAutoLoop(&mLoopMsg);
    };

    virtual void handle(int what, void *data) override {
        looper::handle(what, data);
        switch (what) {
            case MESSAGE_DECODER_INIT:
                result = init();
                sem_post(&runBlock);
                break;
            case MESSAGE_DECODER_LOOP:
                decodeLoop();
                break;
            case MESSAGE_DECODER_SEEK: {
                float *ts = (float *) data;
                seekPosition(*ts);
                break;
            }
            case MESSAGE_DECODER_UNINIT:
                unInit();
                break;
        }
    }
};

#endif //HIKIKOMORI_DECODER_H
