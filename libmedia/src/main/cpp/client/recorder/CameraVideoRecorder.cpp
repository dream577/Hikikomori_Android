//
// Created by bronyna on 2023/2/25.
//

#include "CameraVideoRecorder.h"

CameraVideoRecorder::CameraVideoRecorder() {
    LOGCATE("CameraVideoRecorder::CameraVideoRecorder")
    m_FormatCtx = nullptr;
    m_VideoRenderQueue = make_shared<LinkedBlockingQueue<MediaFrame>>(60);
    m_RenderWindow = make_shared<GLRenderWindow>(this);
    m_RenderWindow->StartRenderLoop();

    m_EnableAudio = false;
    m_EnableVideo = false;
    m_RecordModeExit = true;
}

int CameraVideoRecorder::Init() {
    LOGCATE("CameraVideoRecorder::Init()")
    int result;

    // 申请AVFormatContext，主要是在进行封装格式相关的操作时作为操作上下文的线索
    result = avformat_alloc_output_context2(&m_FormatCtx, nullptr, nullptr, m_FileName);
    if (result < 0) {
        result = avformat_alloc_output_context2(&m_FormatCtx, nullptr, "mp4", m_FileName);
    }
    if (result < 0) {
        LOGCATE("CameraVideoRecorder::Init() avformat_alloc_output_context2 ret=%d", result)
        goto __EXIT;
    }

    if (m_EnableAudio) {
        m_AudioEncoder = make_shared<FFAudioEncoder>(AV_CODEC_ID_AAC, m_SampleRate,
                                                     m_ChannelLayout, 96000);
        m_AudioEncoder->SetEncodeCallback(this);
        // 申请存放音频的AVStream
        m_AudioStream = avformat_new_stream(m_FormatCtx, nullptr);
        if (m_AudioStream) {
            m_AudioStream->id = (int) (m_FormatCtx->nb_streams) - 1;
            m_AudioStream->time_base = AVRational{1, m_SampleRate};
            result = m_AudioEncoder->OpenCodec(m_AudioStream->codecpar);
        } else {
            m_EnableAudio = false;
            LOGCATE("CameraVideoRecorder::Init() open audio encoder error")
        }
    }

    if (m_EnableVideo) {
        m_VideoEncoder = make_shared<FFVideoEncoder>(AV_CODEC_ID_H264, m_ImageWidth,
                                                     m_ImageHeight, m_FrameRate,
                                                     m_VideoBitRate);
        m_VideoEncoder->SetEncodeCallback(this);
        // 申请存放视频的AVStream
        m_VideoStream = avformat_new_stream(m_FormatCtx, nullptr);

        if (m_VideoStream) {
            m_VideoStream->id = (int) (m_FormatCtx->nb_streams) - 1;
            m_VideoStream->time_base = AVRational{1, m_FrameRate};
            result = m_VideoEncoder->OpenCodec(m_VideoStream->codecpar);
        } else {
            m_EnableVideo = false;
            LOGCATE("CameraVideoRecorder::Init() open video encoder error")
        }
    }

    if (!m_EnableVideo && !m_EnableAudio) {
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
    __EXIT:
    return result;
}

void CameraVideoRecorder::StartRecord() {
    LOGCATE("CameraVideoRecorder::StartRecord()")
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&worker, &attr, StartRecordLoop, this);
}

void CameraVideoRecorder::InputVideoFrame(uint8_t *data, int width, int height, int format,
                                          long timestamp) {
//    LOGCATE("CameraVideoRecorder::InputVideoFrame")
    shared_ptr<MediaFrame> frame;
    switch (format) {
        case IMAGE_FORMAT_I420: {
            frame = make_shared<MediaFrame>();
            int yPlaneByteSize = width * height;
            int uvPlaneByteSize = yPlaneByteSize / 2;

            frame->format = IMAGE_FORMAT_I420;
            frame->width = width;
            frame->height = height;
            frame->plane[0] = data;
            frame->plane[1] = data + yPlaneByteSize;
            frame->plane[2] = data + yPlaneByteSize + uvPlaneByteSize / 2;
            frame->planeSize[0] = width;
            frame->planeSize[1] = width / 2;
            frame->planeSize[2] = width / 2;
            frame->pts = timestamp;
            break;
        }
        case IMAGE_FORMAT_NV12:
        case IMAGE_FORMAT_NV21:
            break;

        default: {
            // 默认为 IMAGE_FORMAT_RGBA
            frame = make_shared<MediaFrame>();

            frame->format = IMAGE_FORMAT_RGBA;
            frame->width = width;
            frame->height = height;
            frame->plane[0] = data;
            frame->planeSize[0] = width * 4;
            frame->pts = timestamp;
            break;
        }
    }

    frame->type = AVMEDIA_TYPE_VIDEO;
    if (m_IsCameraRecorder) {
        m_VideoRenderQueue->offer(frame);
    } else {
        m_VideoEncoder->InputFrame(frame);
    }
}

