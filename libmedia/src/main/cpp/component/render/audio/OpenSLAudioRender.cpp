//
// Created by bronyna on 2023/2/5.
//

#include "OpenSLAudioRender.h"
#include "libavutil/avutil.h"

OpenSLAudioRender::OpenSLAudioRender(RenderCallback *callback) {
    this->m_Callback = callback;
    sem_init(&runBlock, 0, 0);
    stop = false;
}

int OpenSLAudioRender::_Init() {
    LOGCATE("OpenSLRender::init");
    int result = -1;
    do {
        result = _CreateEngine();
        if (result != SL_RESULT_SUCCESS) {
            LOGCATE("OpenSLRender::Init _CreateEngine fail. result=%d", result);
            break;
        }

        result = _CreateOutputMixer();
        if (result != SL_RESULT_SUCCESS) {
            LOGCATE("OpenSLRender::Init _CreateOutputMixer fail. result=%d", result);
            break;
        }

        result = _CreateAudioPlayer();
        if (result != SL_RESULT_SUCCESS) {
            LOGCATE("OpenSLRender::Init _CreateAudioPlayer fail. result=%d", result);
            break;
        }

    } while (false);

    if (result != SL_RESULT_SUCCESS) {
        LOGCATE("OpenSLRender::Init fail. result=%d", result);
        UnInit();
    }
    return result;
}

int OpenSLAudioRender::_UnInit() {
    if (m_AudioPlayerObj) {
        (*m_AudioPlayerPlay)->SetPlayState(m_AudioPlayerPlay, SL_PLAYSTATE_STOPPED);
        m_AudioPlayerPlay = nullptr;
    }

    if (m_AudioPlayerObj) {
        (*m_AudioPlayerObj)->Destroy(m_AudioPlayerObj);
        m_AudioPlayerObj = nullptr;
        m_BufferQueue = nullptr;
    }

    if (m_OutputMixObj) {
        (*m_OutputMixObj)->Destroy(m_OutputMixObj);
        m_OutputMixObj = nullptr;
    }

    if (m_EngineObj) {
        (*m_EngineObj)->Destroy(m_EngineObj);
        m_EngineObj = nullptr;
        m_EngineEngine = nullptr;
    }
    LOGCATE("OpenSLRender::unInit finish");
    return 0;
}

int OpenSLAudioRender::_CreateEngine() {
    LOGCATE("OpenSLAudioRender::_CreateEngine")
    SLresult result = SL_RESULT_SUCCESS;
    do {
        SLEngineOption engineOptions[] = {
                {(SLuint32) SL_ENGINEOPTION_THREADSAFE, (SLuint32) SL_BOOLEAN_TRUE}};
        result = slCreateEngine(&m_EngineObj, sizeof(engineOptions) / sizeof(engineOptions[0]),
                                engineOptions, 0, nullptr, nullptr);
        if (result != SL_RESULT_SUCCESS) {
            LOGCATE("OpenSLAudioRender::_CreateEngine slCreateEngine fail. result=%d", result);
            break;
        }
        result = (*m_EngineObj)->Realize(m_EngineObj, SL_BOOLEAN_FALSE);
        if (result != SL_RESULT_SUCCESS) {
            LOGCATE("OpenSLAudioRender::_CreateEngine Realize fail. result=%d", result);
            break;
        }
        result = (*m_EngineObj)->GetInterface(m_EngineObj, SL_IID_ENGINE, &m_EngineEngine);
        if (result != SL_RESULT_SUCCESS) {
            LOGCATE("OpenSLAudioRender::_CreateEngine GetInterface fail. result=%d", result);
            break;
        }
    } while (false);
    return result;
}

int OpenSLAudioRender::_CreateOutputMixer() {
    LOGCATE("OpenSLAudioRender::_CreateOutputMixer")
    SLresult result = SL_RESULT_SUCCESS;
    do {
        const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
        const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};

        result = (*m_EngineEngine)->CreateOutputMix(m_EngineEngine, &m_OutputMixObj, 1, mids, mreq);
        if (result != SL_RESULT_SUCCESS) {
            LOGCATE("OpenSLAudioRender::_CreateEngine CreateOutputMix fail. result=%d", result);
            break;
        }

        result = (*m_OutputMixObj)->Realize(m_OutputMixObj, SL_BOOLEAN_FALSE);
        if (result != SL_RESULT_SUCCESS) {
            LOGCATE("OpenSLAudioRender::_CreateEngine Realize fail. result=%d", result);
            break;
        }
    } while (false);
    return result;
}

