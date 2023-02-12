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
#include <thread>
#include "AudioRender.h"

using namespace std;

class OpenSLAudioRender : public AudioRender {
public:
    OpenSLAudioRender(RenderCallback *callback) : AudioRender(callback) {}

    virtual ~OpenSLAudioRender();

    virtual int init() override;

    virtual int unInit() override;

    virtual int destroy() override;

    virtual void startRenderThread() override;

private:
    int createEngine();

    int createOutputMixer();

    int createAudioPlayer();

    void startPlay();

    static void createSLWaitingThread(OpenSLAudioRender *openSlRender);

    static void audioPlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context);

    virtual void renderAudioFrame() override;

    SLObjectItf m_EngineObj;
    SLEngineItf m_EngineEngine;
    SLObjectItf m_OutputMixObj;
    SLObjectItf m_AudioPlayerObj;
    SLPlayItf m_AudioPlayerPlay;
    SLVolumeItf m_AudioPlayerVolume;
    SLAndroidSimpleBufferQueueItf m_BufferQueue;

    thread *m_thread = nullptr;
};


#endif //HIKIKOMORI_OPENSLAUDIORENDER_H
