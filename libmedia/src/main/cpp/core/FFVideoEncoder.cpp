//
// Created by 郝留凯 on 2023/10/21.
//

#include "FFVideoEncoder.h"

FFVideoEncoder::FFVideoEncoder(AVCodecID encoderId, int imageWidth,
                               int imageHeight, int videoFrameRate, int64_t videoBitRate)
        : FFBaseEncoder(encoderId) {
    this->m_ImageWidth = imageWidth;
    this->m_ImageHeight = imageHeight;
    this->m_VideoFrameRate = videoFrameRate;
    this->m_VideoBitRate = videoBitRate;
}

int FFVideoEncoder::OpenCodec(AVCodecParameters *parameters) {
    LOGCATE("FFVideoEncoder::OpenCodec start: %lld", GetSysCurrentTime())
    int result = 0;

    // 查找编码器
    m_Codec = avcodec_find_encoder(m_EncoderId);
    if (!m_Codec) {
        result = -1;
        LOGCATE("FFVideoEncoder::OpenCodec avcodec_find_encoder:%s error", m_Codec->name)
        goto __EXIT;
    }
    LOGCATE("FFVideoEncoder::OpenCodec avcodec_find_encoder:%s", m_Codec->name)

    // 分配编码器上下文
    m_CodecCtx = avcodec_alloc_context3(m_Codec);
    if (!m_CodecCtx) {
        result = -1;
        LOGCATE("FFVideoEncoder::OpenCodec avcodec_alloc_context3 error")
        goto __EXIT;
    }

    // 配置编码器参数
    m_CodecCtx->codec_id = m_EncoderId;
    m_CodecCtx->bit_rate = m_VideoBitRate;
    m_CodecCtx->width = m_ImageWidth;
    m_CodecCtx->height = m_ImageHeight;
    m_CodecCtx->time_base = (AVRational) {1, m_VideoFrameRate};
    m_CodecCtx->framerate = (AVRational) {m_VideoFrameRate, 1};
    m_CodecCtx->gop_size = 12;
    m_CodecCtx->pix_fmt = AV_PIX_FMT_NV12;

    // 打开编码器
    result = avcodec_open2(m_CodecCtx, m_Codec, nullptr);
    if (result < 0) {
        LOGCATE("FFVideoEncoder::OpenCodec avcodec_open2 error:%s", av_err2str(result))
        goto __EXIT;
    }

    // 存放原始数据帧
    m_SrcFrame = av_frame_alloc();
    if (!m_SrcFrame) {
        LOGCATE("FFVideoEncoder::OpenCodec av_frame_alloc error")
        result = -1;
        goto __EXIT;
    }

    // 存放转换后的数据帧
    m_DstFrame = av_frame_alloc();
    if (!m_DstFrame) {
        LOGCATE("FFVideoEncoder::OpenCodec av_frame_alloc error")
        result = -1;
        goto __EXIT;
    }
    m_DstFrame->format = m_CodecCtx->pix_fmt;
    m_DstFrame->width = m_CodecCtx->width;
    m_DstFrame->height = m_CodecCtx->height;
    result = av_frame_get_buffer(m_DstFrame, 1);
    if (result < 0) {
        LOGCATE("FFVideoEncoder::OpenCodec av_frame_get_buffer error")
        goto __EXIT;
    }

    result = avcodec_parameters_from_context(parameters, m_CodecCtx);
    if (result < 0) {
        LOGCATE("FFVideoEncoder::OpenCodec avcodec_parameters_from_context error")
        goto __EXIT;
    }

//    m_CodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    LOGCATE("FFVideoEncoder::OpenCodec finish: %lld", GetSysCurrentTime())
    __EXIT:
    return result;
}

int FFVideoEncoder::EncodeFrame() {
//    LOGCATE("FFVideoEncoder::EncodeFrame")
    int result;
    AVPacket *pkt = av_packet_alloc();
    auto frame = m_EncoderQueue->poll();
    if (!frame) {
        result = -1;
        goto __EXIT;
    }

    m_SrcFrame->data[0] = frame->plane[0];
    m_SrcFrame->data[1] = frame->plane[1];
    m_SrcFrame->data[2] = frame->plane[2];
    m_SrcFrame->linesize[0] = frame->planeSize[0];
    m_SrcFrame->linesize[1] = frame->planeSize[1];
    m_SrcFrame->linesize[2] = frame->planeSize[2];
    m_SrcFrame->format = frame->format;
    m_SrcFrame->width = frame->width;
    m_SrcFrame->height = frame->height;

    result = av_frame_make_writable(m_DstFrame);
    if (result < 0) {
        LOGCATE("FFVideoEncoder::EncodeFrame av_frame_make_writable error")
        goto __EXIT;
    }

    if (frame->format != m_CodecCtx->pix_fmt) {
        if (!m_SwsCtx) {
            m_SwsCtx = sws_getContext(m_CodecCtx->width, m_CodecCtx->height, AVPixelFormat(frame->format),
                                      m_CodecCtx->width, m_CodecCtx->height, m_CodecCtx->pix_fmt,
                                      SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
            if (!m_SwsCtx) {
                LOGCATE("FFVideoEncoder::EncodeFrame sws_getContext error")
                result = -1;
                goto __EXIT;
            }
        }
        sws_scale(m_SwsCtx, m_SrcFrame->data, m_SrcFrame->linesize,
                  0, m_CodecCtx->height,
                  m_DstFrame->data, m_DstFrame->linesize);
    }
    m_DstFrame->pts = m_NextPts++;

    // 编码输入
    result = avcodec_send_frame(m_CodecCtx, m_DstFrame);
    if (result == AVERROR_EOF) {
        result = 1;
        goto __EXIT;
    } else if (result < 0) {
        LOGCATE("FFVideoEncoder::EncodeFrame avcodec_send_frame error. ret=%s", av_err2str(result))
        result = 0;
        goto __EXIT;
    }

    // 编码输出
    while (!result) {
        result = avcodec_receive_packet(m_CodecCtx, pkt);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            result = 0;
            goto __EXIT;
        } else if (result < 0) {
            LOGCATE("FFVideoEncoder::EncodeFrame avcodec_receive_packet error. ret=%s",
                    av_err2str(result))
            result = 0;
            goto __EXIT;
        }

        if (callback) {
            callback->OnFrameEncoded(pkt, AVMEDIA_TYPE_VIDEO);
        }
    }

    __EXIT:
    av_packet_free(&pkt);
    return result;
}

FFVideoEncoder::~FFVideoEncoder() {
    if (m_SwsCtx) {
        sws_freeContext(m_SwsCtx);
        m_SwsCtx = nullptr;
    }
    LOGCATE("FFAudioEncoder::~FFVideoEncoder()")
}
