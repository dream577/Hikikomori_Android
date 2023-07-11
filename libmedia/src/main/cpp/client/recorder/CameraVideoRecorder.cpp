//
// Created by bronyna on 2023/2/25.
//

#include "CameraVideoRecorder.h"

int AddAdtsHeader(char *const header, const int data_length,
                  const int profile, const int sample_rate,
                  const int channels) {
    int sampling_frequency_index = 3; // 默认使用48000hz
    int adtsLen = data_length + ADTS_HEADER_LEN;
    //ADTS不是单纯的data，是hearder+data的，所以加7这个头部hearder的长度，这里7是因为后面protection absent位设为1，不做校验，所以直接加7，如果做校验，可能会是9

    int frequencies_size = sizeof(sampling_frequencies) / sizeof(sampling_frequencies[0]);
    int i = 0;
    for (i = 0; i < frequencies_size; i++)   //查找采样率
    {
        if (sampling_frequencies[i] == sample_rate) {
            sampling_frequency_index = i;
            break;
        }
    }
    if (i >= frequencies_size) {
        printf("unsupport sample_rate:%d\n", sample_rate);
        return -1;
    }
    /* 固定头部信息 */
    header[0] = 0xff;        // syncword:0xfff    高8bits  帧同步标识一个帧的开始，固定为0xFFF
    header[1] = 0xf0;        // syncword:0xfff    低4bits
    header[1] |= (0 << 3);   // MPEG Version  1bit  0 for MPEG-4,1 for MPEG-2
    header[1] |= (0 << 1);   // Layer:0       2bits 固定为’00’
    header[1] |= 1;          // protection absent:1   1bit  标识是否进行误码校验。0表示有CRC校验，1表示没有CRC校验

    // profile:profile   2bits  标识使用哪个级别的AAC, 1: AAC Main  2:AAC LC (Low Complexity) 3. AAC SSR (Scalable Sample Rate)  4.AAC LTP (Long Term Prediction)
    header[2] = (profile) << 6;
    // sampling frequency index  4bits      标识使用的采样率的下标
    header[2] |= (sampling_frequency_index & 0x0f) << 2;
    header[2] |= (0 << 1);                  // private bit:0   1bit    私有位, 编码时设置为0，解码时忽略
    header[2] |= (channels & 0x04) >> 2;    // channel configuration   高1bit   标识声道数

    header[3] = (channels & 0x03) << 6;     // channel configuration   低2bits
    header[3] |= (0 << 5);                  // original：0      1bit   编码时设置为0，解码时忽略
    header[3] |= (0 << 4);                  // home：0          1bit   编码时设置为0，解码时忽略

    /* 可变头部信息 */
    header[3] |= (0 << 3);                           // copyright id bit：0        1bit
    header[3] |= (0 << 2);                           // copyright id start：0      1bit
    header[3] |= ((adtsLen & 0x1800) >> 11);         // frame length：value    高2bits

    header[4] = (uint8_t) ((adtsLen & 0x7f8) >> 3);  // frame length:value     中间8bits
    header[5] = (uint8_t) ((adtsLen & 0x7) << 5);    // frame length:value     低3bits
    header[5] |= 0x1f;                               // buffer fullness:0x7ff  高5bits
    header[6] = 0xfc;                                // buffer fullness:0x7ff  低6bits
    // number_of_raw_data_blocks_in_frame：
    //    表示ADTS帧中有number_of_raw_data_blocks_in_frame + 1个AAC原始帧。
    return 0;
}

CameraVideoRecorder::CameraVideoRecorder() {
    LOGCATE("CameraVideoRecorder::CameraVideoRecorder")
    m_RenderWindow = new GLRenderWindow(this);
    m_VideoRenderQueue = new ThreadSafeQueue(10, MEDIA_TYPE_VIDEO);
    m_RenderWindow->StartRender();

    m_FormatCtx = nullptr;
    m_VideoOst = nullptr;
    m_AudioOst = nullptr;

    m_EnableAudio = false;
    m_EnableVideo = false;
    m_RecordModeExit = true;
}

