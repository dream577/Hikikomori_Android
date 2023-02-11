//
// Created by bronyna on 2023/2/5.
//

#include "VideoDecoder.h"
#include "LogUtil.h"

int VideoDecoder::init() {
    LOGCATE("VideoDecoder::init")
    int result = FFBaseDecoder::init();
    m_VideoWidth = m_AVCodecContext->width;
    m_VideoHeight = m_AVCodecContext->height;
    return result;
}

VideoDecoder::~VideoDecoder() {
    VideoDecoder::unInit();
}

int VideoDecoder::unInit() {
    LOGCATE("VideoDecoder::unInit")
    FFBaseDecoder::unInit();
    if (m_SwsContext) {
        sws_freeContext(m_SwsContext);
        m_SwsContext = nullptr;
    }

    if (m_FrameBuffer) {
        free(m_FrameBuffer);
        m_FrameBuffer = nullptr;
    }

    if (m_RGBAFrame) {
        av_frame_free(&m_RGBAFrame);
        m_RGBAFrame = nullptr;
    }
    return 0;
}

Frame *VideoDecoder::OnFrameAvailable() {
    LOGCATE("VideoDecoder::OnFrameAvailable")
    Frame *frame = nullptr;
    if (useNaiveWindow) {
        AVRational timeBase = m_AVFormatContext->streams[m_StreamIndex]->time_base;
        long dts = (long) ((m_AVFrame->pkt_dts * av_q2d(timeBase)) * 1000);
        long pts = (long) ((m_AVFrame->pts * av_q2d(timeBase)) * 1000);

        sws_scale(m_SwsContext, m_AVFrame->data, m_AVFrame->linesize, 0,
                  m_VideoHeight, m_RGBAFrame->data, m_RGBAFrame->linesize);

        uint8_t *data = (uint8_t *) malloc(m_BufferSize);
        memcpy(data, m_RGBAFrame->data[0], m_BufferSize);

        uint8_t *yuvBuffer[3];
        int planeSize[3] = {m_RGBAFrame->linesize[0], 0, 0};
        yuvBuffer[0] = data;
        yuvBuffer[1] = yuvBuffer[2] = nullptr;
        frame = new VideoFrame(yuvBuffer, planeSize, m_RenderWidth, m_RenderHeight,
                               dts, pts, VIDEO_FRAME_FORMAT_RGBA);
        LOGCATE("VideoDecoder::OnFrameAvailable frame[w,h]=[%d,%d], [dts,pts]=[%ld,%ld], format=%d ,[line0,line1,line2]=[%d,%d,%d]",
                m_RenderWidth, m_RenderHeight, dts, pts, m_AVCodecContext->pix_fmt,
                planeSize[0],
                planeSize[1], planeSize[2])
    }
    return frame;
}

void VideoDecoder::startDecodeThread() {
    LOGCATE("VideoDecoder::startDecodeThread")
    FFBaseDecoder::startDecodeThread();
}

void VideoDecoder::onDecoderReady() {
    Decoder::onDecoderReady();
    if (useNaiveWindow) {
        m_RGBAFrame = av_frame_alloc();
        m_BufferSize = av_image_get_buffer_size(DST_PIXEL_FORMAT, m_RenderWidth, m_RenderHeight,1);

        m_FrameBuffer = (uint8_t *) av_malloc(m_BufferSize * sizeof(uint8_t));

        av_image_fill_arrays(m_RGBAFrame->data, m_RGBAFrame->linesize,
                             m_FrameBuffer, DST_PIXEL_FORMAT, m_RenderWidth, m_RenderHeight, 1);

        m_SwsContext = sws_getContext(m_VideoWidth, m_VideoHeight, m_AVCodecContext->pix_fmt,
                                      m_RenderWidth, m_RenderHeight, DST_PIXEL_FORMAT,
                                      SWS_BICUBIC, nullptr, nullptr, nullptr);
    }
}
