//
// Created by bronyna on 2023/2/5.
//

#include "FFMediaInputEngine.h"

#include <utility>

FFMediaInputEngine::FFMediaInputEngine(const char *path, shared_ptr<MediaEventCallback> event_cb,
                                       AVDecoderCallback *decoder_cb) {
    this->m_EventCallback = event_cb;
    this->m_DecoderCallback = decoder_cb;
    strcpy(m_Path, path);
    m_VideoStreamIndex = 0;
    m_AudioStreamIndex = 0;
    m_Pkt = nullptr;
    sem_init(&runBlock, 0, 0);
}

int FFMediaInputEngine::_Init() {
    DEBUG_LOGCATE("")
    int result = 0;

    if (m_DecoderCallback == nullptr) {
        DEBUG_LOGCATE("m_DecoderCallback is NULL")
        return result;
    }
    // 创建封装格式上下文
    m_AVFormatContext = avformat_alloc_context();

    // 打开文件
    result = avformat_open_input(&m_AVFormatContext, m_Path, nullptr, nullptr);
    if (result < 0) {
        DEBUG_LOGCATE("avformat_open_input fail")
        goto __EXIT;
    }

    // 获取音视频流信息
    result = avformat_find_stream_info(m_AVFormatContext, nullptr);
    if (result < 0) {
        DEBUG_LOGCATE("avformat_find_stream_info fail")
        goto __EXIT;
    }

    m_VideoStreamIndex = av_find_best_stream(m_AVFormatContext, AVMEDIA_TYPE_VIDEO, -1,
                                             -1, nullptr, 0);
    LOGCATE("FFMediaInputEngine::FindVideoStream, video_stream_index=%d", m_VideoStreamIndex)
    if (m_VideoStreamIndex >= 0) {
        AVCodecParameters *m_VideoParam = m_AVFormatContext->streams[m_VideoStreamIndex]->codecpar;
        double m_VideoTimebase = av_q2d(
                m_AVFormatContext->streams[m_VideoStreamIndex]->time_base);
        m_VideoCodec = make_shared<FFVideoDecoder>(m_DecoderCallback, m_VideoTimebase);
        result = m_VideoCodec->OpenCodec(m_VideoParam);
    }

    m_AudioStreamIndex = av_find_best_stream(m_AVFormatContext, AVMEDIA_TYPE_AUDIO, -1,
                                             -1, nullptr, 0);
    if (m_AudioStreamIndex >= 0) {
        LOGCATE("FFMediaInputEngine::FindAudioStream, audio_stream_index=%d",
                m_AudioStreamIndex)
        AVCodecParameters *m_AudioParam = m_AVFormatContext->streams[m_AudioStreamIndex]->codecpar;
        double m_AudioTimebase = av_q2d(
                m_AVFormatContext->streams[m_AudioStreamIndex]->time_base);
        m_AudioCodec = make_shared<FFAudioDecoder>(m_DecoderCallback, m_AudioTimebase);
        result = m_AudioCodec->OpenCodec(m_AudioParam);
    }
    m_Pkt = av_packet_alloc();

    if (m_EventCallback) {
        m_EventCallback->PostMessage(EVENT_DURATION, m_AVFormatContext->duration / 1000);
    }

    __EXIT:
    return result;
}

int FFMediaInputEngine::_UnInit() {
    if (m_Pkt) {
        av_packet_free(&m_Pkt);
        m_Pkt = nullptr;
    }
    if (m_AVFormatContext != nullptr) {
        avformat_close_input(&m_AVFormatContext);
        avformat_free_context(m_AVFormatContext);
        m_AVFormatContext = nullptr;
    }
    if (m_EventCallback) {
        m_EventCallback = nullptr;
    }
    DEBUG_LOGCATE("finish")
    return 0;
}

void FFMediaInputEngine::_DecodeLoop() {
    DEBUG_LOGCATE("")
    while (m_DecoderCallback->GetPlayerState() == STATE_PAUSE) {
        av_usleep(10 * 1000);
    }
    if (_DecoderLoopOnce() == AVERROR_EOF || m_DecoderCallback->GetPlayerState() == STATE_STOP) {
        disableAutoLoop();
    }
}