int CameraVideoRecorder::Init() {
    LOGCATE("CameraVideoRecorder::Init()")
    int result;
    // 申请AVFormatContext，主要是在进行封装格式相关的操作时作为操作上下文的线索
    result = avformat_alloc_output_context2(&m_FormatCtx, nullptr, nullptr, m_FileName);
    if (result < 0) {
        result = avformat_alloc_output_context2(&m_FormatCtx, nullptr, "mp4", m_FileName);
    }
    if (result < 0) {
        LOGCATE("CameraVideoRecorder::Init() avformat_alloc_output_context2 ret=%d", result)
        goto __EXIT;
    }

    if (m_EnableAudio) {
        m_AudioEncoderQueue = new ThreadSafeQueue(15, MEDIA_TYPE_AUDIO);
        m_AudioOst = new AVOutputStream();
        m_AudioOst->oc = m_FormatCtx;
        result = AddStream(m_AudioOst, AV_CODEC_ID_AAC, nullptr);
        if (result >= 0) {
            result = OpenAudio(m_AudioOst);
        }
        if (result < 0) {
            m_EnableAudio = false;
            LOGCATE("CameraVideoRecorder::Init() audio error")
        }
    }

    if (m_EnableVideo) {
        m_VideoEncoderQueue = new ThreadSafeQueue(15, MEDIA_TYPE_VIDEO);
        m_VideoOst = new AVOutputStream();
        m_VideoOst->oc = m_FormatCtx;
        const char *h264_codec_name = "h264";
        result = AddStream(m_VideoOst, AV_CODEC_ID_H264, h264_codec_name);
        if (result >= 0) {
            result = OpenVideo(m_VideoOst);
        }
        if (result < 0) {
            m_EnableVideo = false;
            LOGCATE("CameraVideoRecorder::Init() video error")
        }
    }

    if (!m_EnableVideo && !m_EnableAudio) {
        goto __EXIT;
    }

    av_dump_format(m_FormatCtx, 0, m_FilePath, 1);
    /* open the output file, if needed */
    if (!(m_FormatCtx->flags & AVFMT_NOFILE)) {
        result = avio_open(&m_FormatCtx->pb, m_FilePath, AVIO_FLAG_WRITE);
        if (result < 0) {
            LOGCATE("CameraVideoRecorder::Init() Could not open '%s': %s", m_FilePath,
                    av_err2str(result))
        }
    }
    __EXIT:
    return result;
}

