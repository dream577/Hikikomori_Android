//
// Created by bronyna on 2023/7/16.
//

#ifndef HIKIKOMORI_FFBASEDECODER_H
#define HIKIKOMORI_FFBASEDECODER_H

#include <memory>

extern "C" {
#include "libavcodec/avcodec.h"
}

#include "FFMediaDef.h"
#include "LogUtil.h"
#include "Callback.h"

#define MAX_VIDEO_FRAME_SIZE 30
#define MAX_AUDIO_FRAME_SIZE 100

class FFBaseDecoder : public looper {
private:
    AVFrame *m_Frame;

protected:
    shared_ptr<LinkedBlockingQueue<AVPacket>> mInputPacketQueue;

    shared_ptr<LinkedBlockingQueue<AVMediaFrame>> mOutputFramePool;

    AVDecoderCallback *m_Callback;

    AVCodecContext *m_CodecCtx;

    double timebase = 0;

    const AVCodec *m_Codec;

    virtual std::shared_ptr<AVMediaFrame> _OnFrameAvailable(AVFrame *frame) = 0;

public:

    FFBaseDecoder(AVDecoderCallback *callback, double timebase);

    virtual int OpenCodec(const AVCodecParameters *param) = 0;

    int DecodeInput(AVPacket *packet);

    int DecodeOutput();

    void FlushDecoder();

    void StopDecoder();

    ~FFBaseDecoder();
};

#endif //HIKIKOMORI_FFBASEDECODER_H
