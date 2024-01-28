//
// Created by 郝留凯 on 2024/1/28.
//

#include "FFBaseDecoder.h"

FFBaseDecoder::FFBaseDecoder(AVDecoderCallback *callback, double timebase) {
    this->m_Callback = callback;
    this->m_CodecCtx = nullptr;
    this->m_Codec = nullptr;
    this->m_Frame = av_frame_alloc();
    this->timebase = timebase;
}

int FFBaseDecoder::DecodeInput(AVPacket *packet) {
    return avcodec_send_packet(m_CodecCtx, packet);
}

int FFBaseDecoder::DecodeOutput() {
    int result;
    while ((result = avcodec_receive_frame(m_CodecCtx, m_Frame)) == 0) {
        std::shared_ptr<AVMediaFrame> frame = _OnFrameAvailable(m_Frame);
        m_Callback->OnFrameReady(frame);
    }
    return result;
}

void FFBaseDecoder::FlushDecoder() {
    avcodec_flush_buffers(m_CodecCtx);
}

void FFBaseDecoder::StopDecoder() {
    if (mOutputFramePool) {
        mOutputFramePool->overrule();
        mOutputFramePool->clear();
    }
    if (m_CodecCtx) {
        avcodec_close(m_CodecCtx);
        m_CodecCtx = nullptr;
    }
    if (m_Frame) {
        av_frame_free(&m_Frame);
        m_Frame = nullptr;
    }
}

FFBaseDecoder::~FFBaseDecoder() {
    StopDecoder();
    m_Callback = nullptr;
}