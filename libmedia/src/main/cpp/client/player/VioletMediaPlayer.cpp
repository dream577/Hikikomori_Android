//
// Created by bronyna on 2023/2/5.
//

#include "VioletMediaPlayer.h"
#include "LogUtil.h"
#include "ImageDef.h"

int VioletMediaPlayer::Init(JNIEnv *jniEnv, jobject obj, char *url, int decodeType,
                            int renderType) {
    LOGCATE("VioletMediaPlayer::Init")
    m_EventCallback = new MediaEventCallback(jniEnv, obj);

    m_InputEngine = new AVInputEngine(url, m_EventCallback, this);
    int result = m_InputEngine->Init();

    m_ImageRenderWindow = new GLRenderWindow(this);
    m_VideoFrameQueue = new ThreadSafeQueue(MAX_VIDEO_QUEUE_SIZE, MEDIA_TYPE_VIDEO);

    m_AudioRender = new OpenSLAudioRender(this);
    m_AudioRender->Init();
    m_AudioFrameQueue = new ThreadSafeQueue(MAX_AUDIO_QUEUE_SIZE, MEDIA_TYPE_AUDIO);

    m_AVSync = new MediaSync();
    return result;
}

int VioletMediaPlayer::UnInit() {
    LOGCATE("VioletMediaPlayer::UnInit")
    if (m_AudioFrameQueue) {
        m_AudioFrameQueue->abort();
    }
    if (m_VideoFrameQueue) {
        m_VideoFrameQueue->abort();
    }
    if (m_InputEngine) {
        m_InputEngine->UnInit();
        delete m_InputEngine;
        m_InputEngine = nullptr;
    }
    if (m_AudioRender) {
        m_AudioRender->UnInit();
        delete m_AudioRender;
        m_AudioRender = nullptr;
    }
    if (m_AudioFrameQueue) {
        delete m_AudioFrameQueue;
        m_AudioFrameQueue = nullptr;
    }
    if (m_ImageRenderWindow) {
        m_ImageRenderWindow->Destroy();
        delete m_ImageRenderWindow;
        m_ImageRenderWindow = nullptr;
    }
    if (m_VideoFrameQueue) {
        delete m_VideoFrameQueue;
        m_VideoFrameQueue = nullptr;
    }
    delete m_EventCallback;
    return 0;
}

void VioletMediaPlayer::OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) {
    if (m_ImageRenderWindow != nullptr) {
        m_ImageRenderWindow->OnSurfaceCreated(jniEnv, surface);
    }
}

void VioletMediaPlayer::OnSurfaceChanged(int width, int height) {
    if (m_ImageRenderWindow != nullptr) {
        m_ImageRenderWindow->OnSurfaceChanged(width, height);
    }
}

void VioletMediaPlayer::OnSurfaceDestroyed() {
    if (m_ImageRenderWindow != nullptr) {
        m_ImageRenderWindow->OnSurfaceDestroyed();
    }
}

void VioletMediaPlayer::Play() {
    LOGCATE("VioletMediaPlayer::Play")
    if (GetPlayerState() != STATE_UNKNOWN) return;
    SetPlayerState(STATE_PLAYING);
    m_InputEngine->StartDecodeLoop();
    m_AudioRender->StartRenderLoop();
    m_ImageRenderWindow->StartRender();
}

void VioletMediaPlayer::Pause() {
    LOGCATE("VioletMediaPlayer::Pause")
    if (GetPlayerState() != STATE_PLAYING) return;
    SetPlayerState(STATE_PAUSE);
}

void VioletMediaPlayer::Resume() {
    LOGCATE("VioletMediaPlayer::Resume")
    if (GetPlayerState() != STATE_PAUSE) return;
    SetPlayerState(STATE_PLAYING);
}

void VioletMediaPlayer::Stop() {
    LOGCATE("VioletMediaPlayer::Stop")
    SetPlayerState(STATE_STOP);
    UnInit();
}

void VioletMediaPlayer::SeekToPosition(float position) {
    LOGCATE("VioletMediaPlayer::SeekToPosition=%f", position)
    switch (GetPlayerState()) {
        case STATE_PAUSE:
            VioletMediaPlayer::Resume();
            break;
        case STATE_STOP:
        case STATE_UNKNOWN:
            return;
        default:;
    }
    m_InputEngine->SeekPosition(position);
}

Frame *VioletMediaPlayer::GetOneFrame(int type) {
//    LOGCATE("VioletMediaPlayer::GetOneFrame type=%d", type)
    Frame *frame = nullptr;
    if (GetPlayerState() == STATE_STOP) return frame;
    if (type == MEDIA_TYPE_VIDEO) {
        frame = m_VideoFrameQueue->poll();
        if (frame) {
            m_AVSync->SyncVideo(frame);
        }
    } else {
        frame = m_AudioFrameQueue->poll();
        if (frame) {
            m_AVSync->SyncAudio(frame);
            m_EventCallback->PostMessage(EVENT_PLAYING, frame->pts);
        }
    }
    return frame;
}

void VioletMediaPlayer::FrameRendFinish(Frame *frame) {
    if (frame) {
        // TODO 此处用来扩展录制模块，暂时搁置
        if (frame->type == MEDIA_TYPE_VIDEO) {

        } else {

        }
        delete frame;
    }
}

void VioletMediaPlayer::OnDecodeOneFrame(Frame *frame) {
//    LOGCATE("VioletMediaPlayer::OnDecodeOneFrame MediaType=%d", frame->type)
    if (GetPlayerState() == STATE_STOP) return;
    if (frame->flag == FLAG_SEEK_FINISH) {
        m_AudioFrameQueue->clearCache();
        m_VideoFrameQueue->clearCache();
    }
    int ret;
    if (frame->type == MEDIA_TYPE_VIDEO) {
        ret = m_VideoFrameQueue->offer(frame);
    } else {
        ret = m_AudioFrameQueue->offer(frame);
    }
    if (ret < 0) {
        delete frame;
    }
}

int VioletMediaPlayer::GetPlayerState() {
    return state;
}

void VioletMediaPlayer::SetPlayerState(PlayerState state) {
    unique_lock<mutex> lock(m_Mutex);
    this->state = state;
    m_Cond.notify_all();
}