int CameraVideoRecorder::AddStream(AVOutputStream *ost, AVCodecID codec_id,
                                   const char *codec_name) {
    int result = 0;

    // 查找编码器
    ost->c = avcodec_find_encoder(codec_id);
    if (!(ost->c)) {
        result = -1;
        LOGCATE("CameraVideoRecorder::AddStream avcodec_find_encoder:%s error", ost->c->name)
        goto __EXIT;
    }
    LOGCATE("CameraVideoRecorder::AddStream codec_name:%s", ost->c->name)

    //  申请AVStream，AVStream流主要作为存放音频、存放视频、字幕数据流使用
    ost->st = avformat_new_stream(ost->oc, nullptr);
    if (!(ost->st)) {
        result = -1;
        LOGCATE("CameraVideoRecorder::AddStream avformat_new_stream error")
        goto __EXIT;
    }
    ost->st->id = (int) (ost->oc->nb_streams) - 1;

    ost->cc = avcodec_alloc_context3(ost->c);
    if ((!ost->cc)) {
        result = -1;
        LOGCATE("CameraVideoRecorder::AddStream avcodec_alloc_context3 error")
        goto __EXIT;
    }

    switch (ost->c->type) {
        case AVMEDIA_TYPE_AUDIO:
            ost->cc->sample_fmt = ost->c->sample_fmts ? ost->c->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
            ost->cc->bit_rate = 96000;
            ost->cc->sample_rate = m_SampleRate;
            ost->cc->channel_layout = m_ChannelLayout;
            ost->cc->channels = av_get_channel_layout_nb_channels(m_ChannelLayout);
            ost->st->time_base = (AVRational) {1, m_SampleRate};
            LOGCATE("AudioCodec[sample_fmt=%d, bit_rate=%d, sample_rate=%d, channel_layout=%d, channels=%d]",
                    ost->cc->sample_fmt, ost->cc->bit_rate, ost->cc->sample_rate,
                    ost->cc->channel_layout, ost->cc->channels)
            break;
        case AVMEDIA_TYPE_VIDEO:
            ost->cc->codec_id = codec_id;
            ost->cc->bit_rate = 1920 * 1080 * fps * 8 * 3 / 2 / 1024;
            ost->cc->width = m_ImageWidth;
            ost->cc->height = m_ImageHeight;
            ost->st->time_base = (AVRational) {1, fps};
            ost->cc->time_base = ost->st->time_base;
            ost->cc->framerate = (AVRational) {fps, 1};
            ost->cc->gop_size = 12;
            ost->cc->pix_fmt = AV_PIX_FMT_NV12;
//            ost->cc->thread_count = 2;
//            ost->cc->profile = 1;
//            ost->cc->level = 3;
//            if (codec_id == AV_CODEC_ID_H264) {
//                av_opt_set(ost->cc->priv_data, "preset", "slow", 0);
//            }
            break;
        default:
            break;
    }
    if (ost->oc->oformat->flags & AVFMT_GLOBALHEADER) {
        ost->cc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    __EXIT:
    return result;
}

int CameraVideoRecorder::OpenAudio(AVOutputStream *ost) {
    LOGCATE("CameraVideoRecorder::OpenAudio")
    int result;
    int nb_samples;

    // 打开编码器
    result = avcodec_open2(ost->cc, ost->c, nullptr);
    if (result < 0) {
        LOGCATE("CameraVideoRecorder::OpenAudio avcodec_open2 error=%d", result)
        goto __EXIT;
    }

    if (ost->c->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE) {
        nb_samples = 10000;
    } else {
        nb_samples = ost->cc->frame_size;
    }

    ost->df = av_frame_alloc();
    if (!(ost->df)) {
        LOGCATE("CameraVideoRecorder::OpenAudio Error allocating ost->df")
        result = -1;
        goto __EXIT;
    }
    ost->df->format = ost->cc->sample_fmt;
    ost->df->channel_layout = ost->cc->channel_layout;
    ost->df->sample_rate = ost->cc->sample_rate;
    ost->df->nb_samples = nb_samples;
    if (nb_samples) {
        result = av_frame_get_buffer(ost->df, 0);
        if (result < 0) {
            LOGCATE("CameraVideoRecorder::OpenAudio Error allocating an audio buffer")
            goto __EXIT;
        }
    }
    ost->sf = av_frame_alloc();
    if (!(ost->sf)) {
        LOGCATE("CameraVideoRecorder::OpenAudio Error allocating ost->sf")
        result = -1;
        goto __EXIT;
    }

    result = avcodec_parameters_from_context(ost->st->codecpar, ost->cc);
    if (result < 0) {
        LOGCATE("CameraVideoRecorder::OpenAudio Could not copy the stream parameters");
        goto __EXIT;
    }

    /* create resampler context */
    ost->sr = swr_alloc();
    if (!(ost->sr)) {
        LOGCATE("CameraVideoRecorder::OpenAudio Could not allocate resampler context")
        result = -1;
        goto __EXIT;
    }

    /* set options */
    av_opt_set_int(ost->sr, "in_channel_count", ost->cc->channels, 0);
    av_opt_set_int(ost->sr, "in_sample_rate", ost->cc->sample_rate, 0);
    av_opt_set_sample_fmt(ost->sr, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    av_opt_set_int(ost->sr, "out_channel_count", ost->cc->channels, 0);
    av_opt_set_int(ost->sr, "out_sample_rate", ost->cc->sample_rate, 0);
    av_opt_set_sample_fmt(ost->sr, "out_sample_fmt", ost->cc->sample_fmt, 0);

    result = swr_init(ost->sr);
    if (result < 0) {
        LOGCATE("CameraVideoRecorder::OpenAudio swr_init error=%d", result)
    }
    __EXIT:
    return result;
}

int CameraVideoRecorder::EncodeAudioFrame(AVOutputStream *ost, AudioFrame *audio_frame) {
    int result;
    int64_t dst_nb_samples;
    AVFrame *frame = ost->sf;
    AVPacket *pkt = av_packet_alloc();

    if (audio_frame) {
        frame->data[0] = audio_frame->data;
        frame->nb_samples = audio_frame->dataSize / 4;
        frame->pts = ost->nextPts;
        ost->nextPts += frame->nb_samples;
    } else {
        frame = nullptr;
    }

    if (frame) {
        dst_nb_samples = av_rescale_rnd(
                swr_get_delay(ost->sr, ost->cc->sample_rate) + frame->nb_samples,
                ost->cc->sample_rate, ost->cc->sample_rate, AV_ROUND_UP);
        av_assert0(dst_nb_samples == frame->nb_samples);

        result = av_frame_make_writable(ost->df);
        if (result < 0) {
            LOGCATE("CameraVideoRecorder::EncodeAudioFrame Error while av_frame_make_writable")
            goto __EXIT;
        }

        result = swr_convert(ost->sr, ost->df->data, (int) dst_nb_samples,
                             (const uint8_t **) frame->data, frame->nb_samples);
        if (result < 0) {
            LOGCATE("CameraVideoRecorder::EncodeAudioFrame Error while converting")
            goto __EXIT;
        }

        frame = ost->df;
        frame->pts = av_rescale_q(ost->sampleCount, (AVRational) {1, ost->cc->sample_rate},
                                  ost->cc->time_base);
        ost->sampleCount += dst_nb_samples;
    }

    // 将数据送入编码器进行编码
    result = avcodec_send_frame(ost->cc, frame);
    if (result == AVERROR_EOF) {
        goto __EXIT;
    } else if (result < 0) {
        LOGCATE("CameraVideoRecorder::EncodeAudioFrame audio avcodec_send_frame fail. ret=%s",
                av_err2str(result))
        result = 0;
        goto __EXIT;
    }

    while (!result) {
        result = avcodec_receive_packet(ost->cc, pkt);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            result = 0;
            goto __EXIT;
        } else if (result < 0) {
            LOGCATE("CameraVideoRecorder::EncodeAudioFrame audio avcodec_receive_packet fail. ret=%d",
                    result)
            result = 0;
            goto __EXIT;
        }

        // 如果音频编码格式为AAC, 为编码出来的数据添加ADTS头部
        if (ost->cc->codec_id == AV_CODEC_ID_AAC) {
            char adts_header[7];
            AddAdtsHeader(adts_header, pkt->size, 2, ost->cc->sample_rate,
                          ost->cc->channels);
            av_grow_packet(pkt, 7);
            for (int i = pkt->size - 1; i >= 7; i--) {
                pkt->data[i] = pkt->data[i - 7];
            }
            memcpy(pkt->data, adts_header, 7);
        }
        LOGCATE("CameraVideoRecorder::EncodeAudioFrame pkt pts=%ld, size=%d", pkt->pts, pkt->size)

        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(pkt, ost->cc->time_base, ost->st->time_base);
        pkt->stream_index = ost->st->index;

        // 将编码数据写入文件
        AVRational *time_base = &ost->oc->streams[pkt->stream_index]->time_base;
        LOGCATE("CameraVideoRecorder::EncodeAudioFrame pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d",
                av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
                av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
                av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
                pkt->stream_index)
        av_interleaved_write_frame(ost->oc, pkt);
    }

    __EXIT:
    av_packet_free(&pkt);
    delete audio_frame;
    return result;
}

int CameraVideoRecorder::OpenVideo(AVOutputStream *ost) {
    int result = 0;
    LOGCATE("CameraVideoRecorder::OpenVideo")
    result = avcodec_open2(ost->cc, ost->c, nullptr);
    if (result < 0) {
        LOGCATE("CameraVideoRecorder::OpenVideo avcodec_open2 Could not open video codec: %s, error_code=%d",
                av_err2str(result), result)
        goto __EXIT;
    }

    ost->df = av_frame_alloc();
    if (!(ost->df)) {
        LOGCATE("CameraVideoRecorder::OpenVideo Error allocating ost->df")
        result = -1;
        goto __EXIT;
    }
    ost->df->format = ost->cc->pix_fmt;
    ost->df->width = ost->cc->width;
    ost->df->height = ost->cc->height;
    result = av_frame_get_buffer(ost->df, 1);
    if (result < 0) {
        LOGCATE("CameraVideoRecorder::OpenVideo Could not allocate frame data.");
        goto __EXIT;
    }
    ost->sf = av_frame_alloc();
    if (!(ost->sf)) {
        LOGCATE("CameraVideoRecorder::OpenVideo Error allocating ost->sf")
        result = -1;
        goto __EXIT;
    }

    result = avcodec_parameters_from_context(ost->st->codecpar, ost->cc);
    if (result < 0) {
        LOGCATE("CameraVideoRecorder::OpenAudio Could not copy the stream parameters");
    }

    __EXIT:
    return result;
}

int CameraVideoRecorder::EncodeVideoFrame(AVOutputStream *ost, VideoFrame *video_frame) {
    int result = 0;
    AVFrame *frame = ost->sf;
    AVPacket *pkt = av_packet_alloc();
    if (video_frame) {
        frame->data[0] = video_frame->yuvBuffer[0];
        frame->data[1] = video_frame->yuvBuffer[1];
        frame->data[2] = video_frame->yuvBuffer[2];
        frame->linesize[0] = video_frame->planeSize[0];
        frame->linesize[1] = video_frame->planeSize[1];
        frame->linesize[2] = video_frame->planeSize[2];
        frame->format = AV_PIX_FMT_YUV420P;
        frame->width = video_frame->width;
        frame->height = video_frame->height;
    } else {
        frame = nullptr;
    }

    if (frame) {
        if (av_frame_make_writable(ost->df) < 0) {
            result = 1;
            goto __EXIT;
        }

        // 转为NV21
        if (frame->format != AV_PIX_FMT_NV12) {
            if (!ost->ss) {
                ost->ss = sws_getContext(ost->cc->width, ost->cc->height, AV_PIX_FMT_YUV420P,
                                         ost->cc->width, ost->cc->height, ost->cc->pix_fmt,
                                         SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
                if (!ost->ss) {
                    LOGCATE("MediaRecorder::EncodeVideoFrame Could not initialize the conversion context\n");
                    result = 1;
                    goto __EXIT;
                }
            }
            sws_scale(ost->ss, (const uint8_t *const *) frame->data, frame->linesize,
                      0, ost->cc->height, ost->df->data, ost->df->linesize);
        }
        ost->df->pts = ost->nextPts++;
        frame = ost->df;
    }

    result = avcodec_send_frame(ost->cc, frame);
    if (result == AVERROR_EOF) {
        result = 1;
        goto __EXIT;
    } else if (result < 0) {
        LOGCATE("CameraVideoRecorder::EncodeVideoFrame video avcodec_send_frame fail. ret=%s",
                av_err2str(result));
        result = 0;
        goto __EXIT;
    }

    while (!result) {
        result = avcodec_receive_packet(ost->cc, pkt);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            result = 0;
            goto __EXIT;
        } else if (result < 0) {
            LOGCATE("CameraVideoRecorder::EncodeVideoFrame video avcodec_receive_packet fail. ret=%s",
                    av_err2str(result))
            result = 0;
            goto __EXIT;
        }
        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(pkt, ost->cc->time_base, ost->st->time_base);
        pkt->stream_index = ost->st->index;

        // 将编码数据写入文件
        AVRational *time_base = &ost->oc->streams[pkt->stream_index]->time_base;
        LOGCATE("CameraVideoRecorder::EncodeVideoFrame pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d",
                av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
                av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
                av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
                pkt->stream_index)
        av_interleaved_write_frame(ost->oc, pkt);
    }
    __EXIT:
    av_packet_free(&pkt);
    return result;
}

int CameraVideoRecorder::UnInit() {
    int result = 0;
    LOGCATE("CameraVideoRecorder::UnInit()")
    if (m_VideoRenderQueue) {
        m_VideoRenderQueue->abort();
    }
    if (m_RenderWindow) {
        m_RenderWindow->Destroy();
        delete m_RenderWindow;
        m_RenderWindow = nullptr;
    }
    if (m_VideoRenderQueue) {
        delete m_VideoRenderQueue;
        m_VideoRenderQueue = nullptr;
    }
    return result;
}

CameraVideoRecorder::~CameraVideoRecorder() {
    UnInit();
}

void CameraVideoRecorder::StartRecord() {
    LOGCATE("CameraVideoRecorder::StartRecord()")
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&worker, &attr, StartRecordLoop, this);
}

