//
// Created by 郝留凯 on 2023/10/21.
//

#ifndef HIKIKOMORI_FFAUDIOENCODER_H
#define HIKIKOMORI_FFAUDIOENCODER_H

#include "FFBaseEncoder.h"

#define   ADTS_HEADER_LEN   7

const int sampling_frequencies[] = {
        96000,  // 0x0
        88200,  // 0x1
        64000,  // 0x2
        48000,  // 0x3
        44100,  // 0x4
        32000,  // 0x5
        24000,  // 0x6
        22050,  // 0x7
        16000,  // 0x8
        12000,  // 0x9
        11025,  // 0xa
        8000   // 0xb
        // 0xc d e f是保留的
};

class FFAudioEncoder : public FFBaseEncoder {
private:
    int m_AudioSampleRate;

    int m_AudioChannelLayout;

    int m_AudioBitRate;

    SwrContext *m_SwrCtx = nullptr;

    long m_SampleCount = 0;

    int add_adts_header(char *const header, const int data_length,
                        const int profile, const int sample_rate,
                        const int channels);

public:
    FFAudioEncoder(AVCodecID encoderId, int audioSampleRate,
                   int audioChannelLayout, int audioBitRate);

    int OpenCodec(AVCodecParameters *parameters) override;

    int EncodeFrame() override;

    ~FFAudioEncoder();
};


#endif //HIKIKOMORI_FFAUDIOENCODER_H
