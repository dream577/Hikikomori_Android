//
// Created by bronyna on 2023/2/5.
//

#include "AVInputEngine.h"
#include "LogUtil.h"
#include "MediaDef.h"

int AVInputEngine::_init() {
    LOGCATE("AVInputEngine::init");
    int result = -1;

    if (m_Callback == nullptr) {
        LOGCATE("AVInputEngine::init m_Callback==nullptr");
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

        // 寻找音视频流
        for (int i = 0; i < m_AVFormatContext->nb_streams; i++) {
            AVStream *stream = m_AVFormatContext->streams[i];
            if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && m_VideoIst == nullptr) {
                LOGCATE("AVInputEngine::init, video_stream_index=%d", i)
                m_VideoIst = new AVInputStream();
                m_VideoIst->fc = m_AVFormatContext;
                m_VideoIst->stream_index = i;
                result = OpenDecoder(m_VideoIst);
            }
            if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && m_AudioIst == nullptr) {
                LOGCATE("AVInputEngine::init, audio_stream_index=%d", i)
                m_AudioIst = new AVInputStream();
                m_AudioIst->fc = m_AVFormatContext;
                m_AudioIst->stream_index = i;
                result = OpenDecoder(m_AudioIst);
                if (result >= 0) {
                    InitAudioSwrContext();
                }
            }
        }

        m_Frame = av_frame_alloc();
        m_Pkt = av_packet_alloc();
    } while (false);
    return result;
}

