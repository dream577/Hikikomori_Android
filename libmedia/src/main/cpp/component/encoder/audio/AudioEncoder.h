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

class AudioEncoder {
private:
    AVFormatContext *m_AVFormatContext = nullptr;
    AVCodecContext *m_AVCodecContext = nullptr;
    SwrContext *m_SwrContext = nullptr;

    AVStream *m_AVStream = nullptr;
    const AVCodec *m_AVCodec = nullptr;
    AVPacket *m_AVPacket = nullptr;
    AVFrame *m_SrcFrame = nullptr;
    AVFrame *m_SwrFrame = nullptr;
    uint8_t *m_FrameBuffer = nullptr;

    int mBitRate;
    int mChannels;
    int mSampleRate;
    int mBitPerSample;
    int mSampleFormat;
    int mFrameBufferSize;
    char mFilePath[1024] = {0};
    char mFileName[512] = {0};

    ThreadSafeQueue *m_AudioFrameQueue = nullptr;

    bool isWriteHeadSuccess = false;

public:

    void Init(int bitRate, int channels, int sampleRate, int bitPerSample, int sampleFormat,
              const char *aacFilePath);

    int StartEncode();

    void EncodeFrame();

    void UnInit();
};


#endif //HIKIKOMORI_AUDIOENCODER_H
