//
// Created by bronyna on 2023/2/5.
//

#include "VideoDecoder.h"
#include "LogUtil.h"

VideoDecoder::~VideoDecoder() {
    quit();
    VideoDecoder::UnInit();
}

int VideoDecoder::Init() {
    LOGCATE("VideoDecoder::Init")
    int result = FFBaseDecoder::Init();
    m_VideoWidth = m_AVCodecContext->width;
    m_VideoHeight = m_AVCodecContext->height;
    return result;
}

int VideoDecoder::UnInit() {
    LOGCATE("VideoDecoder::UnInit start")
    FFBaseDecoder::UnInit();
    return 0;
}

Frame *VideoDecoder::onFrameAvailable() {
    LOGCATE("VideoDecoder::onFrameAvailable")
    VideoFrame *frame = nullptr;

    AVRational timeBase = m_AVFormatContext->streams[m_StreamIndex]->time_base;
    long dts = (long) ((m_AVFrame->pkt_dts * av_q2d(timeBase)) * 1000);
    long pts = (long) ((m_AVFrame->pts * av_q2d(timeBase)) * 1000);

    if (m_AVCodecContext->pix_fmt == AV_PIX_FMT_YUV420P ||
        m_AVCodecContext->pix_fmt == AV_PIX_FMT_YUVJ420P) {
        frame = new VideoFrame();

        int yPlaneByteSize = m_AVFrame->width * m_AVFrame->height;
        int uvPlaneByteSize = yPlaneByteSize / 2;
        uint8_t *data = (uint8_t *) malloc(yPlaneByteSize + uvPlaneByteSize);

        frame->format = VIDEO_FRAME_FORMAT_I420;
        frame->width = m_AVFrame->width;
        frame->height = m_AVFrame->height;
        frame->yuvBuffer[0] = data;
        frame->yuvBuffer[1] = data + yPlaneByteSize;
        frame->yuvBuffer[2] = data + yPlaneByteSize + uvPlaneByteSize / 2;
        memcpy(frame->yuvBuffer[0], m_AVFrame->data[0], yPlaneByteSize);
        memcpy(frame->yuvBuffer[1], m_AVFrame->data[1], uvPlaneByteSize / 2);
        memcpy(frame->yuvBuffer[2], m_AVFrame->data[2], uvPlaneByteSize / 2);
        frame->planeSize[0] = m_AVFrame->linesize[0];
        frame->planeSize[1] = m_AVFrame->linesize[1];
        frame->planeSize[2] = m_AVFrame->linesize[2];
        frame->dts = dts;
        frame->pts = pts;

        if (frame->yuvBuffer[0] && frame->yuvBuffer[1] && !frame->yuvBuffer[2] &&
            frame->planeSize[0] == frame->planeSize[1] && frame->planeSize[2] == 0) {
            // on some android device, output of h264 mediacodec decoder is NV12 兼容某些设备可能出现的格式不匹配问题
            frame->format = VIDEO_FRAME_FORMAT_NV12;
        }
        LOGCATE("VideoDecoder::onFrameAvailable frame[w,h]=[%d,%d], [dts,pts]=[%ld,%ld], format=%d ,[line0,line1,line2]=[%d,%d,%d]",
                m_VideoWidth, m_VideoHeight, dts, pts, m_AVCodecContext->pix_fmt,
                m_AVFrame->linesize[0], m_AVFrame->linesize[1], m_AVFrame->linesize[2])
    }
    return frame;
}
