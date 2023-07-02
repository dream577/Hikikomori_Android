//
// Created by bronyna on 2023/2/9.
//

#include "stdint.h"

#ifndef HIKIKOMORI_MEDIADEF_H
#define HIKIKOMORI_MEDIADEF_H

#define MEDIA_TYPE_VIDEO 0x00
#define MEDIA_TYPE_AUDIO 0x01

// 视频帧类型
#define VIDEO_FRAME_FORMAT_RGBA           0x01
#define VIDEO_FRAME_FORMAT_NV21           0x02
#define VIDEO_FRAME_FORMAT_NV12           0x03
#define VIDEO_FRAME_FORMAT_I420           0x04

// 渲染器类型
#define VIDEO_RENDER_UNKONWN   -1
#define VIDEO_RENDER_ANWINDOW   0
#define VIDEO_RENDER_OPENGL     1
#define VIDEO_RENDER_3D_VR      2

// 帧队列大小
#define MAX_AUDIO_QUEUE_SIZE   5
#define MAX_VIDEO_QUEUE_SIZE   5

// 回调事件
#define EVENT_DURATION         0
#define EVENT_SHOW_LOADING     1
#define EVENT_HIDE_LOADING     2
#define EVENT_PLAYING          3
#define EVENT_EXCEPTION        4
#define EVENT_PAUSED           5
#define EVENT_SEEK_FINISH      6

enum PlayerState {
    STATE_ERROR = -1,
    STATE_UNKNOWN = 0,
    STATE_PLAYING,
    STATE_PAUSE,
    STATE_STOP
};

class Frame {
public:
    long dts;
    long pts;
    int type;
    int format;

    Frame() {};

    Frame(long dts, long pts, int type, int format) {
        Frame::dts = dts;
        Frame::pts = pts;
        Frame::type = type;
        Frame::format = format;
    }

    virtual ~Frame() {}
};

class AudioFrame : public Frame {
public:
    uint8_t *data;
    int dataSize;
    int channels;
    int sampleRate;

    AudioFrame() : Frame() { type = MEDIA_TYPE_AUDIO; }

    AudioFrame(uint8_t *data, int dataSize, int channels, int sampleRate, long dts, long pts,
               int format) : Frame(dts, pts, MEDIA_TYPE_AUDIO, format) {
        AudioFrame::data = data;
        AudioFrame::dataSize = dataSize;
        AudioFrame::channels = channels;
        AudioFrame::sampleRate = sampleRate;
    }

    virtual ~AudioFrame() {
        if (data != nullptr) {
            delete data;
            data = nullptr;
        }
    }
};

class VideoFrame : public Frame {
public:
    uint8_t *yuvBuffer[3];
    int planeSize[3];
    int width;
    int height;

    VideoFrame() : Frame() { type = MEDIA_TYPE_VIDEO; };

    VideoFrame(uint8_t *yuvBuffer[3], int planeSize[3], int width, int height, long dts, long pts,
               int format)
            : Frame(dts, pts, MEDIA_TYPE_VIDEO, format) {
        for (int i = 0; i < 3; i++) {
            VideoFrame::yuvBuffer[i] = yuvBuffer[i];
            VideoFrame::planeSize[i] = planeSize[i];
        }
        VideoFrame::width = width;
        VideoFrame::height = height;
    }

    virtual ~VideoFrame() {
        if (yuvBuffer[0]) {
            delete yuvBuffer[0];
        }
        yuvBuffer[0] = nullptr;
        yuvBuffer[1] = nullptr;
        yuvBuffer[2] = nullptr;
    }
};

struct TransformMatrix {
    int degree;
    int mirror;
    float translateX;
    float translateY;
    float scaleX;
    float scaleY;
    int angleX;
    int angleY;

    TransformMatrix() : translateX(0), translateY(0), scaleX(1.0), scaleY(1.0),
                        degree(0), mirror(0), angleX(0), angleY(0) {}

    void Reset() {
        translateX = 0;
        translateY = 0;
        scaleX = 1.0;
        scaleY = 1.0;
        degree = 0;
        mirror = 0;
    }
};

#endif //HIKIKOMORI_MEDIADEF_H