int OpenSLAudioRender::_CreateAudioPlayer() {
    LOGCATE("OpenSLAudioRender::_CreateAudioPlayer")
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,                                // format type
            (SLuint32) 2,                         // channel count
            SL_SAMPLINGRATE_44_1,                             // 44100hz
            SL_PCMSAMPLEFORMAT_FIXED_16,                      // bits per sample
            SL_PCMSAMPLEFORMAT_FIXED_16,                      // container size
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,   // channel mask
            SL_BYTEORDER_LITTLEENDIAN                         // endianness
    };

    SLDataSource dataSource = {&android_queue, &pcm};

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, m_OutputMixObj};
    SLDataSink slDataSink = {&outputMix, nullptr};

    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    SLresult result = SL_RESULT_SUCCESS;
    do {
        result = (*m_EngineEngine)->CreateAudioPlayer(m_EngineEngine, &m_AudioPlayerObj,
                                                      &dataSource, &slDataSink, 3, ids, req);

        if (result != SL_RESULT_SUCCESS) {
            LOGCATE("OpenSLAudioRender::_CreateEngine _CreateAudioPlayer fail. result=%d", result);
            break;
        }

        result = (*m_AudioPlayerObj)->Realize(m_AudioPlayerObj, SL_BOOLEAN_FALSE);
        if (result != SL_RESULT_SUCCESS) {
            LOGCATE("OpenSLAudioRender::_CreateEngine Realize fail. result=%d", result);
            break;
        }

        result = (*m_AudioPlayerObj)->GetInterface(m_AudioPlayerObj, SL_IID_PLAY,
                                                   &m_AudioPlayerPlay);
        if (result != SL_RESULT_SUCCESS) {
            LOGCATE("OpenSLAudioRender::_CreateEngine GetInterface fail. result=%d", result);
            break;
        }

        result = (*m_AudioPlayerObj)->GetInterface(m_AudioPlayerObj, SL_IID_BUFFERQUEUE,
                                                   &m_BufferQueue);
        if (result != SL_RESULT_SUCCESS) {
            LOGCATE("OpenSLAudioRender::_CreateEngine GetInterface fail. result=%d", result);
            break;
        }

        result = (*m_BufferQueue)->RegisterCallback(m_BufferQueue, _AudioPlayCallback, this);
        if (result != SL_RESULT_SUCCESS) {
            LOGCATE("OpenSLAudioRender::_CreateEngine RegisterCallback fail. result=%d", result);
            break;
        }

        result = (*m_AudioPlayerObj)->GetInterface(m_AudioPlayerObj, SL_IID_VOLUME,
                                                   &m_AudioPlayerVolume);
        if (result != SL_RESULT_SUCCESS) {
            LOGCATE("OpenSLAudioRender::_CreateEngine GetInterface fail. result=%d", result);
            break;
        }
    } while (false);
    return result;
}

void OpenSLAudioRender::_EnqueueAudioFrame() {
    shared_ptr<MediaFrame> frame = m_Callback->GetOneFrame(AVMEDIA_TYPE_AUDIO);
    if (frame && !stop) {
        (*m_BufferQueue)->Enqueue(m_BufferQueue, frame->plane[0], frame->planeSize[0]);
    }
    m_Callback->FrameRendFinish(frame);
}

void
OpenSLAudioRender::_AudioPlayCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context) {
    auto *audioRender = (OpenSLAudioRender *) (context);
    audioRender->_LoopOnce();
}

void OpenSLAudioRender::_StartPlay() {
    (*m_AudioPlayerPlay)->SetPlayState(m_AudioPlayerPlay, SL_PLAYSTATE_PLAYING);
    _AudioPlayCallback(m_BufferQueue, this);
}

void OpenSLAudioRender::_LoopOnce() {
    post(MESSAGE_AUDIO_RENDER_LOOP, nullptr);
}

void OpenSLAudioRender::handle(int what, void *data) {
    looper::handle(what, data);
    switch (what) {
        case MESSAGE_AUDIO_RENDER_INIT:
            result = _Init();
            sem_post(&runBlock);
            break;
        case MESSAGE_AUDIO_RENDER_START : {
            _StartPlay();
            break;
        }
        case MESSAGE_AUDIO_RENDER_LOOP:
            _EnqueueAudioFrame();
            break;
        case MESSAGE_CHANGE_VOLUME:

            break;
        case MESSAGE_AUDIO_RENDER_UNINIT:
            _UnInit();
            break;
    }
}

void OpenSLAudioRender::StartRenderLoop() {
    post(MESSAGE_AUDIO_RENDER_START, nullptr);
}

int OpenSLAudioRender::Init() {
    post(MESSAGE_AUDIO_RENDER_INIT, nullptr);
    sem_wait(&runBlock);
    return result;
}

int OpenSLAudioRender::UnInit() {
    stop = true;
    // 1. 终止自动循环
    // 2. 执行卸载程序
    // 3. 退出循环
    // 4. 父类卸载
    disableAutoLoop();
    post(MESSAGE_AUDIO_RENDER_UNINIT, nullptr);
    quit();
    return 0;
}

OpenSLAudioRender::~OpenSLAudioRender() {
    UnInit();
    m_Callback = nullptr;
}
