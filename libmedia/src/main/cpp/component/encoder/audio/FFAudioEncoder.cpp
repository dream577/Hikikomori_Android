//
// Created by 郝留凯 on 2023/10/21.
//

#include "FFAudioEncoder.h"

FFAudioEncoder::FFAudioEncoder(AVCodecID encoderId, int audioSampleRate,
                               int audioChannelLayout, int audioBitRate) : FFBaseEncoder(
        encoderId) {
    this->m_EncoderId = encoderId;
    this->m_AudioSampleRate = audioSampleRate;
    this->m_AudioChannelLayout = audioChannelLayout;
    this->m_AudioBitRate = audioBitRate;
}

int FFAudioEncoder::OpenCodec(AVCodecParameters *parameters) {
    LOGCATE("FFAudioEncoder::OpenCodec start: %lld", GetSysCurrentTime())
    int result = 0;
    int nb_samples;

    // 查找编码器
    m_Codec = avcodec_find_encoder(m_EncoderId);
    if (!m_Codec) {
        result = -1;
        LOGCATE("FFAudioEncoder::OpenCodec avcodec_find_encoder:%s error", m_Codec->name)
        goto __EXIT;
    }
    LOGCATE("FFAudioEncoder::OpenCodec avcodec_find_encoder:%s", m_Codec->name)

    // 分配编码器上下文
    m_CodecCtx = avcodec_alloc_context3(m_Codec);
    if (!m_CodecCtx) {
        result = -1;
        LOGCATE("FFAudioEncoder::OpenCodec avcodec_alloc_context3 error")
        goto __EXIT;
    }

    // 配置编码器参数
    m_CodecCtx->sample_fmt = m_Codec->sample_fmts ? m_Codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    m_CodecCtx->bit_rate = m_AudioBitRate;
    m_CodecCtx->sample_rate = m_AudioSampleRate;
    m_CodecCtx->channel_layout = m_AudioChannelLayout;
    m_CodecCtx->channels = av_get_channel_layout_nb_channels(m_AudioChannelLayout);

    // 打开编码器
    result = avcodec_open2(m_CodecCtx, m_Codec, nullptr);
    if (result < 0) {
        LOGCATE("FFAudioEncoder::OpenCodec avcodec_open2 error")
        goto __EXIT;
    }

    result = avcodec_parameters_from_context(parameters, m_CodecCtx);
    if (result < 0) {
        LOGCATE("FFAudioEncoder::OpenCodec avcodec_parameters_from_context error")
        goto __EXIT;
    }

    // 存放原始音频
    m_SrcFrame = av_frame_alloc();
    if (!m_SrcFrame) {
        LOGCATE("FFAudioEncoder::OpenCodec av_frame_alloc error")
        goto __EXIT;
    }

    if (m_Codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE) {
        nb_samples = 10000;
    } else {
        nb_samples = m_CodecCtx->frame_size;
    }
    // 存放重采样之后的音频
    m_DstFrame = av_frame_alloc();
    if (!m_DstFrame) {
        LOGCATE("FFAudioEncoder::OpenCodec av_frame_alloc error")
        goto __EXIT;
    }
    m_DstFrame->format = m_CodecCtx->sample_fmt;
    m_DstFrame->channel_layout = m_CodecCtx->channel_layout;
    m_DstFrame->sample_rate = m_CodecCtx->sample_rate;
    m_DstFrame->nb_samples = nb_samples;
    if (nb_samples) {
        result = av_frame_get_buffer(m_DstFrame, 0);
        if (result < 0) {
            LOGCATE("FFAudioEncoder::OpenCodec av_frame_get_buffer error")
            goto __EXIT;
        }
    }

    m_CodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    __EXIT:
    return result;
}