void CameraVideoRecorder::StopRecord() {
    LOGCATE("CameraVideoRecorder::StopRecord() start")
    // 设置停止标志位并等待直到录制真正结束
    m_IsVideoRecording = false;
    m_IsAudioRecording = false;
    if (m_AudioEncoderQueue) {
        m_AudioEncoderQueue->abort();
    }
    if (m_VideoEncoderQueue) {
        m_VideoEncoderQueue->abort();
    }
    while (!m_RecordModeExit) {
        av_usleep(1000 * 10);
    }
    LOGCATE("CameraVideoRecorder::StopRecord() end")
}

void CameraVideoRecorder::RealStopRecord() {
    if (m_AudioEncoderQueue) {
        delete m_AudioEncoderQueue;
        m_AudioEncoderQueue = nullptr;
    }
    if (m_VideoEncoderQueue) {
        delete m_VideoEncoderQueue;
        m_VideoEncoderQueue = nullptr;
    }
    if (m_AudioOst) {
        delete m_AudioOst;
        m_AudioOst = nullptr;
    }
    if (m_VideoOst) {
        delete m_VideoOst;
        m_VideoOst = nullptr;
    }
    if (m_FormatCtx) {
        avformat_free_context(m_FormatCtx);
        m_FormatCtx = nullptr;
    }
    // 重置标志位
    m_EnableVideo = false;
    m_EnableAudio = false;
    LOGCATE("CameraVideoRecorder::RealStopRecord()")
}

