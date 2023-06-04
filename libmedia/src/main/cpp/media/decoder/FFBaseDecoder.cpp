//
// Created by bronyna on 2023/2/5.
//

#include "FFBaseDecoder.h"
#include "LogUtil.h"
#include "MediaDef.h"

FFBaseDecoder::~FFBaseDecoder() {
    LOGCATE("FFBaseDecoder::~FFBaseDecoder ")
}

int FFBaseDecoder::init() {
    LOGCATE("FFBaseDecoder::init");
    int result = -1;

    if (m_Callback == nullptr) {
        LOGCATE("FFBaseDecoder::init m_Callback==nullptr");
        return result;
    }
    do {
        // 1. 创建封装格式上下文
        m_AVFormatContext = avformat_alloc_context();

        // 2. 打开文件
        if (avformat_open_input(&m_AVFormatContext, m_Path, NULL, NULL) != 0) {
            LOGCATE("FFBaseDecoder::init avformat_open_input fail")
            break;
        }

        // 3. 获取音视频流信息
        if (avformat_find_stream_info(m_AVFormatContext, NULL) < 0) {
            LOGCATE("FFBaseDecoder::init avformat_find_stream_info fail")
            break;
        }

        // 4. 寻找音视频流
        for (int i = 0; i < m_AVFormatContext->nb_streams; i++) {
            AVStream *stream = m_AVFormatContext->streams[i];
            if (m_MediaType == stream->codecpar->codec_type) {
                m_StreamIndex = i;
                break;
            }
        }

        if (m_StreamIndex == -1) {
            LOGCATE("FFBaseDecoder::init Fail to find stream index.")
            break;
        }

        // 5. 获取解码器参数
        AVCodecParameters *param = m_AVFormatContext->streams[m_StreamIndex]->codecpar;

        // 6.获取解码器
        m_AVCodec = avcodec_find_decoder(param->codec_id);
        if (m_AVCodec == nullptr) {
            LOGCATE("FFBaseDecoder::init avcodec_find_decoder fail.")
            break;
        }

        // 7. 创建解码器上下文
        m_AVCodecContext = avcodec_alloc_context3(m_AVCodec);
        if (avcodec_parameters_to_context(m_AVCodecContext, param) != 0) {
            LOGCATE("FFBaseDecoder::init avcodec_parameters_to_context fail.")
            break;
        }

        AVDictionary *pAVDictionary = nullptr;
        av_dict_set(&pAVDictionary, "buffer_size", "1024000", 0);
        av_dict_set(&pAVDictionary, "stimeout", "20000000", 0);
        av_dict_set(&pAVDictionary, "max_delay", "30000000", 0);
        av_dict_set(&pAVDictionary, "rtsp_transport", "tcp", 0);

        // 8. 打开解码器
        result = avcodec_open2(m_AVCodecContext, m_AVCodec, &pAVDictionary);
        if (result < 0) {
            LOGCATE("FFBaseDecoder::init avcodec_open2 fail.")
            break;
        }
        result = 0;

        if (m_EventCallback) {
            m_EventCallback->PostMessage(EVENT_DURATION, m_AVFormatContext->duration / 1000);
        }

        m_AVPacket = av_packet_alloc();
        m_AVFrame = av_frame_alloc();

    } while (false);
    return result;
}

int FFBaseDecoder::unInit() {
    LOGCATE("FFBaseDecoder::unInit start");
    if (m_AVFrame != nullptr) {
        av_frame_free(&m_AVFrame);
        m_AVFrame = nullptr;
    }
    if (m_AVPacket != nullptr) {
        av_packet_free(&m_AVPacket);
        m_AVPacket = nullptr;
    }
    if (m_AVCodecContext != nullptr) {
        avcodec_close(m_AVCodecContext);
        avcodec_free_context(&m_AVCodecContext);
        m_AVCodec = nullptr;
        m_AVCodecContext = nullptr;
    }
    if (m_AVFormatContext != nullptr) {
        avformat_close_input(&m_AVFormatContext);
        avformat_free_context(m_AVFormatContext);
        m_AVFormatContext = nullptr;
    }
    if (m_EventCallback) {
        m_EventCallback = nullptr;
    }
    LOGCATE("FFBaseDecoder::unInit finish");
    return 0;
}

void FFBaseDecoder::decodeLoop() {
//    LOGCATE("FFBaseDecoder::decodeLoop");
    while (m_Callback->GetPlayerState() == STATE_PAUSE) {
        av_usleep(10 * 1000);
    }
    if (decodeLoopOnce() != 0 || m_Callback->GetPlayerState() == STATE_STOP) {
        disableAutoLoop();
    }
}

int FFBaseDecoder::decodeLoopOnce() {
    LOGCATE("FFBaseDecoder::decodeLoopOnce m_MediaType=%d", m_MediaType);
    if (m_SeekPosition > 0) {
        int64_t seek_target = static_cast<int64_t>(m_SeekPosition * 1000000);
        int64_t seek_min = INT64_MIN;
        int64_t seek_max = INT64_MAX;
        int seek_ret = avformat_seek_file(m_AVFormatContext, -1, seek_min,
                                          seek_target, seek_max, 0);
        if (seek_ret < 0) {
            m_Callback->OnSeekResult(m_MediaType, false);
            LOGCATE("BaseDecoder::decodeLoopOnce error while seeking m_MediaType=%d", m_MediaType);
        } else {
            if (-1 != m_StreamIndex) {
                avcodec_flush_buffers(m_AVCodecContext);
            }
            m_Callback->OnSeekResult(m_MediaType, true);
            LOGCATE("BaseDecoder::decodeLoopOnce success while seeking m_MediaType=%d",
                    m_MediaType);
        }
        m_SeekPosition = -1;   // 重置seek标志位
    }

    int result = av_read_frame(m_AVFormatContext, m_AVPacket);
    while (result == 0) {
        if (m_AVPacket->stream_index == m_StreamIndex) {
            if (avcodec_send_packet(m_AVCodecContext, m_AVPacket) == AVERROR_EOF) {
                // 解码结束
                result = -1;
                m_Callback->SetPlayerState(STATE_PAUSE); // 暂停整个播放器
                goto __EXIT;
            }

            int frameCount = 0;
            while (avcodec_receive_frame(m_AVCodecContext, m_AVFrame) == 0) {
                Frame *frame = onFrameAvailable();
                m_Callback->OnDecodeOneFrame(frame);
                if (m_EventCallback) {
                    m_EventCallback->PostMessage(EVENT_PLAYING, frame->pts);
                }
                frameCount++;
            }
//            LOGCATE("FFBaseDecoder::decodeLoopOnce frameCount=%d", frameCount);
            if (frameCount > 0) {
                result = 0;
                goto __EXIT;
            }
        }
        av_packet_unref(m_AVPacket);
        result = av_read_frame(m_AVFormatContext, m_AVPacket);
    }

    __EXIT:
    av_packet_unref(m_AVPacket);
    return result;
}

void FFBaseDecoder::seekPosition(float timestamp) {
    m_SeekPosition = timestamp;
}

