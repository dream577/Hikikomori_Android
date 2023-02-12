//
// Created by bronyna on 2023/2/5.
//

#include "FFMediaPlayer.h"
#include "LogUtil.h"
#include "ImageDef.h"

int FFMediaPlayer::Init(JNIEnv *jniEnv, jobject obj, char *url, int decodeType,
                        int renderType, jobject surface) {
    LOGCATE("FFMediaPlayer::Init")
    int result = 0;
    jniEnv->GetJavaVM(&m_JavaVM);
    m_JavaObj = jniEnv->NewGlobalRef(obj);

    m_AudioDecoder = new AudioDecoder(url, this);
    m_AudioRender = new OpenSLAudioRender(this);
    m_AudioFrameQueue = new ThreadSafeQueue(MAX_AUDIO_QUEUE_SIZE, MEDIA_TYPE_AUDIO);

    m_VideoDecoder = new VideoDecoder(url, true, this);
    m_VideoRender = new NativeVideoRender(jniEnv, surface, this);
    m_VideoFrameQueue = new ThreadSafeQueue(MAX_VIDEO_QUEUE_SIZE, MEDIA_TYPE_VIDEO);

    m_AVSync = new MediaSync();
    return result;
}

Frame *FFMediaPlayer::GetOneFrame(int type) {
    LOGCATE("FFMediaPlayer::GetOneFrame, MediaType=%d", type)
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

int FFMediaPlayer::UnInit() {
    LOGCATE("FFMediaPlayer::UnInit")
    unInitAudioPlayer();
    unInitVideoPlayer();
    return 0;
}

FFMediaPlayer::~FFMediaPlayer() {
//    FFMediaPlayer::UnInit();
}

void FFMediaPlayer::unInitAudioPlayer() {
    LOGCATE("FFMediaPlayer::unInitAudioPlayer")
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

void FFMediaPlayer::unInitVideoPlayer() {
    LOGCATE("FFMediaPlayer::unInitVideoPlayer")
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

void FFMediaPlayer::Play() {
    LOGCATE("FFMediaPlayer::Play")
    if (GetPlayerState() != STATE_UNKNOWN) return;
    SetPlayerState(STATE_PLAYING);
    m_AudioDecoder->startDecodeThread();
    m_VideoDecoder->startDecodeThread();
}

void FFMediaPlayer::Pause() {
    LOGCATE("FFMediaPlayer::Pause")
    if (GetPlayerState() != STATE_PLAYING) return;
    SetPlayerState(STATE_PAUSE);
}

void FFMediaPlayer::Resume() {
    LOGCATE("FFMediaPlayer::Resume")
    if (GetPlayerState() != STATE_PAUSE) return;
    if (m_AVSync) {
        m_AVSync->syncTimeStampWhenResume();
    }
    SetPlayerState(STATE_PLAYING);
}

void FFMediaPlayer::Stop() {
    LOGCATE("FFMediaPlayer::Stop")
    SetPlayerState(STATE_STOP);
    UnInit();
}

void FFMediaPlayer::SeekToPosition(float position) {
    LOGCATE("FFMediaPlayer::SeekToPosition")
    switch (GetPlayerState()) {
        case STATE_PAUSE:
            FFMediaPlayer::Resume();
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

void FFMediaPlayer::OnDecoderReady(int type) {
    int result;
    if (type == AVMEDIA_TYPE_VIDEO) {
        auto *decoder = (VideoDecoder *) m_VideoDecoder;
        auto *render = (VideoRender *) m_VideoRender;
        render->setVideoSize(decoder->getVideoWidth(), decoder->getVideoHeight());
        result = m_VideoRender->init();
        if (result != 0) {
            LOGCATE("FFMediaPlayer::Init Video Component init fail")
            unInitVideoPlayer();
        } else {
            LOGCATE("FFMediaPlayer::Init Video Component init success")
            decoder->setRenderSize(render->getRenderWidth(), render->getRenderHeight());
            m_VideoRender->startRenderThread();
        }
    } else {
        result = m_AudioRender->init();
        if (result != 0) {
            LOGCATE("FFMediaPlayer::Init Audio Component init fail")
            unInitAudioPlayer();
        } else {
            LOGCATE("FFMediaPlayer::Init Audio Component init success")
            m_AudioRender->startRenderThread();
        }
    }
}

void FFMediaPlayer::OnDecodeOneFrame(Frame *frame) {
    LOGCATE("FFMediaPlayer::OnDecodeOneFrame MediaType=%d", frame->type)
    if (GetPlayerState() == STATE_STOP) return;
    if (frame->type == MEDIA_TYPE_VIDEO) {
        m_VideoFrameQueue->offer(frame);
    } else {
        m_AudioFrameQueue->offer(frame);
    }
}

void FFMediaPlayer::OnSeekResult(int mediaType, bool result) {
    if (mediaType == MEDIA_TYPE_VIDEO && result) {
        m_VideoFrameQueue->clearCache();
        m_AVSync->videoSeekToPositionSuccess();
    }
    if (mediaType == MEDIA_TYPE_AUDIO && result) {
        m_AudioFrameQueue->clearCache();
        m_AVSync->audioSeekToPositionSuccess();
    }
}

int FFMediaPlayer::GetPlayerState() {
    return state;
}

void FFMediaPlayer::SetPlayerState(PlayerState state) {
    unique_lock<mutex> lock(m_Mutex);
    MediaPlayer::state = state;
    m_Cond.notify_all();
}
