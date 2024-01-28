//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_FFMEDIAINPUTENGINE_H
#define HIKIKOMORI_FFMEDIAINPUTENGINE_H

#define MAX_PATH_LENGTH 2048

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/frame.h"
#include "libavutil/time.h"
}

#include "FFVideoDecoder.h"
#include "FFAudioDecoder.h"

using namespace std;

class FFMediaInputEngine : public looper {
private:
    int mLoopMsg = MESSAGE_LOOP;
    int result = -1;
    float mTempStamp = -1;
    sem_t runBlock;

    char m_Path[MAX_PATH_LENGTH] = {0};               // 文件地址
    float m_SeekPosition = -1;                        // seek position

    AVDecoderCallback *m_DecoderCallback;
    shared_ptr<MediaEventCallback> m_EventCallback;

    AVFormatContext *m_AVFormatContext = nullptr;     // 封装格式上下文
    shared_ptr<FFBaseDecoder> m_VideoCodec;
    shared_ptr<FFBaseDecoder> m_AudioCodec;
    int m_VideoStreamIndex;
    int m_AudioStreamIndex;

    AVPacket *m_Pkt;

    int _Init();

    virtual int _UnInit();

    virtual void _DecodeLoop();

    virtual void _SeekToPosition(float timestamp);

    virtual int _DecoderLoopOnce();

public:

    FFMediaInputEngine(const char *path, shared_ptr<MediaEventCallback> event_cb,
                       AVDecoderCallback *decoder_cb);

    int Init();

    int UnInit();

    int SeekPosition(float timestamp);

    void StartLoop();

    virtual void handle(int what, void *data) override;

    virtual ~FFMediaInputEngine();
};

#endif //HIKIKOMORI_FFMEDIAINPUTENGINE_H
