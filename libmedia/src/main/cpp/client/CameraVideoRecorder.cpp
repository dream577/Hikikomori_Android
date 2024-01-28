//
// Created by bronyna on 2023/2/25.
//

#include "CameraVideoRecorder.h"

CameraVideoRecorder::CameraVideoRecorder() {
    LOGCATE("CameraVideoRecorder::CameraVideoRecorder")
    m_FormatCtx = nullptr;
}

void CameraVideoRecorder::OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) {
    m_VideoRenderQueue = make_shared<LinkedBlockingQueue<AVMediaFrame>>(15);
    m_RenderWindow = make_shared<GLRenderWindow>(this);
    m_RenderWindow->StartLoop();
    m_RenderWindow->OnSurfaceCreated(jniEnv, surface);
}

void CameraVideoRecorder::OnSurfaceChanged(int width, int height) {
    m_RenderWindow->OnSurfaceChanged(width, height);
}

void CameraVideoRecorder::OnSurfaceDestroyed() {
    m_RenderWindow->OnSurfaceDestroyed();
}

void CameraVideoRecorder::UpdateMVPMatrix(float translateX, float translateY, float scaleX,
                                          float scaleY, int degree, int mirror) {
    m_RenderWindow->UpdateMVPMatrix(translateX, translateY, scaleX, scaleY, degree, mirror);
}

void CameraVideoRecorder::StartRecord() {
    LOGCATE("CameraVideoRecorder::StartRecord()")
    if (m_IsRecording) return;
    // 申请AVFormatContext，主要是在进行封装格式相关的操作时作为操作上下文的线索
    int result = avformat_alloc_output_context2(&m_FormatCtx, nullptr, nullptr, m_FileName);
    if (result < 0) {
        result = avformat_alloc_output_context2(&m_FormatCtx, nullptr, "mp4", m_FileName);
    }
    if (result < 0) {
        LOGCATE("CameraVideoRecorder::StartRecord() avformat_alloc_output_context2 ret=%d", result)
        return;
    }

    m_VideoEncoder = make_shared<FFVideoEncoder>(AV_CODEC_ID_H264, m_ImageWidth,
                                                 m_ImageHeight, m_FrameRate,
                                                 m_VideoBitRate);
    m_VideoEncoder->SetEncodeCallback(this);
    // 申请存放视频的AVStream
    m_VideoStream = avformat_new_stream(m_FormatCtx, nullptr);
    if (m_VideoStream) {
        m_VideoStream->id = (int) (m_FormatCtx->nb_streams) - 1;
        result = m_VideoEncoder->OpenCodec(m_VideoStream->codecpar);
        m_VideoStream->time_base = (AVRational) {1, m_FrameRate};
    }
    if (result < 0) {
        m_VideoEncoder.reset();
        LOGCATE("CameraVideoRecorder::StartRecord() open video encoder error")
        m_IsRecording = false;
        goto __EXIT;
    }

    m_AudioEncoder = make_shared<FFAudioEncoder>(AV_CODEC_ID_AAC, m_SampleRate,
                                                 m_ChannelLayout, 96000);
    m_AudioEncoder->SetEncodeCallback(this);
    // 申请存放音频的AVStream
    m_AudioStream = avformat_new_stream(m_FormatCtx, nullptr);
    if (m_AudioStream) {
        m_AudioStream->id = (int) (m_FormatCtx->nb_streams) - 1;
        result = m_AudioEncoder->OpenCodec(m_AudioStream->codecpar);
        m_AudioStream->time_base = (AVRational) {1, m_SampleRate};
    }
    if (result < 0) {
        m_AudioEncoder.reset();
        LOGCATE("CameraVideoRecorder::StartRecord() open audio encoder error")
        m_IsRecording = false;
        goto __EXIT;
    }

    av_dump_format(m_FormatCtx, 0, m_FilePath, 1);
    /* open the output file, if needed */
    if (!(m_FormatCtx->flags & AVFMT_NOFILE)) {
        result = avio_open(&m_FormatCtx->pb, m_FilePath, AVIO_FLAG_WRITE);
        if (result < 0) {
            LOGCATE("CameraVideoRecorder::Init() Could not open '%s': %s", m_FilePath,
                    av_err2str(result))
        }
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&worker, &attr, _StartRecordLoop, this);

    __EXIT:
    return;
}

