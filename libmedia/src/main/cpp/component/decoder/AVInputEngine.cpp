//
// Created by bronyna on 2023/2/5.
//

#include "AVInputEngine.h"
#include "LogUtil.h"
#include "MediaDef.h"

static int HwDecoderInit(AVInputStream *ist, const enum AVHWDeviceType type) {
    int err = 0;

    if ((err = av_hwdevice_ctx_create(&ist->hw_ctx, type,
                                      nullptr, nullptr, 0)) < 0) {
        fprintf(stderr, "Failed to create specified HW device.\n");
        return err;
    }
    ist->cc->hw_device_ctx = av_buffer_ref(ist->hw_ctx);

    return err;
}

static enum AVPixelFormat GetHwFormat(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts) {
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }

    fprintf(stderr, "Failed to get HW surface format.\n");
    return AV_PIX_FMT_NONE;
}

int AVInputEngine::_Init() {
    LOGCATE("AVInputEngine::init")
    int result = 0;
    int index;

    if (m_Callback == nullptr) {
        LOGCATE("AVInputEngine::init m_Callback==nullptr")
        return result;
    }
    do {
        // 创建封装格式上下文
        m_AVFormatContext = avformat_alloc_context();

        // 打开文件
        result = avformat_open_input(&m_AVFormatContext, m_Path, nullptr, nullptr);
        if (result < 0) {
            LOGCATE("AVInputEngine::init avformat_open_input fail")
            break;
        }

        // 获取音视频流信息
        result = avformat_find_stream_info(m_AVFormatContext, nullptr);
        if (result < 0) {
            LOGCATE("AVInputEngine::init avformat_find_stream_info fail")
            break;
        }

        index = av_find_best_stream(m_AVFormatContext, AVMEDIA_TYPE_VIDEO, -1,
                                    -1, nullptr, 0);
        LOGCATE("AVInputEngine::FindVideoStream, video_stream_index=%d", index)
        if (index >= 0) {
            m_VideoIst = new AVInputStream();
            m_VideoIst->fc = m_AVFormatContext;
            m_VideoIst->stream_index = index;
            result = OpenCodec(m_VideoIst);
            m_VideoEnable = result >= 0;
        }

        index = av_find_best_stream(m_AVFormatContext, AVMEDIA_TYPE_AUDIO, -1,
                                    -1, nullptr, 0);
        if (index >= 0) {
            LOGCATE("AVInputEngine::FindAudioStream, audio_stream_index=%d", index)
            m_AudioIst = new AVInputStream();
            m_AudioIst->fc = m_AVFormatContext;
            m_AudioIst->stream_index = index;
            result = OpenCodec(m_AudioIst);
            if (result >= 0) {
                InitAudioSwrContext();
            }
            m_AudioEnable = result >= 0;
        }

        m_Frame = av_frame_alloc();
        m_Pkt = av_packet_alloc();
    } while (false);
    return result;
}

int AVInputEngine::OpenCodec(AVInputStream *ist) {
    LOGCATE("AVInputEngine::OpenCodec")
    const AVCodec *codec;
    int result;
    do {
        if (FindCodecAndAllocCtx(ist) < 0) {
            LOGCATE("AVInputEngine::OpenCodec FindCodecAndAllocCtx fail.")
            result = -1;
            break;
        } else {
            codec = ist->c;
        }
        LOGCATE("AVInputEngine::OpenCodec avcodec_find_decoder name:%s", codec->name)

        AVDictionary *pAVDictionary = nullptr;
        av_dict_set(&pAVDictionary, "buffer_size", "1024000", 0);
        av_dict_set(&pAVDictionary, "stimeout", "20000000", 0);
        av_dict_set(&pAVDictionary, "max_delay", "30000000", 0);
        av_dict_set(&pAVDictionary, "rtsp_transport", "tcp", 0);

        // 打开解码器
        result = avcodec_open2(ist->cc, codec, &pAVDictionary);
        if (result < 0) {
            LOGCATE("AVInputEngine::OpenCodec avcodec_open2 fail.")
            break;
        }

        if (m_EventCallback) {
            m_EventCallback->PostMessage(EVENT_DURATION, m_AVFormatContext->duration / 1000);
        }
    } while (false);
    return result;
}

