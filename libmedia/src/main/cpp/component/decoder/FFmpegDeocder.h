//
// Created by bronyna on 2023/7/16.
//

#ifndef HIKIKOMORI_FFMPEGDEOCDER_H
#define HIKIKOMORI_FFMPEGDEOCDER_H

#include <memory>

extern "C" {
#include "libavcodec/avcodec.h"
}

#include "MediaDef.h"
#include "LogUtil.h"
#include "Callback.h"

class FFmpegDecoder {
private:
    AVFrame *m_Frame;

protected:
    DecoderCallback *m_Callback;

    AVCodecContext *m_CodecCtx;

    const AVCodec *m_Codec;

    virtual std::shared_ptr<MediaFrame> OnFrameAvailable(AVFrame *frame, double timeBase) = 0;

public:

    FFmpegDecoder(DecoderCallback *callback) {
        this->m_Callback = callback;
        m_CodecCtx = nullptr;
        m_Codec = nullptr;
        m_Frame = av_frame_alloc();
    }

    virtual int OpenCodec(const AVCodecParameters *param) = 0;

    int Decode(AVPacket *packet, double timeBase) {
        int result = 0;
        int frameCount = 0;
        result = avcodec_send_packet(m_CodecCtx, packet);
        if (result == AVERROR_EOF) {
            goto __EXIT;
        }

        while ((result = avcodec_receive_frame(m_CodecCtx, m_Frame)) == 0) {
            std::shared_ptr<MediaFrame> frame = OnFrameAvailable(m_Frame, timeBase);
            frameCount++;
            m_Callback->OnDecodeOneFrame(frame);
        }

        __EXIT:
        if (result == 0) return frameCount;
        return result;
    }

    void flush() {
        avcodec_flush_buffers(m_CodecCtx);
    }

    void CloseCodec() {
        if (m_CodecCtx) {
            avcodec_close(m_CodecCtx);
            m_CodecCtx = nullptr;
        }
        if (m_Frame) {
            av_frame_free(&m_Frame);
            m_Frame = nullptr;
        }
    }

    ~FFmpegDecoder() {
        CloseCodec();
        m_Callback = nullptr;
    }
};

#endif //HIKIKOMORI_FFMPEGDEOCDER_H
