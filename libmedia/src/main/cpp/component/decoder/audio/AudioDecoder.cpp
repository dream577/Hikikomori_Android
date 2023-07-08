//
// Created by bronyna on 2023/2/5.
//

#include "AudioDecoder.h"
#include "LogUtil.h"

Frame *AudioDecoder::onFrameAvailable() {
    int result = swr_convert(m_SwrContext, &m_AudioOutBuffer, m_DstFrameDataSize / 2,
                             (const uint8_t **) m_AVFrame->data, m_AVFrame->nb_samples);

    Frame *frame = nullptr;
    if (result > 0) {
        uint8_t *data = (uint8_t *) malloc(m_DstFrameDataSize);
        memcpy(data, m_AudioOutBuffer, m_DstFrameDataSize);

        AVRational timeBase = m_AVFormatContext->streams[m_StreamIndex]->time_base;
        long dts = (long) ((m_AVFrame->pkt_dts * av_q2d(timeBase)) * 1000);
        long pts = (long) ((m_AVFrame->pts * av_q2d(timeBase)) * 1000);
        int channels = m_AVCodecContext->channels;
        int sampleRate = m_AVCodecContext->sample_rate;

        frame = new AudioFrame(data, m_DstFrameDataSize, channels, sampleRate,
                               dts, pts, -1);
//        LOGCATE("AudioDecoder::onFrameAvailable data_size=%d channels=%d sampleRate=%d dts=%ld pts=%ld format=%d",
//                m_DstFrameDataSize, channels, sampleRate, dts, pts, -1);
    }
    return frame;
}

int AudioDecoder::init() {
    LOGCATE("AudioDecoder::init");
    int result = -1;
    do {
        result = FFBaseDecoder::init();
        if (result != 0) {
            break;
        }
        m_SwrContext = swr_alloc();
        av_opt_set_int(m_SwrContext, "in_channel_layout", m_AVCodecContext->channel_layout, 0);
        av_opt_set_int(m_SwrContext, "out_channel_layout", AUDIO_DST_CHANNEL_LAYOUT, 0);

        av_opt_set_int(m_SwrContext, "in_sample_rate", m_AVCodecContext->sample_rate, 0);
        av_opt_set_int(m_SwrContext, "out_sample_rate", AUDIO_DST_SAMPLE_RATE, 0);

        av_opt_set_sample_fmt(m_SwrContext, "in_sample_fmt", m_AVCodecContext->sample_fmt, 0);
        av_opt_set_sample_fmt(m_SwrContext, "out_sample_fmt", DST_SAMPLE_FORMAT, 0);

        swr_init(m_SwrContext);

        LOGCATE("AudioDecoder::init audio metadata sample rate: %d, channel: %d, format: %d, frame_size: %d, layout: %lld",
                m_AVCodecContext->sample_rate, m_AVCodecContext->channels,
                m_AVCodecContext->sample_fmt, m_AVCodecContext->frame_size,
                m_AVCodecContext->channel_layout);
        m_nbSamples = (int) av_rescale_rnd(ACC_NB_SAMPLES, AUDIO_DST_SAMPLE_RATE,
                                           m_AVCodecContext->sample_rate,
                                           AV_ROUND_UP);
        m_DstFrameDataSize = av_samples_get_buffer_size(nullptr, AUDIO_DST_CHANNEL_COUNTS,
                                                        m_nbSamples,
                                                        DST_SAMPLE_FORMAT, 1);

        LOGCATE("AudioDecoder::init [m_nbSamples, m_DstFrameDataSze]=[%d, %d]", m_nbSamples,
                m_DstFrameDataSize);

        m_AudioOutBuffer = (uint8_t *) malloc(m_DstFrameDataSize);
    } while (false);
    return result;
}

int AudioDecoder::unInit() {
    LOGCATE("AudioDecoder::unInit start");
    if (m_AudioOutBuffer) {
        delete m_AudioOutBuffer;
        m_AudioOutBuffer = nullptr;
    }

    if (m_SwrContext) {
        swr_free(&m_SwrContext);
        m_SwrContext = nullptr;
    }

    FFBaseDecoder::unInit();
    LOGCATE("AudioDecoder::unInit finish");
    return 0;
}