int AVInputEngine::FindCodecAndAllocCtx(AVInputStream *ist) {
    bool supportHwCodec;
    bool findHwCodec = false;
    enum AVHWDeviceType deviceType;
    enum AVMediaType mediaType;
    // 获取解码器参数
    AVCodecParameters *param;
    AVCodecID codecId;
    int result = -1;
    int i;

    param = ist->fc->streams[ist->stream_index]->codecpar;
    mediaType = param->codec_type;
    codecId = param->codec_id;

    deviceType = av_hwdevice_find_type_by_name("mediacodec");
    supportHwCodec = deviceType != AV_HWDEVICE_TYPE_NONE;
    if (!supportHwCodec) {
        while ((deviceType = av_hwdevice_iterate_types(deviceType)) != AV_HWDEVICE_TYPE_NONE) {
            LOGCATE("AVInputEngine::OpenCodec Available device types:%s",
                    av_hwdevice_get_type_name(deviceType))
        }
    }

    if (supportHwCodec) {
        if (mediaType == AVMEDIA_TYPE_VIDEO) {
            // 尝试根据名称查找硬解码器
            switch (codecId) {
                case AV_CODEC_ID_H264:
                    ist->c = avcodec_find_decoder_by_name("h264_mediacodec");
                    break;
                case AV_CODEC_ID_HEVC:
                    ist->c = avcodec_find_decoder_by_name("h265_mediacodec");
                    break;
                default:;
            }
        } else {
            // TODO 目前FFmpeg不支持音频硬解码
        }

        if (!ist->c) goto __EXIT;

        for (i = 0;; i++) {
            const AVCodecHWConfig *config = avcodec_get_hw_config(ist->c, i);
            if (!config) {
                LOGCATE("Decoder %s does not support device type %s.\n", ist->c->name,
                        av_hwdevice_get_type_name(deviceType))
            } else if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
                       config->device_type == deviceType) {
                ist->pix_fmt = config->pix_fmt;
                LOGCATE("%s support pix format: %d.\n", ist->c->name, config->pix_fmt)
                findHwCodec = true;
                break;
            }
        }
    }

    __EXIT:

    if (!ist->c) {
        ist->c = avcodec_find_decoder(codecId);
    }
    if (ist->c) {
        ist->cc = avcodec_alloc_context3(ist->c);
        result = avcodec_parameters_to_context(ist->cc, param);
        if (result < 0) {
            LOGCATE("AVInputEngine::init avcodec_parameters_to_context fail.")
        }
    }
    if (findHwCodec) {
        ist->cc->get_format = GetHwFormat;
        HwDecoderInit(ist, deviceType);
    }
    return result;
}