void CameraVideoRecorder::OnDrawVideoFrame(uint8_t *data, int width, int height, int format,
                                           long timestamp) {
//    LOGCATE("CameraVideoRecorder::OnDrawVideoFrame")
    VideoFrame *frame = nullptr;
    switch (format) {
        case IMAGE_FORMAT_RGBA:
            frame = new VideoFrame();

            frame->format = IMAGE_FORMAT_RGBA;
            frame->width = width;
            frame->height = height;
            frame->yuvBuffer[0] = data;
            frame->planeSize[0] = width * 4;
            frame->pts = timestamp;
            break;
        case IMAGE_FORMAT_I420:
            frame = new VideoFrame();
            int yPlaneByteSize = width * height;
            int uvPlaneByteSize = yPlaneByteSize / 2;

            frame->format = IMAGE_FORMAT_I420;
            frame->width = width;
            frame->height = height;
            frame->yuvBuffer[0] = data;
            frame->yuvBuffer[1] = data + yPlaneByteSize;
            frame->yuvBuffer[2] = data + yPlaneByteSize + uvPlaneByteSize / 2;
            frame->planeSize[0] = width;
            frame->planeSize[1] = width / 2;
            frame->planeSize[2] = width / 2;
            frame->pts = timestamp;
            break;
    }

    frame->type = MEDIA_TYPE_VIDEO;
    int ret = m_VideoRenderQueue->offer(frame);
    if (ret < 0) {
        delete frame;
    }
}

