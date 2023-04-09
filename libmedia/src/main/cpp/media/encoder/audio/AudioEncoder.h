//
// Created by bronyna on 2023/2/26.
//

#ifndef HIKIKOMORI_AUDIOENCODER_H
#define HIKIKOMORI_AUDIOENCODER_H

#include "MediaDef.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/frame.h"
#include "libavutil/time.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
};

#include "ThreadSafeQueue.h"

#define DEFAULT_SAMPLE_RATE    44100
#define DEFAULT_CHANNEL_LAYOUT AV_CH_LAYOUT_STEREO

class AudioEncoder {
private:
    AVFormatContext *mAVFormatContext = nullptr;
    AVCodecContext *mAVCodecContext = nullptr;
    SwrContext *mSwrContext = nullptr;

    AVStream *mAVStream = nullptr;
    AVCodec *mAVCodec = nullptr;
    AVPacket *mAVPacket = nullptr;
    AVFrame *mSrcFrame = nullptr;
    AVFrame *mSwrFrame = nullptr;
    uint8_t *mFrameBuffer = nullptr;

    int mBitRate;
    int mChannels;
    int mSampleRate;
    int mBitPerSample;
    int mSampleFormat;
    int mFrameBufferSize;
    char mFilePath[1024] = {0};

    bool isWriteHeadSuccess = false;

public:

    void Init(int bitRate, int channels, int sampleRate, int bitPerSample, int sampleFormat,const char *aacFilePath);

    int StartEncode();

    void EncodeFrame();

    void UnInit();
};


#endif //HIKIKOMORI_AUDIOENCODER_H