int AVInputEngine::InitAudioSwrContext() {
    AVCodecContext *c = m_AudioIst->cc;
    m_SwrContext = swr_alloc();
    av_opt_set_int(m_SwrContext, "in_channel_layout", c->channel_layout, 0);
    av_opt_set_int(m_SwrContext, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);

    av_opt_set_int(m_SwrContext, "in_sample_rate", c->sample_rate, 0);
    av_opt_set_int(m_SwrContext, "out_sample_rate", AUDIO_DST_SAMPLE_RATE, 0);

    av_opt_set_sample_fmt(m_SwrContext, "in_sample_fmt", c->sample_fmt, 0);
    av_opt_set_sample_fmt(m_SwrContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    swr_init(m_SwrContext);

    LOGCATE("AVInputEngine::init audio metadata sample rate: %d, channel: %d, format: %d, frame_size: %d, layout: %lu",
            c->sample_rate, c->channels, c->sample_fmt, c->frame_size, c->channel_layout)
    m_nbSamples = (int) av_rescale_rnd(ACC_NB_SAMPLES, AUDIO_DST_SAMPLE_RATE, c->sample_rate,
                                       AV_ROUND_UP);
    m_DstFrameDataSize = av_samples_get_buffer_size(nullptr, AUDIO_DST_CHANNEL_COUNTS,
                                                    m_nbSamples, AV_SAMPLE_FMT_S16, 1);

    LOGCATE("AVInputEngine::init [m_nbSamples, m_DstFrameDataSze]=[%d, %d]",
            m_nbSamples, m_DstFrameDataSize);

    m_AudioOutBuffer = (uint8_t *) malloc(m_DstFrameDataSize);
    return 0;
}


int AVInputEngine::_UnInit() {
    LOGCATE("AVInputEngine::unInit start");
    if (m_VideoIst) {
        delete m_VideoIst;
        m_VideoIst = nullptr;
    }
    if (m_AudioIst) {
        delete m_AudioIst;
        m_AudioIst = nullptr;
    }
    if (m_Frame) {
        av_frame_free(&m_Frame);
        m_Frame = nullptr;
    }
    if (m_Pkt) {
        av_packet_free(&m_Pkt);
        m_Pkt = nullptr;
    }
    if (m_AVFormatContext != nullptr) {
        avformat_close_input(&m_AVFormatContext);
        avformat_free_context(m_AVFormatContext);
        m_AVFormatContext = nullptr;
    }
    if (m_SwrContext) {
        swr_free(&m_SwrContext);
        m_SwrContext = nullptr;
    }
    if (m_AudioOutBuffer) {
        delete m_AudioOutBuffer;
        m_AudioOutBuffer = nullptr;
    }
    if (m_EventCallback) {
        m_EventCallback = nullptr;
    }
    LOGCATE("AVInputEngine::unInit finish");
    return 0;
}

void AVInputEngine::_DecoderLoop() {
//    LOGCATE("AVInputEngine::decodeLoop");
    while (m_Callback->GetPlayerState() == STATE_PAUSE) {
        av_usleep(10 * 1000);
    }
    if (DecoderLoopOnce() != 0 || m_Callback->GetPlayerState() == STATE_STOP) {
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
            avcodec_flush_buffers(m_VideoIst->cc);
            avcodec_flush_buffers(m_AudioIst->cc);

            LOGCATE("AVInputEngine::DecoderLoopOnce success while seeking")
            m_VideoSeekFinish = true;
            m_AudioSeekFinish = true;
        }
        m_SeekPosition = -1;   // 重置seek标志位
    }

    int result = av_read_frame(m_AVFormatContext, m_Pkt);
    while (result == 0) {
        AVInputStream *ist;
        if (m_Pkt->stream_index == m_AudioIst->stream_index) {
            ist = m_AudioIst;
        }
        if (m_Pkt->stream_index == m_VideoIst->stream_index) {
            ist = m_VideoIst;
        }
        if (ist == nullptr) {
            goto __EXIT;
        }

        if (avcodec_send_packet(ist->cc, m_Pkt) == AVERROR_EOF) {
            // 解码结束
            result = -1;
            m_Callback->SetPlayerState(STATE_PAUSE); // 暂停整个播放器
            goto __EXIT;
        }

        int frameCount = 0;
        while (avcodec_receive_frame(ist->cc, m_Frame) == 0) {
            Frame *frame = nullptr;
            if (ist->cc->codec_type == AVMEDIA_TYPE_AUDIO) {
                frame = AudioFrameAvailable();
            }
            if (ist->cc->codec_type == AVMEDIA_TYPE_VIDEO) {
                frame = VideoFrameAvailable();
            }
            if (frame) {
                if (m_AudioSeekFinish && frame->type == MEDIA_TYPE_AUDIO) {
                    frame->flag = FLAG_SEEK_FINISH;
                    m_AudioSeekFinish = false;
                    LOGCATE("AVInputEngine VideoSeekSuccess=%ld", frame->pts)
                }
                if (m_VideoSeekFinish && frame->type == MEDIA_TYPE_VIDEO) {
                    frame->flag = FLAG_SEEK_FINISH;
                    m_VideoSeekFinish = false;
                    LOGCATE("AVInputEngine AudioSeekSuccess=%ld", frame->pts)
                }
                m_Callback->OnDecodeOneFrame(frame);
                frameCount++;
            }
        }
//            LOGCATE("AVInputEngine::DecoderLoopOnce frameCount=%d", frameCount);
        if (frameCount > 0) {
            result = 0;
            goto __EXIT;
        }

        av_packet_unref(m_Pkt);
        result = av_read_frame(m_AVFormatContext, m_Pkt);
    }

    __EXIT:
    av_packet_unref(m_Pkt);
    return result;
}

Frame *AVInputEngine::AudioFrameAvailable() {
    LOGCATE("AVInputEngine::AudioFrameAvailable");
    AVFormatContext *oc = m_AVFormatContext;
    AVCodecContext *c = m_AudioIst->cc;
    int index = m_AudioIst->stream_index;

    int result = swr_convert(m_SwrContext, &m_AudioOutBuffer, m_DstFrameDataSize / 2,
                             (const uint8_t **) m_Frame->data, m_Frame->nb_samples);
    Frame *frame = nullptr;
    if (result > 0) {
        auto *data = (uint8_t *) malloc(m_DstFrameDataSize);
        memcpy(data, m_AudioOutBuffer, m_DstFrameDataSize);

        AVRational timeBase = oc->streams[index]->time_base;
        long dts = (long) ((m_Frame->pkt_dts * av_q2d(timeBase)) * 1000);
        long pts = (long) ((m_Frame->pts * av_q2d(timeBase)) * 1000);
        int channels = c->channels;
        int sampleRate = c->sample_rate;

        frame = new AudioFrame(data, m_DstFrameDataSize, channels, sampleRate,
                               dts, pts, -1);
    }

//        LOGCATE("AudioDecoder::onFrameAvailable data_size=%d channels=%d sampleRate=%d dts=%ld pts=%ld format=%d",
//                m_DstFrameDataSize, channels, sampleRate, dts, pts, -1);
    return frame;
}

