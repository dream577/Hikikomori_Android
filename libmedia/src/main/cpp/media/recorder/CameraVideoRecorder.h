//
// Created by bronyna on 2023/2/25.
//

#ifndef HIKIKOMORI_CAMERAVIDEORECORDER_H
#define HIKIKOMORI_CAMERAVIDEORECORDER_H

#include "Callback.h"
#include "GLRenderWindow.h"
#include "ThreadSafeQueue.h"

class CameraVideoRecorder : public RenderCallback {
private:

    GLRenderWindow *mRenderWindow;

    ThreadSafeQueue *mVideoFrameQueue;

    ThreadSafeQueue *mAudioFrameQueue;

public:
    CameraVideoRecorder();

    ~CameraVideoRecorder();

    void StartRecord();

    void StopRecord();

    void OnDrawPreviewFrame(uint8_t *data, int width, int height, int format, long timestamp);

    void UnInit();

    Frame *GetOneFrame(int type) override;

    GLRenderWindow *GetVideoRender() {
        return mRenderWindow;
    }

};


#endif //HIKIKOMORI_CAMERAVIDEORECORDER_H
