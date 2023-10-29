//
// Created by bronyna on 2023/2/5.
//

#include "VioletMediaPlayer.h"

int VioletMediaPlayer::Init(JNIEnv *jniEnv, jobject obj, char *url, int decodeType,
                            int renderType) {
    LOGCATE("VioletMediaPlayer::Init")

    m_EventCallback = make_shared<MediaEventCallback>(jniEnv, obj);

    m_InputEngine = make_shared<FFMediaInputEngine>(url, m_EventCallback, this);
    int result = m_InputEngine->Init();

    m_ImageRenderWindow = make_shared<GLRenderWindow>(this);
    m_VideoFrameQueue = make_shared<LinkedBlockingQueue<MediaFrame>>(MAX_VIDEO_FRAME_SIZE);

    m_AudioRender = make_shared<OpenSLAudioRender>(this);
    m_AudioRender->Init();
    m_AudioFrameQueue = make_shared<LinkedBlockingQueue<MediaFrame>>(MAX_AUDIO_FRAME_SIZE);

    m_AVSync = make_shared<MediaSync>();
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
    if (m_ImageRenderWindow) {
        m_ImageRenderWindow.reset();
    }
    if (m_AudioRender) {
        m_AudioRender.reset();
    }
    if (m_InputEngine) {
        m_InputEngine.reset();
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

void VioletMediaPlayer::UpdateMVPMatrix(float translateX, float translateY, float scaleX,
                                        float scaleY, int degree, int mirror) {
    if (m_ImageRenderWindow) {
        m_ImageRenderWindow->UpdateMVPMatrix(translateX, translateY, scaleX,
                                             scaleY, degree, mirror);
    }
}

void VioletMediaPlayer::Play() {
    LOGCATE("VioletMediaPlayer::Play")
    if (GetPlayerState() != STATE_UNKNOWN) return;
    SetPlayerState(STATE_PLAYING);
    m_InputEngine->StartDecodeLoop();

    m_AudioRender->StartRenderLoop();
    m_ImageRenderWindow->StartRenderLoop();
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
    if (type == AVMEDIA_TYPE_VIDEO) {
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
        if (frame->type == AVMEDIA_TYPE_VIDEO) {
            MediaFrame::recycle(frame);
        } else {

        }
    }
}

void VioletMediaPlayer::OnFrameReady(std::shared_ptr<MediaFrame> frame) {
//    LOGCATE("VioletMediaPlayer::OnFrameReady MediaType=%d", frame->type)
    if (GetPlayerState() == STATE_STOP) return;
    if (frame->flag != FLAG_NONE) {
        switch (frame->flag) {
            case FLAG_SEEK_FINISH:
                m_AudioFrameQueue->clear();
                m_VideoFrameQueue->clear();
                break;
            default: {

            }
        }
        return;
    }

    if (frame->type == AVMEDIA_TYPE_VIDEO) {
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

void VioletMediaPlayer::StartRecord() {

}

void VioletMediaPlayer::StopRecord() {

}

