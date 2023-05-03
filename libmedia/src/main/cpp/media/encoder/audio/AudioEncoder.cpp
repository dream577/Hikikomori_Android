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

int AudioEncoder::StartEncode() {
    int result = -1;
    do {
        av_register_all();

        result = avformat_alloc_output_context2(&mAVFormatContext, nullptr, nullptr, mFilePath);
        if (result < 0) {
            LOGCATE("FFBaseEncoder::Init avformat_alloc_output_context2 ret=%d", result)
            break;
        }

        result = avio_open(&mAVFormatContext->pb, mFilePath, AVIO_FLAG_WRITE);
        if (result < 0) {
            LOGCATE("FFBaseEncoder::Init avio_open ret=%d", result)
            break;
        }

        mAVStream = avformat_new_stream(mAVFormatContext, nullptr);
        if (mAVStream == nullptr) {
            result = -1;
            LOGCATE("FFBaseEncoder::Init avformat_new_stream ret=%d", result)
            break;
        }

        AVOutputFormat *avOutputFormat = mAVFormatContext->oformat;
        mAVCodec = avcodec_find_encoder(avOutputFormat->audio_codec);
        if (mAVCodec == nullptr) {
            result = -1;
            LOGCATE("FFBaseEncoder::Init avcodec_find_encoder ret=%d", result)
            break;
        }

        mAVCodecContext = mAVStream->codec;
        mAVCodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
        mAVCodecContext->codec_id = AV_CODEC_ID_AAC;
        mAVCodecContext->codec_type = AVMEDIA_TYPE_AUDIO;
        mAVCodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;//float, planar, 4 字节
        mAVCodecContext->sample_rate = DEFAULT_SAMPLE_RATE;
        mAVCodecContext->channel_layout = DEFAULT_CHANNEL_LAYOUT;
        mAVCodecContext->channels = av_get_channel_layout_nb_channels(
                mAVCodecContext->channel_layout);
        mAVCodecContext->bit_rate = 96000;

        result = avcodec_open2(mAVCodecContext, mAVCodec, nullptr);
        if (result < 0) {
            LOGCATE("FFBaseEncoder::Init avcodec_open2 ret=%d", result)
            break;
        }

        av_dump_format(mAVFormatContext, 0, mFilePath, 1);

        mSrcFrame = av_frame_alloc();
        mSrcFrame->nb_samples = mAVCodecContext->frame_size;
        mSrcFrame->format = mAVCodecContext->sample_fmt;

        mFrameBufferSize = av_samples_get_buffer_size(nullptr, mAVCodecContext->channels,
                                                      mAVCodecContext->frame_size,
                                                      mAVCodecContext->sample_fmt, 1);

        LOGCATE("FFBaseEncoder::Init av_samples_get_buffer_size m_frameBufferSize=%d, nb_samples=%d",
                mFrameBufferSize, mSrcFrame->nb_samples);
        mFrameBuffer = (uint8_t *) malloc(mFrameBufferSize);
        avcodec_fill_audio_frame(mSrcFrame, mAVCodecContext->channels, mAVCodecContext->sample_fmt,
                                 mFrameBuffer, mFrameBufferSize, 1);

        //写文件头
        result = avformat_write_header(mAVFormatContext, nullptr);
        if (result != AVSTREAM_INIT_IN_INIT_OUTPUT) {
            LOGCATE("FFBaseEncoder::Init avformat_write_header ret=%d", result)
            break;
        }
        isWriteHeadSuccess = true;
        av_new_packet(mAVPacket, mFrameBufferSize);

        // 音频转码器
        mSwrContext = swr_alloc();
        av_opt_set_channel_layout(mSwrContext, "in_channel_layout", , 0);
        av_opt_set_channel_layout(mSwrContext, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
        av_opt_set_int(mSwrContext, "in_sample_rate", mSampleRate, 0);
        av_opt_set_int(mSwrContext, "out_sample_rate", DEFAULT_SAMPLE_RATE, 0);
        av_opt_set_int(mSwrContext, "in_sample_fmt", mSampleFormat, 0);
        av_opt_set_int(mSwrContext, "out_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
        swr_init(mSwrContext);
    } while (false);
    return 0;
}

void AudioEncoder::EncodeFrame() {

//    avcodec_send_frame();
//    avcodec_receive_packet()
//    av_write_frame(mAVFormatContext, mAVPacket);
}

void AudioEncoder::UnInit() {

    if (mSwrContext) {
        swr_free(&mSwrContext);
        mSwrContext = nullptr;
    }
    if (mAVPacket != nullptr) {
        av_packet_free(&mAVPacket);
        mAVPacket = nullptr;
    }
    if (mFrameBuffer) {
        delete mFrameBuffer;
        mFrameBuffer = nullptr;
    }
    if (mSrcFrame) {
        av_frame_free(&mSrcFrame);
        mSrcFrame = nullptr;
    }
    if (mSwrFrame) {
        av_frame_free(&mSrcFrame);
        mSrcFrame = nullptr;
    }
    if (mAVCodecContext) {
        avcodec_close(mAVCodecContext);
        avcodec_free_context(&mAVCodecContext);
        mAVCodec = nullptr;
        mAVCodecContext = nullptr;
    }
    if (mAVFormatContext != nullptr) {
        avio_close(mAVFormatContext->pb);
        mAVFormatContext = nullptr;
    }
}