int AVInputEngine::InitAudioSwrContext() {
    AVCodecContext *c = m_AudioIst->c;
    m_SwrContext = swr_alloc();
    av_opt_set_int(m_SwrContext, "in_channel_layout", c->channel_layout, 0);
    av_opt_set_int(m_SwrContext, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);

    av_opt_set_int(m_SwrContext, "in_sample_rate", c->sample_rate, 0);
    av_opt_set_int(m_SwrContext, "out_sample_rate", AUDIO_DST_SAMPLE_RATE, 0);

    av_opt_set_sample_fmt(m_SwrContext, "in_sample_fmt", c->sample_fmt, 0);
    av_opt_set_sample_fmt(m_SwrContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    swr_init(m_SwrContext);

    LOGCATE("AVInputEngine::init audio metadata sample rate: %d, channel: %d, format: %d, frame_size: %d, layout: %lu",
            c->sample_rate, c->channels, c->sample_fmt, c->frame_size, c->channel_layout);
    m_nbSamples = (int) av_rescale_rnd(ACC_NB_SAMPLES, AUDIO_DST_SAMPLE_RATE, c->sample_rate,
                                       AV_ROUND_UP);
    m_DstFrameDataSize = av_samples_get_buffer_size(nullptr, AUDIO_DST_CHANNEL_COUNTS,
                                                    m_nbSamples, AV_SAMPLE_FMT_S16, 1);

    LOGCATE("AVInputEngine::init [m_nbSamples, m_DstFrameDataSze]=[%d, %d]",
            m_nbSamples, m_DstFrameDataSize);

    m_AudioOutBuffer = (uint8_t *) malloc(m_DstFrameDataSize);
    return 0;
}

int AVInputEngine::OpenDecoder(AVInputStream *ist) {
    LOGCATE("AVInputEngine::OpenDecoder")
    int result = 0;
    const AVCodec *codec;
    do {
        // 获取解码器参数
        AVCodecParameters *param = ist->fc->streams[ist->stream_index]->codecpar;

        // 获取解码器
        codec = avcodec_find_decoder(param->codec_id);
        if (codec == nullptr) {
            LOGCATE("AVInputEngine::init avcodec_find_decoder fail.")
            result = -1;
            break;
        }
        LOGCATE("AVInputEngine::init avcodec_find_decoder name:%s", codec->name)

        // 创建解码器上下文
        ist->c = avcodec_alloc_context3(codec);
        result = avcodec_parameters_to_context(ist->c, param);
        if (result < 0) {
            LOGCATE("AVInputEngine::init avcodec_parameters_to_context fail.")
            break;
        }

        AVDictionary *pAVDictionary = nullptr;
        av_dict_set(&pAVDictionary, "buffer_size", "1024000", 0);
        av_dict_set(&pAVDictionary, "stimeout", "20000000", 0);
        av_dict_set(&pAVDictionary, "max_delay", "30000000", 0);
        av_dict_set(&pAVDictionary, "rtsp_transport", "tcp", 0);

        // 打开解码器
        result = avcodec_open2(ist->c, codec, &pAVDictionary);
        if (result < 0) {
            LOGCATE("AVInputEngine::init avcodec_open2 fail.")
            break;
        }

        if (m_EventCallback) {
            m_EventCallback->PostMessage(EVENT_DURATION, m_AVFormatContext->duration / 1000);
        }
    } while (false);
    return result;
}


int AVInputEngine::_unInit() {
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

void AVInputEngine::_decodeLoop() {
//    LOGCATE("AVInputEngine::decodeLoop");
    while (m_Callback->GetPlayerState() == STATE_PAUSE) {
        av_usleep(10 * 1000);
    }
    if (decodeLoopOnce() != 0 || m_Callback->GetPlayerState() == STATE_STOP) {
        disableAutoLoop();
    }
}

int AVInputEngine::decodeLoopOnce() {
//    LOGCATE("AVInputEngine::decodeLoopOnce m_MediaType=%d", m_MediaType)
    if (m_SeekPosition > 0) {
        auto seek_target = static_cast<int64_t>(m_SeekPosition * 1000000);
        int64_t seek_min = INT64_MIN;
        int64_t seek_max = INT64_MAX;
        int ret = avformat_seek_file(m_AVFormatContext, -1,
                                     seek_min, seek_target, seek_max, 0);
        LOGCATE("seek_result:%d", ret)
        if (ret >= 0) {
            avcodec_flush_buffers(m_VideoIst->c);
            avcodec_flush_buffers(m_AudioIst->c);

            LOGCATE("AVInputEngine::decodeLoopOnce success while seeking")
            video_seek_finish = true;
            audio_seek_finish = true;
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

        if (avcodec_send_packet(ist->c, m_Pkt) == AVERROR_EOF) {
            // 解码结束
            result = -1;
            m_Callback->SetPlayerState(STATE_PAUSE); // 暂停整个播放器
            goto __EXIT;
        }

        int frameCount = 0;
        while (avcodec_receive_frame(ist->c, m_Frame) == 0) {
            Frame *frame = nullptr;
            if (ist->c->codec_type == AVMEDIA_TYPE_AUDIO) {
                frame = AudioFrameAvailable();
            }
            if (ist->c->codec_type == AVMEDIA_TYPE_VIDEO) {
                frame = VideoFrameAvailable();
            }
            if (frame) {
                if (audio_seek_finish && frame->type == MEDIA_TYPE_AUDIO) {
                    frame->flag = FLAG_SEEK_FINISH;
                    audio_seek_finish = false;
                    LOGCATE("AVInputEngine VideoSeekSuccess=%ld", frame->pts)
                }
                if (video_seek_finish && frame->type == MEDIA_TYPE_VIDEO) {
                    frame->flag = FLAG_SEEK_FINISH;
                    video_seek_finish = false;
                    LOGCATE("AVInputEngine AudioSeekSuccess=%ld", frame->pts)
                }
                m_Callback->OnDecodeOneFrame(frame);
                frameCount++;
            }
        }
//            LOGCATE("AVInputEngine::decodeLoopOnce frameCount=%d", frameCount);
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
    AVCodecContext *c = m_AudioIst->c;
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
    VideoFrame *frame = nullptr;
    AVFormatContext *oc = m_VideoIst->fc;
    AVCodecContext *c = m_VideoIst->c;
    int index = m_VideoIst->stream_index;

    AVRational timeBase = oc->streams[index]->time_base;
    long dts = (long) ((m_Frame->pkt_dts * av_q2d(timeBase)) * 1000);
    long pts = (long) ((m_Frame->pts * av_q2d(timeBase)) * 1000);

    if (c->pix_fmt == AV_PIX_FMT_YUV420P ||
        c->pix_fmt == AV_PIX_FMT_YUVJ420P) {
        frame = new VideoFrame();

        int yPlaneByteSize = m_Frame->width * m_Frame->height;
        int uvPlaneByteSize = yPlaneByteSize / 2;
        auto *data = (uint8_t *) malloc(yPlaneByteSize + uvPlaneByteSize);

        frame->format = VIDEO_FRAME_FORMAT_I420;
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
            frame->format = VIDEO_FRAME_FORMAT_NV12;
        }
//        LOGCATE("VideoDecoder::onFrameAvailable frame[w,h]=[%d,%d], [dts,pts]=[%ld,%ld], format=%d ,[line0,line1,line2]=[%d,%d,%d]",
//                m_VideoWidth, m_VideoHeight, dts, pts, m_AVCodecContext->pix_fmt,
//                m_AVFrame->linesize[0], m_AVFrame->linesize[1], m_AVFrame->linesize[2])
    }
    return frame;
}

void AVInputEngine::_seekPosition(float timestamp) {
    m_SeekPosition = timestamp;
}

