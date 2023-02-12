//
// Created by bronyna on 2023/2/5.
//

#include "VioletMediaPlayer.h"
#include "LogUtil.h"
#include "ImageDef.h"

int VioletMediaPlayer::Init(JNIEnv *jniEnv, jobject obj, char *url, int decodeType,
                            int renderType, jobject surface) {
    LOGCATE("VioletMediaPlayer::Init")
    int result = 0;
    jniEnv->GetJavaVM(&m_JavaVM);
    m_JavaObj = jniEnv->NewGlobalRef(obj);

    int result1 = initAudioPlayer(url);
    if (result1 != 0) {
        LOGCATE("VioletMediaPlayer::Init Audio Component init fail")
        unInitAudioPlayer();
    }
    int result2 = initVideoPlayer(jniEnv, surface, url);
    if (result2 != 0) {
        LOGCATE("VioletMediaPlayer::Init Video Component init fail")
        unInitVideoPlayer();
    }

    if (result1 == 0 || result2 == 0) {
        m_AVSync = new MediaSync();
    }
    return result | result2;
}

Frame *VioletMediaPlayer::GetOneFrame(int type) {
    LOGCATE("VioletMediaPlayer::GetOneFrame, MediaType=%d", type)
    Frame *frame = nullptr;
    if (GetPlayerState() == STATE_STOP) return frame;
    if (type == MEDIA_TYPE_VIDEO) {
        frame = m_VideoFrameQueue->poll();
        if (frame) m_AVSync->videoSyncToSystemClock(frame->pts);
    } else {
        frame = m_AudioFrameQueue->poll();
        if (frame) m_AVSync->audioSyncToSystemClock(frame->pts);
    }
    return frame;
}

int VioletMediaPlayer::UnInit() {
    LOGCATE("VioletMediaPlayer::UnInit")
    unInitAudioPlayer();
    unInitVideoPlayer();
    return 0;
}

VioletMediaPlayer::~VioletMediaPlayer() {
//    VioletMediaPlayer::UnInit();
}

int VioletMediaPlayer::initAudioPlayer(char *url) {
    int result = -1;
    m_AudioDecoder = new AudioDecoder(url, this);
    result = m_AudioDecoder->init();
    if (result == 0) {
        m_AudioRender = new OpenSLAudioRender(this);
        result = m_AudioRender->init();
        if (result == 0) {
            m_AudioFrameQueue = new ThreadSafeQueue(MAX_AUDIO_QUEUE_SIZE, MEDIA_TYPE_AUDIO);
        }
    }
    return result;
}

int VioletMediaPlayer::initVideoPlayer(JNIEnv *jniEnv, jobject surface, char *url) {
    int result = -1;
    m_VideoDecoder = new VideoDecoder(url, true, this);
    result = m_VideoDecoder->init();
    if (result == 0) {
        m_VideoRender = new VideoNativeRender(
                jniEnv, surface, m_VideoDecoder->GetAVCodecContext()->pix_fmt, this);
        m_VideoRender->setVideoSize(m_VideoDecoder->getVideoWidth(),
                                    m_VideoDecoder->getVideoHeight());

        result = m_VideoRender->init();
        if (result == 0) {
            m_VideoFrameQueue = new ThreadSafeQueue(MAX_VIDEO_QUEUE_SIZE, MEDIA_TYPE_VIDEO);
        }
    }
    return result;
}

void VioletMediaPlayer::unInitAudioPlayer() {
    LOGCATE("VioletMediaPlayer::unInitAudioPlayer")
    if (m_AudioFrameQueue) {
        m_AudioFrameQueue->abort();
    }
    if (m_AudioDecoder) {
        m_AudioDecoder->destroy();
        delete m_AudioDecoder;
        m_AudioDecoder = nullptr;
    }
    if (m_AudioFrameQueue) {
        delete m_AudioFrameQueue;
        m_AudioFrameQueue = nullptr;
    }
    if (m_AudioRender) {
        m_AudioRender->destroy();
        delete m_AudioRender;
        m_AudioRender = nullptr;
    }
}

void VioletMediaPlayer::unInitVideoPlayer() {
    LOGCATE("VioletMediaPlayer::unInitVideoPlayer")
    if (m_VideoFrameQueue) {
        m_VideoFrameQueue->abort();
    }
    if (m_VideoDecoder) {
        m_VideoDecoder->destroy();
        delete m_VideoDecoder;
        m_VideoDecoder = nullptr;
    }
    if (m_VideoFrameQueue) {
        delete m_VideoFrameQueue;
        m_VideoFrameQueue = nullptr;
    }
    if (m_VideoRender) {
        m_VideoRender->destroy();
        delete m_VideoRender;
        m_VideoRender = nullptr;
    }
}

void VioletMediaPlayer::Play() {
    LOGCATE("VioletMediaPlayer::Play")
    if (GetPlayerState() != STATE_UNKNOWN) return;
    SetPlayerState(STATE_PLAYING);
    if (m_AudioDecoder) {
        m_AudioDecoder->startDecodeThread();
        m_AudioRender->startRenderThread();
    }
    if (m_VideoDecoder) {
        m_VideoDecoder->startDecodeThread();
        m_VideoRender->startRenderThread();
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
        m_AVSync->syncTimeStampWhenResume();
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
        m_AudioDecoder->seekPosition(position);
    }
    if (m_VideoDecoder) {
        m_VideoDecoder->seekPosition(position);
    }
}

void VioletMediaPlayer::OnDecodeOneFrame(Frame *frame) {
    LOGCATE("VioletMediaPlayer::OnDecodeOneFrame MediaType=%d", frame->type)
    if (GetPlayerState() == STATE_STOP) return;
    if (frame->type == MEDIA_TYPE_VIDEO) {
        m_VideoFrameQueue->offer(frame);
    } else {
        m_AudioFrameQueue->offer(frame);
    }
}

void VioletMediaPlayer::OnSeekResult(int mediaType, bool result) {
    if (mediaType == MEDIA_TYPE_VIDEO && result) {
        m_VideoFrameQueue->clearCache();
        m_AVSync->videoSeekToPositionSuccess();
    }
    if (mediaType == MEDIA_TYPE_AUDIO && result) {
        m_AudioFrameQueue->clearCache();
        m_AVSync->audioSeekToPositionSuccess();
    }
}

int VioletMediaPlayer::GetPlayerState() {
    return state;
}

void VioletMediaPlayer::SetPlayerState(PlayerState state) {
    unique_lock<mutex> lock(m_Mutex);
    MediaPlayer::state = state;
    m_Cond.notify_all();
}

