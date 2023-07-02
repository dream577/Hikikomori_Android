//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_OPENSLAUDIORENDER_H
#define HIKIKOMORI_OPENSLAUDIORENDER_H

#include <cstdint>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <queue>
#include <string>

#include "Callback.h"
#include "looper.h"

using namespace std;

enum AudioRenderMessage {
    MESSAGE_AUDIO_RENDER_INIT = 0,
    MESSAGE_AUDIO_RENDER_START,
    MESSAGE_AUDIO_RENDER_LOOP,
    MESSAGE_CHANGE_VOLUME,
    MESSAGE_AUDIO_RENDER_UNINIT
};

class OpenSLAudioRender : public looper {
private:
    int mLoopMsg = MESSAGE_AUDIO_RENDER_LOOP;
    RenderCallback *m_Callback;
    volatile bool stop = false;

    int result = -1;
    sem_t runBlock;

    int createEngine();

    int createOutputMixer();

    int createAudioPlayer();

    void onStartPlay();

    static void audioPlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context);

    void playAudioFrame();

    void onPlayFrame();

    SLObjectItf m_EngineObj;
    SLEngineItf m_EngineEngine;
    SLObjectItf m_OutputMixObj;
    SLObjectItf m_AudioPlayerObj;
    SLPlayItf m_AudioPlayerPlay;
    SLVolumeItf m_AudioPlayerVolume;
    SLAndroidSimpleBufferQueueItf m_BufferQueue;

    int init();

    int unInit();

protected:
    virtual void handle(int what, void *data) override;

public:
    OpenSLAudioRender(RenderCallback *callback) {
        this->m_Callback = callback;
        sem_init(&runBlock, 0, 0);
    }

    ~OpenSLAudioRender() {
        m_Callback = nullptr;
    }

    int Init();

    int UnInit();

    void StartRenderLoop();
};


#endif //HIKIKOMORI_OPENSLAUDIORENDER_H