Frame *AVInputEngine::VideoFrameAvailable() {
    LOGCATE("AVInputEngine::VideoFrameAvailable");
    auto *frame = new VideoFrame();
    AVFormatContext *oc = m_VideoIst->fc;
    AVCodecContext *c = m_VideoIst->cc;
    int index = m_VideoIst->stream_index;

    AVRational timeBase = oc->streams[index]->time_base;
    long dts = (long) ((m_Frame->pkt_dts * av_q2d(timeBase)) * 1000);
    long pts = (long) ((m_Frame->pts * av_q2d(timeBase)) * 1000);

    switch (c->pix_fmt) {
        case AV_PIX_FMT_RGBA: {
            break;
        }
        case AV_PIX_FMT_YUV420P:
        case AV_PIX_FMT_YUVJ420P: {
            int yPlaneByteSize = m_Frame->width * m_Frame->height;
            int uvPlaneByteSize = yPlaneByteSize / 2;
            auto *data = (uint8_t *) malloc(yPlaneByteSize + uvPlaneByteSize);

            frame->format = IMAGE_FORMAT_I420;
            frame->width = m_Frame->width;
            frame->height = m_Frame->height;
            frame->yuvBuffer[0] = data;
            frame->yuvBuffer[1] = data + yPlaneByteSize;
            frame->yuvBuffer[2] = data + yPlaneByteSize + uvPlaneByteSize / 2;
            memcpy(frame->yuvBuffer[0], m_Frame->data[0], yPlaneByteSize);
            memcpy(frame->yuvBuffer[1], m_Frame->data[1], uvPlaneByteSize / 2);
            memcpy(frame->yuvBuffer[2], m_Frame->data[2], uvPlaneByteSize / 2);
            frame->planeSize[0] = m_Frame->linesize[0];
            frame->planeSize[1] = m_Frame->linesize[1];
            frame->planeSize[2] = m_Frame->linesize[2];
            frame->dts = dts;
            frame->pts = pts;

            if (frame->yuvBuffer[0] && frame->yuvBuffer[1] && !frame->yuvBuffer[2] &&
                frame->planeSize[0] == frame->planeSize[1] && frame->planeSize[2] == 0) {
                // on some android device, output of h264 mediacodec decoder is NV12 兼容某些设备可能出现的格式不匹配问题
                frame->format = IMAGE_FORMAT_NV12;
            }
            break;
        }
        case AV_PIX_FMT_NV12: {
            int yPlaneByteSize = m_Frame->width * m_Frame->height;
            int uvPlaneByteSize = yPlaneByteSize / 2;
            auto *data = (uint8_t *) malloc(yPlaneByteSize + uvPlaneByteSize);

            frame->format = IMAGE_FORMAT_NV12;
            frame->width = m_Frame->width;
            frame->height = m_Frame->height;
            frame->yuvBuffer[0] = data;
            frame->yuvBuffer[1] = data + yPlaneByteSize;
            memcpy(frame->yuvBuffer[0], m_Frame->data[0], yPlaneByteSize);
            memcpy(frame->yuvBuffer[1], m_Frame->data[1], uvPlaneByteSize);
            frame->planeSize[0] = m_Frame->linesize[0];
            frame->planeSize[1] = m_Frame->linesize[1];
            frame->dts = dts;
            frame->pts = pts;
            break;
        }
        case AV_PIX_FMT_NV21: {
            int yPlaneByteSize = m_Frame->width * m_Frame->height;
            int uvPlaneByteSize = yPlaneByteSize / 2;
            auto *data = (uint8_t *) malloc(yPlaneByteSize + uvPlaneByteSize);

            frame->format = IMAGE_FORMAT_NV21;
            frame->width = m_Frame->width;
            frame->height = m_Frame->height;
            frame->yuvBuffer[0] = data;
            frame->yuvBuffer[1] = data + yPlaneByteSize;
            memcpy(frame->yuvBuffer[0], m_Frame->data[0], yPlaneByteSize);
            memcpy(frame->yuvBuffer[1], m_Frame->data[1], uvPlaneByteSize);
            frame->planeSize[0] = m_Frame->linesize[0];
            frame->planeSize[1] = m_Frame->linesize[1];
            frame->dts = dts;
            frame->pts = pts;
            break;
        }
        default:
            delete frame;
    }

    return frame;
}

void AVInputEngine::_SeekPosition(float timestamp) {
    m_SeekPosition = timestamp;
}

