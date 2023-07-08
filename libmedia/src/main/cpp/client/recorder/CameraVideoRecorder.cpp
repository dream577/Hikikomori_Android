//
// Created by bronyna on 2023/2/25.
//

#include "CameraVideoRecorder.h"
#include "LogUtil.h"

CameraVideoRecorder::CameraVideoRecorder() {
    LOGCATE("CameraVideoRecorder::CameraVideoRecorder");
    m_RenderWindow = new GLRenderWindow(this);
    m_VideoFrameQueue = new ThreadSafeQueue(MAX_AUDIO_QUEUE_SIZE, MEDIA_TYPE_VIDEO);
    m_AudioFrameQueue = new ThreadSafeQueue(MAX_AUDIO_QUEUE_SIZE, MEDIA_TYPE_AUDIO);
    m_RenderWindow->StartRender();

    m_AudioEncoder = new AudioEncoder();
    m_AudioEncoder->StartEncode();
}

void CameraVideoRecorder::UnInit() {
    LOGCATE("CameraVideoRecorder::UnInit");
    if (m_AudioFrameQueue) {
        m_AudioFrameQueue->abort();
    }

    if (m_VideoFrameQueue) {
        m_VideoFrameQueue->abort();
    }

    if (m_RenderWindow) {
        m_RenderWindow->Destroy();
        delete m_RenderWindow;
        m_RenderWindow = nullptr;
    }

    if (m_AudioFrameQueue) {
        delete m_AudioFrameQueue;
        m_AudioFrameQueue = nullptr;
    }

    if (m_VideoFrameQueue) {
        delete m_VideoFrameQueue;
        m_VideoFrameQueue = nullptr;
    }
}

CameraVideoRecorder::~CameraVideoRecorder() = default;

void CameraVideoRecorder::StartRecord() {
    LOGCATE("CameraVideoRecorder::StartRecord");
}

void CameraVideoRecorder::StopRecord() {
    LOGCATE("CameraVideoRecorder::StopRecord");
}

void CameraVideoRecorder::OnDrawPreviewFrame(uint8_t *data, int width, int height, int format,
                                             long timestamp) {
//    LOGCATE("CameraVideoRecorder::OnDrawPreviewFrame")
    VideoFrame *frame = nullptr;
    switch (format) {
        case VIDEO_FRAME_FORMAT_RGBA:
            frame = new VideoFrame();

            frame->format = VIDEO_FRAME_FORMAT_RGBA;
            frame->width = width;
            frame->height = height;
            frame->yuvBuffer[0] = data;
            frame->planeSize[0] = width * 4;
            frame->pts = timestamp;
            break;
        case VIDEO_FRAME_FORMAT_I420:
            frame = new VideoFrame();
            int yPlaneByteSize = width * height;
            int uvPlaneByteSize = yPlaneByteSize / 2;

            frame->format = VIDEO_FRAME_FORMAT_I420;
            frame->width = width;
            frame->height = height;
            frame->yuvBuffer[0] = data;
            frame->yuvBuffer[1] = data + yPlaneByteSize;
            frame->yuvBuffer[2] = data + yPlaneByteSize + uvPlaneByteSize / 2;
            frame->planeSize[0] = width;
            frame->planeSize[1] = width / 2;
            frame->planeSize[2] = width / 2;
            frame->pts = timestamp;
            break;
    }

    m_VideoFrameQueue->offer(frame);
}

Frame *CameraVideoRecorder::GetOneFrame(int type) {
    LOGCATE("CameraVideoRecorder::GetOneFrame");
    Frame *frame;
    if (type == MEDIA_TYPE_VIDEO) {
        frame = m_VideoFrameQueue->poll();
    } else {
        frame = m_AudioFrameQueue->poll();
    }
    return frame;
}
