//
// Created by bronyna on 2023/2/26.
//

#include "AudioEncoder.h"
#include "LogUtil.h"

void
AudioEncoder::Init(int bitRate, int channels, int sampleRate, int bitPerSample,
                   int sampleFormat, const char *aacFilePath) {
    mBitRate = bitRate;
    mChannels = channels;
    mSampleRate = sampleRate;
    mBitPerSample = bitPerSample;
    mSampleFormat = sampleFormat;
    m_AudioFrameQueue = new ThreadSafeQueue(100, MEDIA_TYPE_AUDIO);
    strcpy(mFilePath, aacFilePath);
}

/* check that a given sample format is supported by the encoder */
static int check_sample_fmt(const AVCodec *codec, enum AVSampleFormat sample_fmt) {
    const enum AVSampleFormat *p = codec->sample_fmts;

    while (*p != AV_SAMPLE_FMT_NONE) {
        if (*p == sample_fmt)
            return 1;
        p++;
    }
    return 0;
}

/* just pick the highest supported samplerate */
static int select_sample_rate(const AVCodec *codec) {
    const int *p;
    int best_samplerate = 0;

    if (!codec->supported_samplerates)
        return 44100;

    p = codec->supported_samplerates;
    while (*p) {
        if (!best_samplerate || abs(44100 - *p) < abs(44100 - best_samplerate))
            best_samplerate = *p;
        p++;
    }
    return best_samplerate;
}

static int select_channel_layout(const AVCodec *codec, AVChannelLayout *dst) {
    const AVChannelLayout *p, *best_ch_layout;
    int best_nb_channels = 0;

//    if (!codec->ch_layouts)
//        return av_channel_layout_copy(dst, &(AVChannelLayout) AV_CHANNEL_LAYOUT_STEREO);

    p = codec->ch_layouts;
    while (p->nb_channels) {
        int nb_channels = p->nb_channels;

        if (nb_channels > best_nb_channels) {
            best_ch_layout = p;
            best_nb_channels = nb_channels;
        }
        p++;
    }
    return av_channel_layout_copy(dst, best_ch_layout);
}

int AudioEncoder::StartEncode() {
    int result = -1;
    do {
        const AVOutputFormat *oformat = av_guess_format(nullptr, "mp3", "hello");

        // 1. 申请AVFormatContext，主要是在进行封装格式相关的操作时作为操作上下文的线索
        result = avformat_alloc_output_context2(&m_AVFormatContext, oformat, "mp4", "hello");
        if (result < 0) {
            LOGCATE("FFBaseEncoder::Init avformat_alloc_output_context2 ret=%d", result)
            break;
        }

        // 2. 查找编码器，并根据视频的编码参数对AVCodecContext的参数进行设置
        m_AVCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
        if (m_AVCodec == nullptr) {
            result = -1;
            LOGCATE("FFBaseEncoder::Init avcodec_find_encoder ret=%d", result)
            break;
        }
        m_AVCodecContext = avcodec_alloc_context3(m_AVCodec);
        m_AVCodecContext->codec_id = AV_CODEC_ID_AAC;
        m_AVCodecContext->codec_type = AVMEDIA_TYPE_AUDIO;
        m_AVCodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
        m_AVCodecContext->sample_rate = 44100;
        m_AVCodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
        m_AVCodecContext->channels = av_get_channel_layout_nb_channels(
                m_AVCodecContext->channel_layout);
        m_AVCodecContext->bit_rate = 96000;
        m_AVCodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

        // 3. 申请AVStream，AVStream流主要作为存放音频、存放视频、字幕数据流使用
        m_AVStream = avformat_new_stream(m_AVFormatContext, m_AVCodec);
        if (m_AVStream == nullptr) {
            result = -1;
            LOGCATE("FFBaseEncoder::Init avformat_new_stream ret=%d", result)
            break;
        }
        m_AVStream->id = 0;

        result = avcodec_open2(m_AVCodecContext, m_AVCodec, nullptr);
        if (result < 0) {
            LOGCATE("FFBaseEncoder::Init avcodec_open2 ret=%d", result)
            break;
        }


        result = avio_open(&m_AVFormatContext->pb, mFilePath, AVIO_FLAG_WRITE);
        if (result < 0) {
            LOGCATE("FFBaseEncoder::Init avio_open ret=%d", result)
            break;
        }


        av_dump_format(m_AVFormatContext, 0, mFilePath, 1);

        m_SrcFrame = av_frame_alloc();
        m_SrcFrame->nb_samples = m_AVCodecContext->frame_size;
        m_SrcFrame->format = m_AVCodecContext->sample_fmt;

        mFrameBufferSize = av_samples_get_buffer_size(nullptr, m_AVCodecContext->channels,
                                                      m_AVCodecContext->frame_size,
                                                      m_AVCodecContext->sample_fmt, 1);

        LOGCATE("FFBaseEncoder::Init av_samples_get_buffer_size m_frameBufferSize=%d, nb_samples=%d",
                mFrameBufferSize, m_SrcFrame->nb_samples);
        m_FrameBuffer = (uint8_t *) malloc(mFrameBufferSize);
        avcodec_fill_audio_frame(m_SrcFrame, m_AVCodecContext->channels,
                                 m_AVCodecContext->sample_fmt,
                                 m_FrameBuffer, mFrameBufferSize, 1);

        //写文件头
        result = avformat_write_header(m_AVFormatContext, nullptr);
        if (result != AVSTREAM_INIT_IN_INIT_OUTPUT) {
            LOGCATE("FFBaseEncoder::Init avformat_write_header ret=%d", result)
            break;
        }
        isWriteHeadSuccess = true;
        av_new_packet(m_AVPacket, mFrameBufferSize);

        // 音频转码器
        m_SwrContext = swr_alloc();
//        av_opt_set_channel_layout(m_SwrContext, "in_channel_layout", , 0);
        av_opt_set_channel_layout(m_SwrContext, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
        av_opt_set_int(m_SwrContext, "in_sample_rate", mSampleRate, 0);
        av_opt_set_int(m_SwrContext, "out_sample_rate", 44100, 0);
        av_opt_set_int(m_SwrContext, "in_sample_fmt", mSampleFormat, 0);
        av_opt_set_int(m_SwrContext, "out_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
        swr_init(m_SwrContext);
    } while (false);
    return 0;
}

void AudioEncoder::EncodeFrame() {

//    avcodec_send_frame();
//    avcodec_receive_packet()
//    av_write_frame(m_AVFormatContext, m_AVPacket);
}

void AudioEncoder::UnInit() {

    if (m_SwrContext) {
        swr_free(&m_SwrContext);
        m_SwrContext = nullptr;
    }
    if (m_AVPacket != nullptr) {
        av_packet_free(&m_AVPacket);
        m_AVPacket = nullptr;
    }
    if (m_FrameBuffer) {
        delete m_FrameBuffer;
        m_FrameBuffer = nullptr;
    }
    if (m_SrcFrame) {
        av_frame_free(&m_SrcFrame);
        m_SrcFrame = nullptr;
    }
    if (m_SwrFrame) {
        av_frame_free(&m_SrcFrame);
        m_SrcFrame = nullptr;
    }
    if (m_AVCodecContext) {
        avcodec_close(m_AVCodecContext);
        avcodec_free_context(&m_AVCodecContext);
        m_AVCodec = nullptr;
        m_AVCodecContext = nullptr;
    }
    if (m_AVFormatContext != nullptr) {
        avio_close(m_AVFormatContext->pb);
        m_AVFormatContext = nullptr;
    }
}
