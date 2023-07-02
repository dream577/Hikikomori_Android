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

    m_AudioDecoder = new AudioDecoder(url, m_EventCallback, this);
    int result1 = m_AudioDecoder->Init();
    if (result1 == 0) {
        m_AudioRender = new OpenSLAudioRender(this);
        result1 = m_AudioRender->Init();
        if (result1 == 0) {
            m_AudioFrameQueue = new ThreadSafeQueue(MAX_AUDIO_QUEUE_SIZE, MEDIA_TYPE_AUDIO);
        }
    }
    if (result1 != 0) {
        LOGCATE("VioletMediaPlayer::Init Audio Component init fail")
        unInitAudioPlayer();
    } else {
        LOGCATE("VioletMediaPlayer::Init Audio Component init success")
    }

    m_VideoDecoder = new VideoDecoder(url, nullptr, this);
    int result2 = m_VideoDecoder->Init();
    if (result2 == 0) {
        m_ImageRenderWindow = new GLRenderWindow(this);
        m_VideoFrameQueue = new ThreadSafeQueue(MAX_VIDEO_QUEUE_SIZE, MEDIA_TYPE_VIDEO);
    }
    if (result2 != 0) {
        LOGCATE("VioletMediaPlayer::Init Video Component init fail")
        unInitVideoPlayer();
    } else {
        LOGCATE("VioletMediaPlayer::Init Video Component init success")
    }

    if (result1 == 0 || result2 == 0) {
        m_AVSync = new MediaSync();
    }
    return result1 | result2;
}


int VioletMediaPlayer::UnInit() {
    LOGCATE("VioletMediaPlayer::UnInit")
    unInitAudioPlayer();
    unInitVideoPlayer();
    delete m_EventCallback;
    return 0;
}

void VioletMediaPlayer::unInitAudioPlayer() {
    LOGCATE("VioletMediaPlayer::unInitAudioPlayer")
    if (m_AudioFrameQueue) {
        m_AudioFrameQueue->abort();
    }
    if (m_AudioDecoder) {
        m_AudioDecoder->UnInit();
        delete m_AudioDecoder;
        m_AudioDecoder = nullptr;
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
}

void VioletMediaPlayer::unInitVideoPlayer() {
    LOGCATE("VioletMediaPlayer::unInitVideoPlayer")
    if (m_VideoFrameQueue) {
        m_VideoFrameQueue->abort();
    }
    if (m_VideoDecoder) {
        m_VideoDecoder->UnInit();
        delete m_VideoDecoder;
        m_VideoDecoder = nullptr;
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
    if (m_AudioDecoder) {
        m_AudioDecoder->StartDecodeLoop();
        m_AudioRender->StartRenderLoop();
    }
    if (m_VideoDecoder) {
        m_VideoDecoder->StartDecodeLoop();
        m_ImageRenderWindow->StartRender();
    }

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
    LOGCATE("VioletMediaPlayer::SeekToPosition")
    switch (GetPlayerState()) {
        case STATE_PAUSE:
            VioletMediaPlayer::Resume();
            break;
        case STATE_STOP:
        case STATE_UNKNOWN:
            return;
        default:;
    }
    if (m_AudioDecoder) {
        m_AudioDecoder->SeekPosition(position);
    }
    if (m_VideoDecoder) {
        m_VideoDecoder->SeekPosition(position);
    }
}

Frame *VioletMediaPlayer::GetOneFrame(int type) {
    Frame *frame = nullptr;
    if (GetPlayerState() == STATE_STOP) return frame;
    if (type == MEDIA_TYPE_VIDEO) {
        frame = m_VideoFrameQueue->poll();
        if (frame) m_AVSync->SyncVideo(frame);
    } else {
        frame = m_AudioFrameQueue->poll();
        if (frame) m_AVSync->SyncAudio(frame);
    }
    return frame;
}

void VioletMediaPlayer::OnDecodeOneFrame(Frame *frame) {
//    LOGCATE("VioletMediaPlayer::OnDecodeOneFrame MediaType=%d", frame->type)
    if (GetPlayerState() == STATE_STOP) return;
    if (frame->type == MEDIA_TYPE_VIDEO) {
        m_VideoFrameQueue->offer(frame);
    } else {
        m_AudioFrameQueue->offer(frame);
    }
}

void VioletMediaPlayer::OnSeekResult(int mediaType, bool result) {
    m_EventCallback->PostMessage(EVENT_SEEK_FINISH, 0);
    if (result) {
        if (mediaType == MEDIA_TYPE_VIDEO) {
            m_VideoFrameQueue->clearCache();
        }
        if (mediaType == MEDIA_TYPE_AUDIO) {
            m_AudioFrameQueue->clearCache();
        }
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