void CameraVideoRecorder::InputAudioFrame(uint8_t *data, int size, long timestamp,
                                          int sample_rate, int sample_format, int channel_layout) {
    if (!m_IsAudioRecording) {
        delete data;
        return;
    }
    auto *frame = new MediaFrame();
    frame->plane[0] = data;
    frame->planeSize[0] = size;
    frame->sampleRate = sample_rate;
    frame->sampleFormat = sample_format;
    frame->channelLayout = channel_layout;
    frame->pts = timestamp;
    frame->type = AVMEDIA_TYPE_AUDIO;

    int ret = m_AudioEncoder->InputFrame(shared_ptr<MediaFrame>(frame));
    if (ret < 0) {
        delete frame;
    }
}


shared_ptr<MediaFrame> CameraVideoRecorder::GetOneFrame(int type) {
//    LOGCATE("CameraVideoRecorder::GetOneFrame")
    return m_VideoRenderQueue->poll();
}

void CameraVideoRecorder::FrameRendFinish(shared_ptr<MediaFrame> frame) {
//    LOGCATE("CameraVideoRecorder::FrameRendFinish")
    if (frame && frame->type == AVMEDIA_TYPE_VIDEO) {
        if (m_IsVideoRecording) {
            m_VideoEncoder->InputFrame(shared_ptr<MediaFrame>(frame));
        }
    }
}

void CameraVideoRecorder::OnFrameEncoded(AVPacket *pkt, AVMediaType type) {
    if (type == AVMEDIA_TYPE_VIDEO) {
        pkt->stream_index = m_VideoStream->index;

        AVRational *time_base = &m_VideoStream->time_base;
        av_packet_rescale_ts(pkt, *time_base, *time_base);
        LOGCATE("CameraVideoRecorder::OnFrameEncoded pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s",
                av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
                av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
                av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base))
    } else {
        pkt->stream_index = m_AudioStream->index;
        AVRational *time_base = &m_AudioStream->time_base;
        av_packet_rescale_ts(pkt, *time_base, *time_base);
        LOGCATE("CameraVideoRecorder::OnFrameEncoded pkt pts=%ld, size=%d", pkt->pts, pkt->size)
    }
    av_interleaved_write_frame(m_FormatCtx, pkt);
}

void *CameraVideoRecorder::StartRecordLoop(void *recorder) {
    auto *mRecorder = (CameraVideoRecorder *) recorder;
    int result;
    mRecorder->m_RecordModeExit = false;

    // 根据是否启用音频录制、视频录制来设置是否正在录制的标志位
    mRecorder->m_IsVideoRecording = mRecorder->m_EnableVideo;
    mRecorder->m_IsAudioRecording = mRecorder->m_EnableAudio;
    LOGCATE("CameraVideoRecorder::StartRecordLoop  %d, %d", mRecorder->m_IsAudioRecording,
            mRecorder->m_IsVideoRecording)

    if (!mRecorder->m_IsVideoRecording && !mRecorder->m_IsAudioRecording) {
        goto __EXIT;
    }

    // 写文件头
    result = avformat_write_header(mRecorder->m_FormatCtx, nullptr);
    if (result < 0) {
        LOGCATE("CameraVideoRecorder::StartRecordLoop Error occurred when opening output file: %s",
                av_err2str(result))
        goto __EXIT;
    }

    // 音频或者视频录制未结束
    while (mRecorder->m_IsAudioRecording || mRecorder->m_IsVideoRecording) {
        if (mRecorder->m_IsAudioRecording && mRecorder->m_IsVideoRecording) {
            double videoTimestamp = mRecorder->m_VideoEncoder->GetCurrentTimestamp();
            double audioTimestamp = mRecorder->m_AudioEncoder->GetCurrentTimestamp();
            if (videoTimestamp < audioTimestamp) {
                mRecorder->m_VideoEncoder->EncodeFrame();
            } else {
                mRecorder->m_AudioEncoder->EncodeFrame();
            }
        } else if (mRecorder->m_IsAudioRecording) {
            mRecorder->m_AudioEncoder->EncodeFrame();
        } else {
            mRecorder->m_VideoEncoder->EncodeFrame();
        }
    }

    // 写文件尾
    av_write_trailer(mRecorder->m_FormatCtx);
    LOGCATE("CameraVideoRecorder::StartRecordLoop record finish, save as: %s",
            mRecorder->m_FilePath)
    avio_close(mRecorder->m_FormatCtx->pb);

    __EXIT:
    mRecorder->RealStopRecord();
    mRecorder->m_RecordModeExit = true;
    return nullptr;
}


void CameraVideoRecorder::StopRecord() {
    LOGCATE("CameraVideoRecorder::StopRecord() start")
    // 设置停止标志位并等待直到录制真正结束
    m_IsVideoRecording = false;
    m_IsAudioRecording = false;
    if (m_VideoEncoder) {
        m_VideoEncoder->Flush();
    }
    if (m_AudioEncoder) {
        m_AudioEncoder->Flush();
    }
    while (!m_RecordModeExit) {
        av_usleep(1000 * 10);
    }
    LOGCATE("CameraVideoRecorder::StopRecord() end")
}

void CameraVideoRecorder::RealStopRecord() {
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
    // 重置标志位
    m_EnableVideo = false;
    m_EnableAudio = false;
    LOGCATE("CameraVideoRecorder::RealStopRecord()")
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
