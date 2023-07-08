//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_AUDIODECODER_H
#define HIKIKOMORI_AUDIODECODER_H

extern "C" {
#include "libswresample/swresample.h"
};

#include "FFBaseDecoder.h"

// 音频编码采样率
static const int AUDIO_DST_SAMPLE_RATE = 44100;
// 音频编码通道数
static const int AUDIO_DST_CHANNEL_COUNTS = 2;
// 音频编码声道格式
static const uint64_t AUDIO_DST_CHANNEL_LAYOUT = AV_CH_LAYOUT_STEREO;
// 音频编码比特率
static const int AUDIO_DST_BIT_RATE = 64000;
// ACC音频一帧采样数
static const int ACC_NB_SAMPLES = 1024;

class AudioDecoder : public FFBaseDecoder {

private:

    SwrContext *m_SwrContext;    // audio resample context

    int m_nbSamples = 0;         // number of sample per channel

    int m_DstFrameDataSize = 0;  // dst frame data size

    uint8_t *m_AudioOutBuffer = nullptr;

    const AVSampleFormat DST_SAMPLE_FORMAT = AV_SAMPLE_FMT_S16;

    virtual Frame *onFrameAvailable() override;

protected:

public:
    AudioDecoder(char *path, MediaEventCallback *eventCallback, DecoderCallback *callback)
            : FFBaseDecoder(path, AVMEDIA_TYPE_AUDIO, callback) {
        m_EventCallback = eventCallback;
    }

    virtual ~AudioDecoder() {}

    virtual int init() override;

    virtual int unInit() override;
};


#endif //HIKIKOMORI_AUDIODECODER_H
