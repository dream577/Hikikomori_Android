//
// Created by bronyna on 2023/2/25.
//

#include "CameraVideoRecorder.h"
#include "LogUtil.h"

CameraVideoRecorder::CameraVideoRecorder() {
    LOGCATE("CameraVideoRecorder::CameraVideoRecorder");
    mVideoRender = new VideoGLRender(this);
    mVideoFrameQueue = new ThreadSafeQueue(MAX_AUDIO_QUEUE_SIZE, MEDIA_TYPE_VIDEO);
    mAudioFrameQueue = new ThreadSafeQueue(MAX_AUDIO_QUEUE_SIZE, MEDIA_TYPE_AUDIO);
    mVideoRender->StartRenderLoop();
}

void CameraVideoRecorder::UnInit() {
    LOGCATE("CameraVideoRecorder::UnInit");
    if (mAudioFrameQueue) {
        mAudioFrameQueue->abort();
    }

    if (mVideoFrameQueue) {
        mVideoFrameQueue->abort();
    }

    if (mVideoRender) {
        mVideoRender->UnInit();
        delete mVideoRender;
        mVideoRender = nullptr;
    }

    if (mAudioFrameQueue) {
        delete mAudioFrameQueue;
        mAudioFrameQueue = nullptr;
    }

    if (mVideoFrameQueue) {
        delete mVideoFrameQueue;
        mVideoFrameQueue = nullptr;
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

    mVideoFrameQueue->offer(frame);
}

Frame *CameraVideoRecorder::GetOneFrame(int type) {
    LOGCATE("CameraVideoRecorder::GetOneFrame");
    Frame *frame;
    if (type == MEDIA_TYPE_VIDEO) {
        frame = mVideoFrameQueue->poll();
    } else {
        frame = mAudioFrameQueue->poll();
    }
    return frame;
}
