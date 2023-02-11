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
    m_AudioFrameQueue = new ThreadSafeQueue(MAX_AUDIO_QUEUE_SIZE);

    m_VideoDecoder = new VideoDecoder(url, true, this);
    m_VideoRender = new NativeVideoRender(jniEnv, surface, this);
    m_VideoFrameQueue = new ThreadSafeQueue(MAX_VIDEO_QUEUE_SIZE);

    m_AVSync = new MediaSync();
    return result;
}

Frame *FFMediaPlayer::GetOneFrame(int type) {
    Frame *frame;
    if (type == FRAME_TYPE_VIDEO) {
        frame = m_VideoFrameQueue->poll();
        m_AVSync->videoSyncToSystemClock(frame->dts, frame->pts);
    } else {
        frame = m_AudioFrameQueue->poll();
        m_AVSync->audioSyncToSystemClock(frame->dts, frame->pts);
    }
    return frame;
}

int FFMediaPlayer::UnInit() {
    unInitAudioPlayer();
    unInitVideoPlayer();
    return 0;
}

FFMediaPlayer::~FFMediaPlayer() {
    FFMediaPlayer::UnInit();
}

void FFMediaPlayer::unInitAudioPlayer() {
    if (m_AudioDecoder) {
        m_AudioDecoder->unInit();
        delete m_AudioDecoder;
        m_AudioDecoder = nullptr;
    }
    if (m_AudioRender) {
        m_AudioRender->unInit();
        delete m_AudioRender;
        m_AudioRender = nullptr;
    }
    if (m_AudioFrameQueue) {
        delete m_AudioFrameQueue;
        m_AudioFrameQueue = nullptr;
    }
}

void FFMediaPlayer::unInitVideoPlayer() {
    if (m_VideoDecoder) {
        m_VideoDecoder->unInit();
        delete m_VideoDecoder;
        m_VideoDecoder = nullptr;
    }

    if (m_VideoRender) {
        m_VideoRender->unInit();
        delete m_VideoRender;
        m_VideoRender = nullptr;
    }

    if (m_VideoFrameQueue) {
        delete m_VideoFrameQueue;
        m_VideoFrameQueue = nullptr;
    }
}

void FFMediaPlayer::Play() {
    SetPlayerState(STATE_PLAYING);
    m_AudioDecoder->startDecodeThread();
    m_VideoDecoder->startDecodeThread();
}

void FFMediaPlayer::Pause() {
    SetPlayerState(STATE_PAUSE);
}

void FFMediaPlayer::Resume() {
    SetPlayerState(STATE_PLAYING);
}

void FFMediaPlayer::Stop() {

}

void FFMediaPlayer::SeekToPosition(float position) {

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
    if (frame->type == FRAME_TYPE_VIDEO) {
        m_VideoFrameQueue->offer(frame);
    } else {
        m_AudioFrameQueue->offer(frame);
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
