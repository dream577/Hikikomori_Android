//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_INPUTENGINE_H
#define HIKIKOMORI_INPUTENGINE_H

#define MAX_PATH_LENGTH 2048

#include "MediaDef.h"
#include "Callback.h"
#include <memory>

enum DecoderMsg {
    MESSAGE_DECODER_INIT = 0,
    MESSAGE_REINIT,
    MESSAGE_DECODER_LOOP,
    MESSAGE_DECODER_UNINIT,
    MESSAGE_DECODER_SEEK
};

class InputEngine : public looper {
private:
    int mLoopMsg = MESSAGE_DECODER_LOOP;
    int result = -1;
    float mTempStamp = -1;
    sem_t runBlock;

protected:

    virtual int _Init() = 0;

    virtual void _DecoderLoop() = 0;

    virtual void _SeekPosition(float timestamp) = 0;

    virtual int _UnInit() = 0;

public:
    InputEngine() {
        sem_init(&runBlock, 0, 0);
    }

    virtual ~InputEngine() {
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
                result = _Init();
                sem_post(&runBlock);
                break;
            case MESSAGE_DECODER_LOOP:
                _DecoderLoop();
                break;
            case MESSAGE_DECODER_SEEK: {
                float *ts = (float *) data;
                _SeekPosition(*ts);
                break;
            }
            case MESSAGE_DECODER_UNINIT:
                _UnInit();
                break;
        }
    }
};

#endif //HIKIKOMORI_INPUTENGINE_H