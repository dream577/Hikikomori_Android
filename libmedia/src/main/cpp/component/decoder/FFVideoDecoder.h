//
// Created by bronyna on 2023/7/16.
//

#ifndef HIKIKOMORI_FFVIDEODECODER_H
#define HIKIKOMORI_FFVIDEODECODER_H

#include "FFBaseDeocder.h"

static enum AVPixelFormat hw_pix_fmt;

class FFVideoDecoder : public FFBaseDecoder {

private:
    AVBufferRef *hw_ctx;
    AVPixelFormat pix_fmt;

protected:
    std::shared_ptr<MediaFrame> OnFrameAvailable(AVFrame *frame) override;

public:
    FFVideoDecoder(DecoderCallback *callback, double timebase);

    int OpenCodec(const AVCodecParameters *param) override;

    static enum AVPixelFormat GetHwFormat(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);

    static int
    HwDecoderInit(AVCodecContext *c, AVBufferRef *hw_ctx, const enum AVHWDeviceType type);

    ~FFVideoDecoder();
};


#endif //HIKIKOMORI_FFVIDEODECODER_H
