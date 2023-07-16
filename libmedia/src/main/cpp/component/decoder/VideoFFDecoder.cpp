//
// Created by bronyna on 2023/7/16.
//

#include "VideoFFDecoder.h"

#include <utility>

VideoFFDecoder::VideoFFDecoder(DecoderCallback *callback): FFmpegDecoder(callback) {
    hw_ctx = nullptr;
}

int VideoFFDecoder::OpenCodec(const AVCodecParameters *param) {
    enum AVHWDeviceType deviceType;
    AVCodecID codecId = param->codec_id;
    bool supportHwCodec = true;
    int i;
    int result = -1;

    deviceType = av_hwdevice_find_type_by_name("mediacodec");
    supportHwCodec = deviceType != AV_HWDEVICE_TYPE_NONE;
    if (!supportHwCodec) {
        while ((deviceType = av_hwdevice_iterate_types(deviceType)) != AV_HWDEVICE_TYPE_NONE) {
            LOGCATE("VideoFFDecoder::OpenCodec Available device types:%s", av_hwdevice_get_type_name(deviceType))
        }
    }

    switch (codecId) {
        case AV_CODEC_ID_H264:
            m_Codec = avcodec_find_decoder_by_name("h264_mediacodec");
            break;
        case AV_CODEC_ID_HEVC:
            m_Codec = avcodec_find_decoder_by_name("h265_mediacodec");
            break;
        default:;
    }

    if (m_Codec) {
        for (i = 0;; i++) {
            const AVCodecHWConfig *config = avcodec_get_hw_config(m_Codec, i);
            if (!config) {
                LOGCATE("Decoder %s does not support device type %s.\n", m_Codec->name, av_hwdevice_get_type_name(deviceType))
                supportHwCodec = false;
                m_Codec = avcodec_find_decoder(codecId); // 不支持时切换软解码器
            } else if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && config->device_type == deviceType) {
                pix_fmt = config->pix_fmt;
                LOGCATE("%s support pix format: %d.\n", m_Codec->name, config->pix_fmt)
                break;
            }
        }
    } else {
        m_Codec = avcodec_find_decoder(codecId);
    }
    if (!m_Codec){
        return result;
    }

    m_CodecCtx = avcodec_alloc_context3(m_Codec);
    if (!m_CodecCtx) {
        return result;
    }

    if (supportHwCodec) {
        m_CodecCtx->get_format = GetHwFormat;
        HwDecoderInit(m_CodecCtx, hw_ctx, deviceType);
    }

    result = avcodec_parameters_to_context(m_CodecCtx, param);
    if (result < 0) {
        LOGCATE("VideoFFDecoder::OpenCodec avcodec_parameters_to_context fail.")
        return result;
    }

    AVDictionary *pAVDictionary = nullptr;
    av_dict_set(&pAVDictionary, "buffer_size", "1024000", 0);
    av_dict_set(&pAVDictionary, "stimeout", "20000000", 0);
    av_dict_set(&pAVDictionary, "max_delay", "30000000", 0);
    av_dict_set(&pAVDictionary, "rtsp_transport", "tcp", 0);

    // 打开解码器
    result = avcodec_open2(m_CodecCtx, m_Codec, &pAVDictionary);
    if (result < 0) {
        LOGCATE("VideoFFDecoder::OpenCodec avcodec_open2 fail.")
    }

    return result;
}