void CameraVideoRecorder::InputVideoFrame(uint8_t *data, int width, int height, int format) {
//    LOGCATE("CameraVideoRecorder::InputVideoFrame")
    shared_ptr<AVMediaFrame> frame = make_shared<AVMediaFrame>();
    switch (format) {
        case AV_PIX_FMT_YUV420P: {
            int yPlaneByteSize = width * height;
            int uvPlaneByteSize = yPlaneByteSize / 2;

            frame->format = format;
            frame->width = width;
            frame->height = height;
            frame->plane[0] = data;
            frame->plane[1] = data + yPlaneByteSize;
            frame->plane[2] = data + yPlaneByteSize + uvPlaneByteSize / 2;
            frame->planeSize[0] = width;
            frame->planeSize[1] = width / 2;
            frame->planeSize[2] = width / 2;
            break;
        }
        case AV_PIX_FMT_NV12:
        case AV_PIX_FMT_NV21: {
            int yPlaneByteSize = width * height;
            int uvPlaneByteSize = yPlaneByteSize / 2;

            frame->format = format;
            frame->width = width;
            frame->height = height;
            frame->plane[0] = data;
            frame->plane[1] = data + yPlaneByteSize;
            frame->planeSize[0] = width;
            frame->planeSize[1] = width / 2;
            break;
        }
        default: {
            // 默认为 AV_PIX_FMT_RGBA
            int yPlaneByteSize = width * height;
            frame->format = format;
            frame->width = width;
            frame->height = height;
            frame->plane[0] = data;
            frame->plane[1] = data + yPlaneByteSize;
            frame->plane[2] = data + yPlaneByteSize * 2;
            frame->planeSize[0] = width;
            frame->planeSize[1] = width;
            frame->planeSize[2] = width;
        }
    }

    frame->type = AVMEDIA_TYPE_VIDEO;
    if (m_RenderWindow) {
        m_VideoRenderQueue->offer(frame);
    } else if (m_VideoEncoder && m_IsRecording) {
        m_VideoEncoder->InputFrame(frame);
    }
}

void CameraVideoRecorder::FrameRendFinish(shared_ptr<AVMediaFrame> frame) {
//    LOGCATE("CameraVideoRecorder::FrameRendFinish")
    if (frame && frame->type == AVMEDIA_TYPE_VIDEO) {
        if (m_VideoEncoder && m_IsRecording) {
            m_VideoEncoder->InputFrame(frame);
        }
    }
}

void CameraVideoRecorder::InputAudioFrame(uint8_t *data, int size, int sample_rate,
                                          int sample_format, int channel_layout) {
    if (!m_IsRecording) {
        delete data;
        return;
    }
    auto frame = make_shared<AVMediaFrame>();
    frame->plane[0] = data;
    frame->planeSize[0] = size;
    frame->sampleRate = sample_rate;
    frame->sampleFormat = sample_format;
    frame->channelLayout = channel_layout;
    frame->type = AVMEDIA_TYPE_AUDIO;
    if (m_AudioEncoder && m_IsRecording) {
        m_AudioEncoder->InputFrame(frame);
    }
}

shared_ptr<AVMediaFrame> CameraVideoRecorder::GetOneFrame(int type) {
//    LOGCATE("CameraVideoRecorder::GetOneFrame")
    return m_VideoRenderQueue->poll();
}

