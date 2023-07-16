//
// Created by bronyna on 2023/7/16.
//

#ifndef HIKIKOMORI_AUDIOFFDECODER_H
#define HIKIKOMORI_AUDIOFFDECODER_H

#include "FFmpegDeocder.h"

extern "C" {
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
};

#define AUDIO_DST_SAMPLE_RATE       44100                   // 音频编码采样率
#define AUDIO_DST_CHANNEL_COUNTS    2                       // 音频编码通道数
#define AUDIO_DST_CHANNEL_LAYOUT    AV_CH_LAYOUT_STEREO     // 音频编码声道格式
#define AUDIO_DST_BIT_RATE          64000                   // 音频编码比特率
#define ACC_NB_SAMPLES              1024                    // ACC音频一帧采样数

class AudioFFDecoder : public FFmpegDecoder {
private:
    SwrContext *m_SwrContext;    // audio resample context
    int m_nbSamples = 0;         // number of sample per channel
    int m_DstFrameDataSize = 0;  // dst frame data size
    uint8_t *m_AudioOutBuffer;

protected:
    std::shared_ptr<MediaFrame> OnFrameAvailable(AVFrame *frame, double timeBase) override;

public:
    AudioFFDecoder(DecoderCallback *callback);

    int OpenCodec(const AVCodecParameters *param) override;

    ~AudioFFDecoder();

};


#endif //HIKIKOMORI_AUDIOFFDECODER_H