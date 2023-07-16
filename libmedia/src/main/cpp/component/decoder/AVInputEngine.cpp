//
// Created by bronyna on 2023/2/5.
//

#include "AVInputEngine.h"

#include <utility>

AVInputEngine::AVInputEngine(const char *path, shared_ptr<MediaEventCallback> event_cb,
                             DecoderCallback *decoder_cb) {
    this->m_EventCallback = event_cb;
    this->m_DecoderCallback = decoder_cb;
    strcpy(m_Path, path);

    m_VideoEnable = false;
    m_VideoStreamIndex = 0;
    m_VideoTimebase = 0;

    m_AudioEnable = false;
    m_AudioStreamIndex = 0;
    m_AudioTimebase = 0;

    m_Pkt = nullptr;
}

int AVInputEngine::_Init() {
    LOGCATE("AVInputEngine::_Init")
    int result = 0;

    if (m_DecoderCallback == nullptr) {
        LOGCATE("AVInputEngine::_Init m_DecoderCallback==nullptr")
        return result;
    }
    do {
        // 创建封装格式上下文
        m_AVFormatContext = avformat_alloc_context();

        // 打开文件
        result = avformat_open_input(&m_AVFormatContext, m_Path, nullptr, nullptr);
        if (result < 0) {
            LOGCATE("AVInputEngine::_Init avformat_open_input fail")
            break;
        }

        // 获取音视频流信息
        result = avformat_find_stream_info(m_AVFormatContext, nullptr);
        if (result < 0) {
            LOGCATE("AVInputEngine::_Init avformat_find_stream_info fail")
            break;
        }

        m_VideoStreamIndex = av_find_best_stream(m_AVFormatContext, AVMEDIA_TYPE_VIDEO, -1,
                                                 -1, nullptr, 0);
        LOGCATE("AVInputEngine::FindVideoStream, video_stream_index=%d", m_VideoStreamIndex)
        if (m_VideoStreamIndex >= 0) {
            m_VideoCodec = make_shared<VideoFFDecoder>(m_DecoderCallback);
            AVCodecParameters *m_VideoParam = m_AVFormatContext->streams[m_VideoStreamIndex]->codecpar;
            m_VideoTimebase = av_q2d(m_AVFormatContext->streams[m_VideoStreamIndex]->time_base);
            result = m_VideoCodec->OpenCodec(m_VideoParam);
            m_VideoEnable = result >= 0;
        }

        m_AudioStreamIndex = av_find_best_stream(m_AVFormatContext, AVMEDIA_TYPE_AUDIO, -1,
                                                 -1, nullptr, 0);
        if (m_AudioStreamIndex >= 0) {
            LOGCATE("AVInputEngine::FindAudioStream, audio_stream_index=%d", m_AudioStreamIndex)
            m_AudioCodec = make_shared<AudioFFDecoder>(m_DecoderCallback);
            AVCodecParameters *m_AudioParam = m_AVFormatContext->streams[m_AudioStreamIndex]->codecpar;
            m_AudioTimebase = av_q2d(m_AVFormatContext->streams[m_AudioStreamIndex]->time_base);
            result = m_AudioCodec->OpenCodec(m_AudioParam);
            m_AudioEnable = result >= 0;
        }
        m_Pkt = av_packet_alloc();

        if (m_EventCallback) {
            m_EventCallback->PostMessage(EVENT_DURATION, m_AVFormatContext->duration / 1000);
        }
    } while (false);
    return result;
}

int AVInputEngine::_UnInit() {
    LOGCATE("AVInputEngine::_UnInit start");
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
    LOGCATE("AVInputEngine::unInit finish");
    return 0;
}

void AVInputEngine::_DecoderLoop() {
//    LOGCATE("AVInputEngine::decodeLoop");
    while (m_DecoderCallback->GetPlayerState() == STATE_PAUSE) {
        av_usleep(10 * 1000);
    }
    if (DecoderLoopOnce() != 0 || m_DecoderCallback->GetPlayerState() == STATE_STOP) {
        disableAutoLoop();
    }
}

int AVInputEngine::DecoderLoopOnce() {
//    LOGCATE("AVInputEngine::DecoderLoopOnce m_MediaType=%d", m_MediaType)
    if (m_SeekPosition > 0) {
        auto seek_target = static_cast<int64_t>(m_SeekPosition * 1000000);
        int64_t seek_min = INT64_MIN;
        int64_t seek_max = INT64_MAX;
        int ret = avformat_seek_file(m_AVFormatContext, -1,
                                     seek_min, seek_target, seek_max, 0);
        LOGCATE("seek_result:%d", ret)
        if (ret >= 0) {
            if (m_AudioCodec) {
                m_AudioCodec->flush();
            }
            if (m_VideoCodec) {
                m_VideoCodec->flush();
            }
            LOGCATE("AVInputEngine::DecoderLoopOnce success while seeking")
            m_SeekFinish = true;
        }
        m_SeekPosition = -1;   // 重置seek标志位
    }

    int result = av_read_frame(m_AVFormatContext, m_Pkt);
    while (result == 0) {
        if (m_Pkt->stream_index == m_VideoStreamIndex && m_VideoEnable) {
            result = m_VideoCodec->Decode(m_Pkt, m_VideoTimebase);
        }
        if (m_Pkt->stream_index == m_AudioStreamIndex && m_AudioEnable) {
            result = m_AudioCodec->Decode(m_Pkt, m_AudioTimebase);
        }

        switch (result) {
            case AVERROR(EAGAIN):
                av_packet_unref(m_Pkt);
                result = av_read_frame(m_AVFormatContext, m_Pkt);
                break;
            case AVERROR_EOF:
                m_DecoderCallback->SetPlayerState(STATE_PAUSE); // 暂停整个播放器
                goto __EXIT;
            case AVERROR(EINVAL):
            case AVERROR_INPUT_CHANGED:
            case AVERROR(ENOMEM):
            default:
                break;
        }

        if (m_DecoderCallback->GetPlayerState() == STATE_STOP) break;
    }

    __EXIT:
    av_packet_unref(m_Pkt);
    return result;
}

void AVInputEngine::_SeekPosition(float timestamp) {
    m_SeekPosition = timestamp;
}

AVInputEngine::~AVInputEngine() {
    InputEngine::UnInit();
    m_DecoderCallback = nullptr;
    m_EventCallback = nullptr;
}