void CameraVideoRecorder::OnFrameEncoded(AVPacket *pkt, AVMediaType type) {
    int streamIndex = 0;
    AVRational srcTimebase, dstTimebase;
    if (type == AVMEDIA_TYPE_VIDEO) {
        streamIndex = m_VideoStream->index;
        srcTimebase = AVRational{1, m_FrameRate};
        dstTimebase = m_VideoStream->time_base;
//        LOGCATE("CameraVideoRecorder::OnFrameEncoded video[pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s]",
//                av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, &dstTimebase),
//                av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, &dstTimebase),
//                av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, &dstTimebase))
    } else {
        streamIndex = m_AudioStream->index;
        srcTimebase = AVRational{1, m_SampleRate};
        dstTimebase = m_AudioStream->time_base;
//        LOGCATE("CameraVideoRecorder::OnFrameEncoded audio[pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s]",
//                av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, &dstTimebase),
//                av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, &dstTimebase),
//                av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, &dstTimebase))
    }
    pkt->stream_index = streamIndex;
    av_packet_rescale_ts(pkt, srcTimebase, dstTimebase);
    av_interleaved_write_frame(m_FormatCtx, pkt);
}

void *CameraVideoRecorder::_StartRecordLoop(void *recorder) {
    auto *mRecorder = (CameraVideoRecorder *) recorder;
    int result;

    mRecorder->m_IsRecording = true;

    // 写文件头
    result = avformat_write_header(mRecorder->m_FormatCtx, nullptr);
    if (result < 0) {
        LOGCATE("CameraVideoRecorder::_StartRecordLoop Error occurred when opening output file: %s",
                av_err2str(result))
        goto __EXIT;
    }

    // 音频或者视频录制未结束
    while (mRecorder->m_IsRecording) {
        double videoTimestamp = mRecorder->m_VideoEncoder->GetCurrentTimestamp();
        double audioTimestamp = mRecorder->m_AudioEncoder->GetCurrentTimestamp();
        if (videoTimestamp < audioTimestamp) {
            mRecorder->m_VideoEncoder->EncodeFrame();
        } else {
            mRecorder->m_AudioEncoder->EncodeFrame();
        }
    }

    // 写文件尾
    av_write_trailer(mRecorder->m_FormatCtx);
    LOGCATE("CameraVideoRecorder::_StartRecordLoop record finish, save as: %s",
            mRecorder->m_FilePath)
    avio_close(mRecorder->m_FormatCtx->pb);

    __EXIT:
    mRecorder->_RealStopRecord();
    return nullptr;
}


void CameraVideoRecorder::StopRecord() {
    LOGCATE("CameraVideoRecorder::StopRecord() start")
    // 设置停止标志位并等待直到录制真正结束
    m_IsRecording = false;
    if (m_VideoEncoder) {
        m_VideoEncoder->Flush();
    }
    if (m_AudioEncoder) {
        m_AudioEncoder->Flush();
    }
    LOGCATE("CameraVideoRecorder::StopRecord() end")
}

void CameraVideoRecorder::_RealStopRecord() {
    if (m_VideoStream) {
        m_VideoStream = nullptr;
    }
    if (m_AudioStream) {
        m_AudioStream = nullptr;
    }
    if (m_VideoEncoder) {
        m_VideoEncoder.reset();
    }
    if (m_AudioEncoder) {
        m_AudioEncoder.reset();
    }
    if (m_FormatCtx) {
        avformat_free_context(m_FormatCtx);
        m_FormatCtx = nullptr;
    }
    LOGCATE("CameraVideoRecorder::_RealStopRecord()")
}

int CameraVideoRecorder::UnInit() {
    int result = 0;
    LOGCATE("CameraVideoRecorder::UnInit()")
    if (m_VideoRenderQueue) {
        m_VideoRenderQueue->overrule();
    }
    if (m_RenderWindow) {
        m_RenderWindow.reset();
    }
    if (m_VideoRenderQueue) {
        m_VideoRenderQueue.reset();
    }
    return result;
}

CameraVideoRecorder::~CameraVideoRecorder() {
    UnInit();
}
