//
// Created by bronyna on 2023/7/16.
//

#ifndef HIKIKOMORI_VIDEOFFDECODER_H
#define HIKIKOMORI_VIDEOFFDECODER_H

#include "FFmpegDeocder.h"

static enum AVPixelFormat hw_pix_fmt;

class VideoFFDecoder : public FFmpegDecoder {

private:
    AVBufferRef *hw_ctx;
    AVPixelFormat pix_fmt;

protected:
    std::shared_ptr<MediaFrame> OnFrameAvailable(AVFrame *frame, double timeBase) override;

public:
    VideoFFDecoder(DecoderCallback *callback);

    int OpenCodec(const AVCodecParameters *param) override;

    static enum AVPixelFormat GetHwFormat(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);

    static int
    HwDecoderInit(AVCodecContext *c, AVBufferRef *hw_ctx, const enum AVHWDeviceType type);

    ~VideoFFDecoder();
};


#endif //HIKIKOMORI_VIDEOFFDECODER_H