void CameraVideoRecorder::InputAudioData(uint8_t *data, int size, long timestamp,
                                         int sample_rate, int sample_format, int channel_layout) {
    if (!m_IsAudioRecording) {
        delete data;
        return;
    }
    auto *frame = new AudioFrame();
    frame->data = data;
    frame->dataSize = size;
    frame->sampleRate = sample_rate;
    frame->sampleFormat = sample_format;
    frame->channelLayout = channel_layout;
    frame->pts = timestamp;
    int ret = m_AudioEncoderQueue->offer(frame);
    if (ret < 0) {
        delete frame;
    }
}

Frame *CameraVideoRecorder::GetOneFrame(int type) {
//    LOGCATE("CameraVideoRecorder::GetOneFrame")
    Frame *frame;
    if (type == MEDIA_TYPE_VIDEO) {
        frame = m_VideoRenderQueue->poll();
    } else {
        frame = m_AudioEncoderQueue->poll();
    }
    return frame;
}

void CameraVideoRecorder::FrameRendFinish(Frame *frame) {
//    LOGCATE("CameraVideoRecorder::FrameRendFinish")
    if (frame) {
        if (frame->type == MEDIA_TYPE_VIDEO) {
            if (m_IsVideoRecording) {
                m_VideoEncoderQueue->offer(frame);
            } else {
                delete frame;
            }
        } else {
            delete frame;
        }
    }
}