int FFMediaInputEngine::_DecoderLoopOnce() {
//    LOGCATE("FFMediaInputEngine::_DecoderLoopOnce")
    if (m_SeekPosition > 0) {
        auto seek_target = static_cast<int64_t>(m_SeekPosition * 1000000);
        int64_t seek_min = INT64_MIN;
        int64_t seek_max = INT64_MAX;
        int ret = avformat_seek_file(m_AVFormatContext, -1,
                                     seek_min, seek_target, seek_max, 0);
        LOGCATE("seek_result:%d", ret)
        if (ret >= 0) {
            if (m_AudioCodec) {
                m_AudioCodec->FlushDecoder();
            }
            if (m_VideoCodec) {
                m_VideoCodec->FlushDecoder();
            }
            std::shared_ptr<AVMediaFrame> frame = std::make_shared<AVMediaFrame>();
            frame->flag = FLAG_SEEK_FINISH;
            m_DecoderCallback->OnFrameReady(frame);
            LOGCATE("FFMediaInputEngine::_DecoderLoopOnce success while seeking")
        }
        m_SeekPosition = -1;   // 重置seek标志位
    }

    shared_ptr<FFBaseDecoder> codec;
    int result = 0;
    while (true) {
        result = av_read_frame(m_AVFormatContext, m_Pkt);
        if (result < 0) break;
        if (m_Pkt->stream_index == m_VideoStreamIndex) {
            codec = m_VideoCodec;
        } else {
            codec = m_AudioCodec;
        }

        result = codec->DecodeInput(m_Pkt);
        if (result < 0) {
            if (result == AVERROR(EAGAIN)) {
                codec->DecodeOutput();
                codec->DecodeInput(m_Pkt);
            } else if (result == AVERROR_EOF) {
                // AVERROR_EOF \ AVERROR(EINVAL) \ AVERROR(ENOMEM)
                break;
            }
        }

        result = codec->DecodeOutput();
        if (result < 0) {
            if (result == AVERROR(EAGAIN)) {

            } else {
                // AVERROR_EOF \ AVERROR(EINVAL) \ AVERROR_INPUT_CHANGED
                break;
            }
        }

        if (m_DecoderCallback->GetPlayerState() == STATE_STOP) break;
    }

    av_packet_unref(m_Pkt);
    return result;
}

void FFMediaInputEngine::_SeekToPosition(float timestamp) {
    m_SeekPosition = timestamp;
}

int FFMediaInputEngine::Init() {
    post(MESSAGE_INIT, nullptr);
    sem_wait(&runBlock);
    return result;
}

int FFMediaInputEngine::UnInit() {
    disableAutoLoop();
    post(MESSAGE_UNINIT, nullptr);
    quit();
    return 0;
}

int FFMediaInputEngine::SeekPosition(float timestamp) {
    mTempStamp = timestamp;
    post(MESSAGE_SEEK, &mTempStamp);
    return 0;
}

void FFMediaInputEngine::StartLoop() {
    enableAutoLoop(&mLoopMsg);
}

void FFMediaInputEngine::handle(int what, void *data) {
    looper::handle(what, data);
    switch (what) {
        case MESSAGE_INIT:
            result = _Init();
            sem_post(&runBlock);
            break;
        case MESSAGE_LOOP:
            _DecodeLoop();
            break;
        case MESSAGE_SEEK: {
            float *ts = (float *) data;
            _SeekToPosition(*ts);
            break;
        }
        case MESSAGE_UNINIT:
            _UnInit();
            break;
    }
}


FFMediaInputEngine::~FFMediaInputEngine() {
    LOGCATE("FFMediaInputEngine::unInit 1")
    if (m_AudioCodec) {
        m_AudioCodec->StopDecoder();
    }
    if (m_VideoCodec) {
        m_VideoCodec->StopDecoder();
    }
    UnInit();
    LOGCATE("FFMediaInputEngine::unInit 2")
    m_DecoderCallback = nullptr;
    m_EventCallback = nullptr;
    sem_destroy(&runBlock);
}

