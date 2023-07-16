//
// Created by bronyna on 2023/2/5.
//

#include "VioletMediaPlayer.h"

int VioletMediaPlayer::Init(JNIEnv *jniEnv, jobject obj, char *url, int decodeType,
                            int renderType) {
    LOGCATE("VioletMediaPlayer::Init")

    m_EventCallback = shared_ptr<MediaEventCallback>(new MediaEventCallback(jniEnv, obj));

    m_InputEngine = shared_ptr<AVInputEngine>(new AVInputEngine(url, m_EventCallback, this));
    int result = m_InputEngine->Init();

    m_ImageRenderWindow = shared_ptr<GLRenderWindow>(new GLRenderWindow(this));
    m_VideoFrameQueue = make_shared<LinkedBlockingQueue<MediaFrame>>(MAX_VIDEO_QUEUE_SIZE);

    m_AudioRender = shared_ptr<OpenSLAudioRender>(new OpenSLAudioRender(this));
    m_AudioRender->Init();
    m_AudioFrameQueue = make_shared<LinkedBlockingQueue<MediaFrame>>(MAX_AUDIO_QUEUE_SIZE);

    m_AVSync = shared_ptr<MediaSync>(new MediaSync());
    return result;
}

int VioletMediaPlayer::UnInit() {
    LOGCATE("VioletMediaPlayer::UnInit")
    if (m_AudioFrameQueue) {
        m_AudioFrameQueue->overrule();
    }
    if (m_VideoFrameQueue) {
        m_VideoFrameQueue->overrule();
    }
    if (m_InputEngine) {
        m_InputEngine.reset();
    }
    if (m_AudioRender) {
        m_AudioRender.reset();
    }
    if (m_ImageRenderWindow) {
        m_ImageRenderWindow.reset();
    }
    return 0;
}

void VioletMediaPlayer::OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) {
    if (m_ImageRenderWindow) {
        m_ImageRenderWindow->OnSurfaceCreated(jniEnv, surface);
    }
}

void VioletMediaPlayer::OnSurfaceChanged(int width, int height) {
    if (m_ImageRenderWindow) {
        m_ImageRenderWindow->OnSurfaceChanged(width, height);
    }
}

void VioletMediaPlayer::OnSurfaceDestroyed() {
    if (m_ImageRenderWindow) {
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

shared_ptr<MediaFrame> VioletMediaPlayer::GetOneFrame(int type) {
//    LOGCATE("VioletMediaPlayer::GetOneFrame type=%d", type)
    shared_ptr<MediaFrame> frame;
    if (GetPlayerState() == STATE_STOP) return frame;
    if (type == MEDIA_TYPE_VIDEO) {
        frame = m_VideoFrameQueue->poll();
        if (frame) {
            m_AVSync->SyncVideo(frame->pts, frame->flag);
        }
    } else {
        frame = m_AudioFrameQueue->poll();
        if (frame) {
            m_AVSync->SyncAudio(frame->pts, frame->flag);
            m_EventCallback->PostMessage(EVENT_PLAYING, frame->pts);
        }
    }
    return frame;
}

void VioletMediaPlayer::FrameRendFinish(shared_ptr<MediaFrame> frame) {
    if (frame) {
        // TODO 此处用来扩展录制模块，暂时搁置
        if (frame->type == MEDIA_TYPE_VIDEO) {

        } else {

        }
    }
}

void VioletMediaPlayer::OnDecodeOneFrame(std::shared_ptr<MediaFrame> frame) {
//    LOGCATE("VioletMediaPlayer::OnDecodeOneFrame MediaType=%d", frame->type)
    if (GetPlayerState() == STATE_STOP) return;
    if (frame->flag == FLAG_SEEK_FINISH) {
        m_AudioFrameQueue->clear();
        m_VideoFrameQueue->clear();
    }

    if (frame->type == MEDIA_TYPE_VIDEO) {
        m_VideoFrameQueue->offer(frame);
    } else {
        m_AudioFrameQueue->offer(frame);
    }
}

int VioletMediaPlayer::GetPlayerState() {
    return state;
}

void VioletMediaPlayer::SetPlayerState(PlayerState state) {
    unique_lock<mutex> lock(m_Mutex);
    this->state = state;
    lock.unlock();
    m_Cond.notify_all();
}

