//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_DECODER_H
#define HIKIKOMORI_DECODER_H

#define MAX_PATH_LENGTH 2048

#include "MediaDef.h"
#include "Callback.h"

enum DecoderMsg {
    MESSAGE_DECODER_INIT = 0,
    MESSAGE_REINIT,
    MESSAGE_DECODER_LOOP,
    MESSAGE_DECODER_UNINIT,
    MESSAGE_DECODER_SEEK
};

class Decoder : public looper {
private:
    int mLoopMsg = MESSAGE_DECODER_LOOP;
    int result = -1;
    float mTempStamp = -1;
    sem_t runBlock;

protected:
    DecoderCallback *m_Callback = nullptr;
    MediaEventCallback *m_EventCallback = nullptr;

    virtual int _init() = 0;

    virtual void _decodeLoop() = 0;

    virtual void _seekPosition(float timestamp) = 0;

    virtual int _unInit() = 0;

public:
    Decoder(DecoderCallback *decoder_cb, MediaEventCallback *event_cb) {
        m_Callback = decoder_cb;
        m_EventCallback = event_cb;
        sem_init(&runBlock, 0, 0);
    }

    virtual ~Decoder() {
        m_Callback = nullptr;
        m_EventCallback = nullptr;
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
                result = _init();
                sem_post(&runBlock);
                break;
            case MESSAGE_DECODER_LOOP:
                _decodeLoop();
                break;
            case MESSAGE_DECODER_SEEK: {
                float *ts = (float *) data;
                _seekPosition(*ts);
                break;
            }
            case MESSAGE_DECODER_UNINIT:
                _unInit();
                break;
        }
    }
};

#endif //HIKIKOMORI_DECODER_H