std::shared_ptr<MediaFrame> VideoFFDecoder::OnFrameAvailable(AVFrame *avFrame, double timeBase) {
//    LOGCATE("AVInputEngine::VideoFrameAvailable")
    std::shared_ptr<MediaFrame> frame = std::make_shared<MediaFrame>();
    frame->type = MEDIA_TYPE_VIDEO;

    long dts = (long) ((avFrame->pkt_dts * timeBase) * 1000);
    long pts = (long) ((avFrame->pts * timeBase) * 1000);

    switch (avFrame->format) {
        case AV_PIX_FMT_YUV420P:
        case AV_PIX_FMT_YUVJ420P: {
            int yPlaneByteSize = avFrame->width * avFrame->height;
            int uvPlaneByteSize = yPlaneByteSize / 2;
            auto *data = (uint8_t *) malloc(yPlaneByteSize + uvPlaneByteSize);

            frame->format = IMAGE_FORMAT_I420;
            frame->width = avFrame->width;
            frame->height = avFrame->height;
            frame->plane[0] = data;
            frame->plane[1] = data + yPlaneByteSize;
            frame->plane[2] = data + yPlaneByteSize + uvPlaneByteSize / 2;
            memcpy(frame->plane[0], avFrame->data[0], yPlaneByteSize);
            memcpy(frame->plane[1], avFrame->data[1], uvPlaneByteSize / 2);
            memcpy(frame->plane[2], avFrame->data[2], uvPlaneByteSize / 2);
            frame->planeSize[0] = avFrame->linesize[0];
            frame->planeSize[1] = avFrame->linesize[1];
            frame->planeSize[2] = avFrame->linesize[2];
            frame->dts = dts;
            frame->pts = pts;

            if (frame->plane[0] && frame->plane[1] && !frame->plane[2] &&
                frame->planeSize[0] == frame->planeSize[1] && frame->planeSize[2] == 0) {
                // on some android device, output of h264 mediacodec decoder is NV12 兼容某些设备可能出现的格式不匹配问题
                frame->format = IMAGE_FORMAT_NV12;
            }
            break;
        }
        case AV_PIX_FMT_NV12: {
            int yPlaneByteSize = avFrame->width * avFrame->height;
            int uvPlaneByteSize = yPlaneByteSize / 2;
            auto *data = (uint8_t *) malloc(yPlaneByteSize + uvPlaneByteSize);

            frame->format = IMAGE_FORMAT_NV12;
            frame->width = avFrame->width;
            frame->height = avFrame->height;
            frame->plane[0] = data;
            frame->plane[1] = data + yPlaneByteSize;
            memcpy(frame->plane[0], avFrame->data[0], yPlaneByteSize);
            memcpy(frame->plane[1], avFrame->data[1], uvPlaneByteSize);
            frame->planeSize[0] = avFrame->linesize[0];
            frame->planeSize[1] = avFrame->linesize[1];
            frame->dts = dts;
            frame->pts = pts;
            break;
        }
        case AV_PIX_FMT_NV21: {
            int yPlaneByteSize = avFrame->width * avFrame->height;
            int uvPlaneByteSize = yPlaneByteSize / 2;
            auto *data = (uint8_t *) malloc(yPlaneByteSize + uvPlaneByteSize);

            frame->format = IMAGE_FORMAT_NV21;
            frame->width = avFrame->width;
            frame->height = avFrame->height;
            frame->plane[0] = data;
            frame->plane[1] = data + yPlaneByteSize;
            memcpy(frame->plane[0], avFrame->data[0], yPlaneByteSize);
            memcpy(frame->plane[1], avFrame->data[1], uvPlaneByteSize);
            frame->planeSize[0] = avFrame->linesize[0];
            frame->planeSize[1] = avFrame->linesize[1];
            frame->dts = dts;
            frame->pts = pts;
            break;
        }
        default:;
            // AV_PIX_FMT_RGBA
//            delete frame;
    }
    return frame;
}

enum AVPixelFormat VideoFFDecoder::GetHwFormat(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts) {
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }

    LOGCATE("Failed to get HW surface format.\n");
    return AV_PIX_FMT_NONE;
}

int VideoFFDecoder::HwDecoderInit(AVCodecContext *c, AVBufferRef *hw_ctx, const enum AVHWDeviceType type) {
    int err = 0;

    if ((err = av_hwdevice_ctx_create(&hw_ctx, type,
                                      nullptr, nullptr, 0)) < 0) {
        LOGCATE("Failed to create specified HW device.\n")
        return err;
    }
    c->hw_device_ctx = av_buffer_ref(hw_ctx);

    return err;
}

VideoFFDecoder::~VideoFFDecoder() {
    if (hw_ctx) {
        av_buffer_unref(&hw_ctx);
        hw_ctx = nullptr;
    }
}