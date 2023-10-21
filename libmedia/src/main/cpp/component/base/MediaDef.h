//
// Created by bronyna on 2023/2/9.
//

#include "stdint.h"
#include "CustomContainer.h"

#ifndef HIKIKOMORI_MEDIADEF_H
#define HIKIKOMORI_MEDIADEF_H

// 视频帧类型
#define  IMAGE_FORMAT_RGBA     0x01
#define  IMAGE_FORMAT_NV21     0x02
#define  IMAGE_FORMAT_NV12     0x03
#define  IMAGE_FORMAT_I420     0x04

// 渲染器类型
#define VIDEO_RENDER_UNKONWN   -1
#define VIDEO_RENDER_ANWINDOW   0
#define VIDEO_RENDER_OPENGL     1
#define VIDEO_RENDER_3D_VR      2

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

enum FrameFlag {
    FLAG_NONE = -1,
    FLAG_SEEK_FINISH,  // seek操作结束后的第一帧
    FLAG_RECORD_END,   // 录制的最后一帧
};

class MediaFrame {
public:
    /**
     * common
     */
    uint8_t *plane[3];
    int planeSize[3];
    long dts;
    long pts;
    int type;  // video/audio

    /**
     * video
     */
    int format;
    int width;
    int height;

    /**
     * audio
     */
    int channels;
    int sampleRate;
    int sampleFormat;
    int channelLayout;

    int flag = FLAG_NONE;

    std::weak_ptr<LinkedBlockingQueue<MediaFrame>> pool;

    MediaFrame() {
        plane[0] = plane[1] = plane[2] = nullptr;
        planeSize[0] = planeSize[1] = planeSize[2] = 0;
    };

    static void recycle(shared_ptr<MediaFrame> frame) {
        auto queue = frame->pool.lock();
        if (queue) {
            queue->offer(frame);
        }
    }

    virtual ~MediaFrame() {
        if (plane[0]) {
            delete plane[0];
        }
        plane[0] = nullptr;
        plane[1] = nullptr;
        plane[2] = nullptr;
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
