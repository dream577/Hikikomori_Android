//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_INPUTENGINE_H
#define HIKIKOMORI_INPUTENGINE_H

#define MAX_PATH_LENGTH 2048

#include "MediaDef.h"
#include "Callback.h"
#include <memory>

enum VideoInputEngineMessage {
    MESSAGE_INIT = 0,
    MESSAGE_LOOP,
    MESSAGE_UNINIT,
    MESSAGE_SEEK
};

class InputEngine : public looper {
private:
    int mLoopMsg = MESSAGE_LOOP;
    int result = -1;
    float mTempStamp = -1;
    sem_t runBlock;

protected:

    virtual int init() = 0;

    virtual void decodeLoop() = 0;

    virtual void seekToPosition(float timestamp) = 0;

    virtual int unInit() = 0;

public:
    InputEngine() {
        sem_init(&runBlock, 0, 0);
    }

    virtual ~InputEngine() {
        sem_destroy(&runBlock);
    }

    virtual int Init() {
        post(MESSAGE_INIT, nullptr);
        sem_wait(&runBlock);
        return result;
    };

    virtual int UnInit() {
        disableAutoLoop();
        post(MESSAGE_UNINIT, nullptr);
        quit();
        return 0;
    }

    virtual int SeekPosition(float timestamp) {
        mTempStamp = timestamp;
        post(MESSAGE_SEEK, &mTempStamp);
        return 0;
    }

    virtual void StartDecodeLoop() {
        enableAutoLoop(&mLoopMsg);
    };

    virtual void handle(int what, void *data) override {
        looper::handle(what, data);
        switch (what) {
            case MESSAGE_INIT:
                result = init();
                sem_post(&runBlock);
                break;
            case MESSAGE_LOOP:
                decodeLoop();
                break;
            case MESSAGE_SEEK: {
                float *ts = (float *) data;
                seekToPosition(*ts);
                break;
            }
            case MESSAGE_UNINIT:
                unInit();
                break;
        }
    }
};

#endif //HIKIKOMORI_INPUTENGINE_H