void *CameraVideoRecorder::StartRecordLoop(void *recorder) {
    auto *mRecorder = (CameraVideoRecorder *) recorder;
    int result;
    mRecorder->m_RecordModeExit = false;

    // 根据是否启用音频录制、视频录制来设置是否正在录制的标志位
    mRecorder->m_IsVideoRecording = mRecorder->m_EnableVideo;
    mRecorder->m_IsAudioRecording = mRecorder->m_EnableAudio;
    LOGCATE("CameraVideoRecorder::StartRecordLoop  %d, %d", mRecorder->m_IsAudioRecording, mRecorder->m_IsVideoRecording)

    if (!mRecorder->m_IsVideoRecording && !mRecorder->m_IsAudioRecording) {
        goto __EXIT;
    }

    // 写文件头
    result = avformat_write_header(mRecorder->m_FormatCtx, nullptr);
    if (result < 0) {
        LOGCATE("CameraVideoRecorder::StartRecordLoop Error occurred when opening output file: %s",
                av_err2str(result))
        goto __EXIT;
    }

    AudioFrame *audio_frame;
    VideoFrame *video_frame;
    // 音频或者视频录制未结束
    while (mRecorder->m_IsAudioRecording || mRecorder->m_IsVideoRecording) {
        if (mRecorder->m_IsAudioRecording && mRecorder->m_IsVideoRecording) {
            double videoTimestamp = (double) mRecorder->m_VideoOst->nextPts *
                                    av_q2d(mRecorder->m_VideoOst->cc->time_base);
            double audioTimestamp = (double) mRecorder->m_AudioOst->nextPts *
                                    av_q2d(mRecorder->m_AudioOst->cc->time_base);
            if (videoTimestamp < audioTimestamp) {
                video_frame = (VideoFrame *) mRecorder->m_VideoEncoderQueue->poll();
                mRecorder->EncodeVideoFrame(mRecorder->m_VideoOst, video_frame);
            } else {
                audio_frame = (AudioFrame *) mRecorder->m_AudioEncoderQueue->poll();
                mRecorder->EncodeAudioFrame(mRecorder->m_AudioOst, audio_frame);
            }
        } else if (mRecorder->m_IsAudioRecording) {
            audio_frame = (AudioFrame *) mRecorder->m_AudioEncoderQueue->poll();
            mRecorder->EncodeAudioFrame(mRecorder->m_AudioOst, audio_frame);
        } else {
            video_frame = (VideoFrame *) mRecorder->m_VideoEncoderQueue->poll();
            mRecorder->EncodeVideoFrame(mRecorder->m_VideoOst, video_frame);
        }
    }

    // 写文件尾
    av_write_trailer(mRecorder->m_FormatCtx);
    LOGCATE("CameraVideoRecorder::StartRecordLoop record finish, save as: %s",
            mRecorder->m_FilePath)
    avio_close(mRecorder->m_FormatCtx->pb);

    __EXIT:
    mRecorder->RealStopRecord();
    mRecorder->m_RecordModeExit = true;
    return nullptr;
}
