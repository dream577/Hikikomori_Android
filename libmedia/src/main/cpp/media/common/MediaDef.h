//
// Created by bronyna on 2023/2/9.
//

#ifndef HIKIKOMORI_MEDIADEF_H
#define HIKIKOMORI_MEDIADEF_H

// 帧类型
#define FRAME_TYPE_AUDIO 0x00
#define FRAME_TYPE_VIDEO 0x01

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

enum PlayerState {
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

    AudioFrame(uint8_t *data, int dataSize, int channels, int sampleRate, long dts, long pts,
               int format) : Frame(dts, pts, FRAME_TYPE_AUDIO, format) {
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

    VideoFrame(uint8_t *yuvBuffer[3], int planeSize[3], int width, int height, long dts, long pts,
               int format)
            : Frame(dts, pts, FRAME_TYPE_VIDEO, format) {
        for (int i = 0; i < 3; i++) {
            VideoFrame::yuvBuffer[i] = yuvBuffer[i];
            VideoFrame::planeSize[i] = planeSize[i];
        }
        VideoFrame::width = width;
        VideoFrame::height = height;
    }

    virtual ~VideoFrame() {
        for (int i = 0; i < 3; i++) {
            if (yuvBuffer[i] != nullptr) {
                delete yuvBuffer[i];
                yuvBuffer[i] = nullptr;
            }
        }
    }
};

#endif //HIKIKOMORI_MEDIADEF_H
