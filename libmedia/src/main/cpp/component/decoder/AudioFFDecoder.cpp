//
// Created by bronyna on 2023/7/16.
//

#include "AudioFFDecoder.h"

#include <utility>

AudioFFDecoder::AudioFFDecoder(DecoderCallback *callback) : FFmpegDecoder(
        callback) {
    m_SwrContext = nullptr;
    m_AudioOutBuffer = nullptr;
}

int AudioFFDecoder::OpenCodec(const AVCodecParameters *param) {
    AVCodecID codecId = param->codec_id;
    int result = -1;

    m_Codec = avcodec_find_decoder(codecId);
    if (!m_Codec) {
        return result;
    }

    m_CodecCtx = avcodec_alloc_context3(m_Codec);
    if (!m_CodecCtx) {
        return result;
    }

    result = avcodec_parameters_to_context(m_CodecCtx, param);
    if (result < 0) {
        LOGCATE("AudioFFDecoder::OpenCodec avcodec_parameters_to_context fail.")
        return result;
    }

    AVDictionary *pAVDictionary = nullptr;
    av_dict_set(&pAVDictionary, "buffer_size", "1024000", 0);
    av_dict_set(&pAVDictionary, "stimeout", "20000000", 0);
    av_dict_set(&pAVDictionary, "max_delay", "30000000", 0);
    av_dict_set(&pAVDictionary, "rtsp_transport", "tcp", 0);

    // 打开解码器
    result = avcodec_open2(m_CodecCtx, m_Codec, &pAVDictionary);
    if (result < 0) {
        LOGCATE("AudioFFDecoder::OpenCodec avcodec_open2 fail.")
        return result;
    }

    m_SwrContext = swr_alloc();
    av_opt_set_int(m_SwrContext, "in_channel_layout", m_CodecCtx->channel_layout, 0);
    av_opt_set_int(m_SwrContext, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);

    av_opt_set_int(m_SwrContext, "in_sample_rate", m_CodecCtx->sample_rate, 0);
    av_opt_set_int(m_SwrContext, "out_sample_rate", AUDIO_DST_SAMPLE_RATE, 0);

    av_opt_set_sample_fmt(m_SwrContext, "in_sample_fmt", m_CodecCtx->sample_fmt, 0);
    av_opt_set_sample_fmt(m_SwrContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    swr_init(m_SwrContext);

    LOGCATE("AudioFFDecoder::OpenCodec audio metadata sample rate: %d, channel: %d, format: %d, frame_size: %d, layout: %lu",
            m_CodecCtx->sample_rate, m_CodecCtx->channels, m_CodecCtx->sample_fmt,
            m_CodecCtx->frame_size, m_CodecCtx->channel_layout)
    m_nbSamples = (int) av_rescale_rnd(ACC_NB_SAMPLES, AUDIO_DST_SAMPLE_RATE,
                                       m_CodecCtx->sample_rate, AV_ROUND_UP);
    m_DstFrameDataSize = av_samples_get_buffer_size(nullptr, AUDIO_DST_CHANNEL_COUNTS,
                                                    m_nbSamples, AV_SAMPLE_FMT_S16, 1);

    LOGCATE("AudioFFDecoder::OpenCodec [m_nbSamples, m_DstFrameDataSize]=[%d, %d]",
            m_nbSamples, m_DstFrameDataSize);

    m_AudioOutBuffer = (uint8_t *) malloc(m_DstFrameDataSize);

    return result;
}

std::shared_ptr<MediaFrame> AudioFFDecoder::OnFrameAvailable(AVFrame *avFrame, double timeBase) {

    int result = swr_convert(m_SwrContext, &m_AudioOutBuffer, m_DstFrameDataSize / 2,
                             (const uint8_t **) avFrame->data, avFrame->nb_samples);
    std::shared_ptr<MediaFrame> frame = std::make_shared<MediaFrame>();
    frame->type = MEDIA_TYPE_AUDIO;
    if (result > 0) {
        auto *data = (uint8_t *) malloc(m_DstFrameDataSize);
        memcpy(data, m_AudioOutBuffer, m_DstFrameDataSize);

        long dts = (long) ((avFrame->pkt_dts * timeBase) * 1000);
        long pts = (long) ((avFrame->pts * timeBase) * 1000);
        int channels = m_CodecCtx->channels;
        int sampleRate = m_CodecCtx->sample_rate;

        frame->plane[0] = data;
        frame->planeSize[0] = m_DstFrameDataSize;
        frame->channels = channels;
        frame->sampleRate = sampleRate;
        frame->dts = dts;
        frame->pts = pts;
        frame->format = -1;
    }

//    LOGCATE("AudioFFDecoder::onFrameAvailable data_size=%d channels=%d sampleRate=%d dts=%ld pts=%ld format=%d",
//            m_DstFrameDataSize, frame->channels, frame->sampleRate, frame->pts, frame->dts, -1);
    return frame;
}

AudioFFDecoder::~AudioFFDecoder() {
    if (m_SwrContext) {
        swr_free(&m_SwrContext);
        m_SwrContext = nullptr;
    }
    if (m_AudioOutBuffer) {
        free(m_AudioOutBuffer);
        m_AudioOutBuffer = nullptr;
    }
}
