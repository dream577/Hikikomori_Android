//
// Created by bronyna on 2023/7/16.
//

#ifndef HIKIKOMORI_FFBASEDECODER_H
#define HIKIKOMORI_FFBASEDECODER_H

#include <memory>

extern "C" {
#include "libavcodec/avcodec.h"
}

#include "MediaDef.h"
#include "LogUtil.h"
#include "Callback.h"

#define MAX_VIDEO_FRAME_SIZE 30
#define MAX_AUDIO_FRAME_SIZE 100

class FFBaseDecoder {
private:
    AVFrame *m_Frame;

protected:
    shared_ptr<LinkedBlockingQueue<MediaFrame>> pool;

    DecoderCallback *m_Callback;

    AVCodecContext *m_CodecCtx;

    double timebase = 0;

    const AVCodec *m_Codec;

    virtual std::shared_ptr<MediaFrame> _OnFrameAvailable(AVFrame *frame) = 0;

public:

    FFBaseDecoder(DecoderCallback *callback, double timebase) {
        this->m_Callback = callback;
        this->m_CodecCtx = nullptr;
        this->m_Codec = nullptr;
        this->m_Frame = av_frame_alloc();
        this->timebase = timebase;
    }

    virtual int OpenCodec(const AVCodecParameters *param) = 0;

    int Decode(AVPacket *packet) {
        int result = 0;
        int frameCount = 0;
        result = avcodec_send_packet(m_CodecCtx, packet);
        if (result == AVERROR_EOF) {
            goto __EXIT;
        }

        while ((result = avcodec_receive_frame(m_CodecCtx, m_Frame)) == 0) {
            std::shared_ptr<MediaFrame> frame = _OnFrameAvailable(m_Frame);
            frameCount++;
            m_Callback->OnFrameReady(frame);
        }

        __EXIT:
        if (result == 0 || frameCount > 0) return frameCount;
        return result;
    }

    void Flush() {
        avcodec_flush_buffers(m_CodecCtx);
    }

    void Stop() {
        if (pool) {
            pool->overrule();
            pool->clear();
        }
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

    ~FFBaseDecoder() {
        CloseCodec();
        m_Callback = nullptr;
    }
};

#endif //HIKIKOMORI_FFBASEDECODER_H
