//
// Created by bronyna on 2023/2/9.
//

extern "C" {
#include "libavutil/avutil.h"
}

#include <jni.h>
#include "stdint.h"
#include "LinkedBlockingQueue.h"

#ifndef HIKIKOMORI_MEDIADEF_H
#define HIKIKOMORI_MEDIADEF_H

// 回调事件
#define EVENT_DURATION         0
#define EVENT_SHOW_LOADING     1
#define EVENT_HIDE_LOADING     2
#define EVENT_PLAYING          3
#define EVENT_EXCEPTION        4
#define EVENT_PAUSED           5
#define EVENT_SEEK_FINISH      6

enum AVMediaMessage {
    MESSAGE_INIT = 0,
    MESSAGE_LOOP,
    MESSAGE_SEEK,
    MESSAGE_UNINIT,
};

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

class AVMediaFrame {
public:
    uint8_t *plane[3];
    int planeSize[3];
    long dts;
    long pts;
    int type;  // 可选值：AVMEDIA_TYPE_VIDEO, AVMEDIA_TYPE_AUDIO

    // video相关
    int format; // 可选值：AV_PIX_FMT_RGBA，AV_PIX_FMT_NV21，AV_PIX_FMT_NV12，AV_PIX_FMT_YUV420P
    int width;
    int height;

    // audio相关
    int channels;
    int sampleRate;
    int sampleFormat;
    int channelLayout;

    int flag = FLAG_NONE;

    std::weak_ptr<LinkedBlockingQueue<AVMediaFrame>> pool;

    AVMediaFrame() {
        plane[0] = plane[1] = plane[2] = nullptr;
        planeSize[0] = planeSize[1] = planeSize[2] = 0;
    };

    static void recycle(shared_ptr<AVMediaFrame> frame) {
        auto queue = frame->pool.lock();
        if (queue) {
            queue->offer(frame);
        }
    }

    virtual ~AVMediaFrame() {
        if (plane[0]) {
            delete plane[0];
        }
        plane[0] = nullptr;
        plane[1] = nullptr;
        plane[2] = nullptr;
    }
};

struct AVTransformMatrix {
    int degree;
    int mirror;
    float translateX;
    float translateY;
    float scaleX;
    float scaleY;
    int angleX;
    int angleY;

    AVTransformMatrix() : translateX(0), translateY(0), scaleX(1.0), scaleY(1.0),
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

class AVSurface {
public:
    virtual void OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) = 0;

    virtual void OnSurfaceChanged(int width, int height) = 0;

    virtual void
    UpdateMVPMatrix(float translateX, float translateY, float scaleX, float scaleY, int degree,
                    int mirror) = 0;

    virtual void OnSurfaceDestroyed() = 0;
};

class AVRenderCallback {
public:
    virtual std::shared_ptr<AVMediaFrame> GetOneFrame(int type) = 0;

    virtual void FrameRendFinish(std::shared_ptr<AVMediaFrame> frame) = 0;
};

class AVDecoderCallback {
public:
    virtual void OnFrameReady(std::shared_ptr<AVMediaFrame> frame) = 0;

    virtual int GetPlayerState() = 0;

    virtual void SetPlayerState(PlayerState state) = 0;
};

#endif //HIKIKOMORI_MEDIADEF_H