int FFAudioEncoder::EncodeFrame() {
    int result = 0;
    int64_t dstSamples;
    AVPacket *pkt = av_packet_alloc();
    auto frame = m_EncoderQueue->poll();
    if (!frame) {
        result = -1;
        goto __EXIT;
    }

    m_SrcFrame->data[0] = frame->plane[0];
    m_SrcFrame->nb_samples = frame->planeSize[0] / 4;
    m_SrcFrame->pts = m_NextPts;
    m_NextPts += m_SrcFrame->nb_samples;

    if (!m_SwrCtx) {
        m_SwrCtx = swr_alloc();
        if (!m_SwrCtx) {
            LOGCATE("FFAudioEncoder::EncodeFrame swr_alloc error")
            result = -1;
            goto __EXIT;
        }
        av_opt_set_int(m_SwrCtx, "in_channel_count", m_CodecCtx->channels, 0);
        av_opt_set_int(m_SwrCtx, "in_sample_rate", m_CodecCtx->sample_rate, 0);
        av_opt_set_sample_fmt(m_SwrCtx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
        av_opt_set_int(m_SwrCtx, "out_channel_count", m_CodecCtx->channels, 0);
        av_opt_set_int(m_SwrCtx, "out_sample_rate", m_CodecCtx->sample_rate, 0);
        av_opt_set_sample_fmt(m_SwrCtx, "out_sample_fmt", m_CodecCtx->sample_fmt, 0);

        result = swr_init(m_SwrCtx);
        if (result < 0) {
            LOGCATE("FFAudioEncoder::EncodeFrame swr_init error")
            goto __EXIT;
        }
    }

    result = av_frame_make_writable(m_DstFrame);
    if (result < 0) {
        LOGCATE("FFAudioEncoder::EncodeFrame av_frame_make_writable error")
        goto __EXIT;
    }

    dstSamples = av_rescale_rnd(
            swr_get_delay(m_SwrCtx, m_CodecCtx->sample_rate) + m_SrcFrame->nb_samples,
            m_CodecCtx->sample_rate, m_CodecCtx->sample_rate, AV_ROUND_UP);
    result = swr_convert(m_SwrCtx, m_DstFrame->data, dstSamples,
                         (const uint8_t **) m_SrcFrame->data, m_SrcFrame->nb_samples);
    if (result < 0) {
        LOGCATE("FFAudioEncoder::EncodeFrame swr_convert error")
        goto __EXIT;
    }

    m_DstFrame->pts = av_rescale_q(m_SampleCount, AVRational{1, m_CodecCtx->sample_rate},
                                   m_CodecCtx->time_base);
    m_SampleCount += dstSamples;

    // 编码输入
    result = avcodec_send_frame(m_CodecCtx, m_DstFrame);
    if (result == AVERROR_EOF) {
        result = 1;
        goto __EXIT;
    } else if (result < 0) {
        LOGCATE("FFAudioEncoder::EncodeFrame avcodec_send_frame error. ret=%s", av_err2str(result))
        result = 0;
        goto __EXIT;
    }

    while (!result) {
        // 编码输出
        result = avcodec_receive_packet(m_CodecCtx, pkt);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            result = 0;
            goto __EXIT;
        } else if (result < 0) {
            LOGCATE("FFAudioEncoder::EncodeFrame avcodec_receive_packet error. ret=%s",
                    av_err2str(result))
            result = 0;
            goto __EXIT;
        }

        // 如果音频编码格式为AAC, 为编码出来的数据添加ADTS头部
        if (m_Codec->id == AV_CODEC_ID_AAC) {
            char adts_header[7];
            add_adts_header(adts_header, pkt->size, 2, m_CodecCtx->sample_rate,
                            m_CodecCtx->ch_layout.nb_channels);
            av_grow_packet(pkt, 7);
            for (int i = pkt->size - 1; i >= 7; i--) {
                pkt->data[i] = pkt->data[i - 7];
            }
            memcpy(pkt->data, adts_header, 7);
        }

        if (callback) {
            callback->OnFrameEncoded(pkt, AVMEDIA_TYPE_AUDIO);
        }
    }

    __EXIT:
    av_packet_free(&pkt);
    return result;
}

int FFAudioEncoder::add_adts_header(char *const header, const int data_length,
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

FFAudioEncoder::~FFAudioEncoder() {
    if (m_SwrCtx) {
        swr_free(&m_SwrCtx);
        m_SwrCtx = nullptr;
    }
    LOGCATE("FFAudioEncoder::~FFAudioEncoder()")
}
