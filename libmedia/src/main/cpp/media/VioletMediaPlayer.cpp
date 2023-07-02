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

    int result1 = initAudioPlayer(url);
    if (result1 != 0) {
        LOGCATE("VioletMediaPlayer::Init Audio Component init fail")
        unInitAudioPlayer();
    } else {
        LOGCATE("VioletMediaPlayer::Init Audio Component init success")
    }
    int result2 = initVideoPlayer(url);
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

int VioletMediaPlayer::UnInit() {
    LOGCATE("VioletMediaPlayer::UnInit")
    unInitAudioPlayer();
    unInitVideoPlayer();
    delete m_EventCallback;
    return 0;
}

int VioletMediaPlayer::initAudioPlayer(char *url) {
    int result = -1;
    m_AudioDecoder = new AudioDecoder(url, m_EventCallback, this);
    result = m_AudioDecoder->Init();
    if (result == 0) {
        m_AudioRender = new OpenSLAudioRender(this);
        result = m_AudioRender->Init();
        if (result == 0) {
            m_AudioFrameQueue = new ThreadSafeQueue(MAX_AUDIO_QUEUE_SIZE, MEDIA_TYPE_AUDIO);
        }
    }
    return result;
}

int VioletMediaPlayer::initVideoPlayer(char *url) {
    int result = -1;
    m_VideoDecoder = new VideoDecoder(url, nullptr, this);
    result = m_VideoDecoder->Init();
    if (result == 0) {
        m_ImageRenderWindow = new GLRenderWindow(this);
        m_VideoFrameQueue = new ThreadSafeQueue(MAX_VIDEO_QUEUE_SIZE, MEDIA_TYPE_VIDEO);
    }
    return result;
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
    if (m_AVSync) {
        m_AVSync->SyncTimeStampWhenResume();
    }
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
        if (frame) m_AVSync->VideoSyncToSystemClock(frame->pts);
    } else {
        frame = m_AudioFrameQueue->poll();
        if (frame) m_AVSync->AudioSyncToSystemClock(frame->pts);
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
    if (mediaType == MEDIA_TYPE_VIDEO && result) {
        m_VideoFrameQueue->clearCache();
        m_AVSync->VideoSeekToPositionSuccess();
    }
    if (mediaType == MEDIA_TYPE_AUDIO && result) {
        m_AudioFrameQueue->clearCache();
        m_AVSync->AudioSeekToPositionSuccess();
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

