//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_FFMEDIAINPUTENGINE_H
#define HIKIKOMORI_FFMEDIAINPUTENGINE_H

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/frame.h"
#include "libavutil/time.h"
}

#include "InputEngine.h"
#include "FFVideoDecoder.h"
#include "FFAudioDecoder.h"

using namespace std;

class FFMediaInputEngine : public InputEngine {
private:

    char m_Path[MAX_PATH_LENGTH] = {0};               // 文件地址
    float m_SeekPosition = -1;                        // seek position

    DecoderCallback *m_DecoderCallback;
    shared_ptr<MediaEventCallback> m_EventCallback;

    AVFormatContext *m_AVFormatContext = nullptr;     // 封装格式上下文

    /*
     * Video相关
     */
    shared_ptr<FFBaseDecoder> m_VideoCodec;
    int m_VideoStreamIndex;

    /*
     * Audio相关
     */
    shared_ptr<FFBaseDecoder> m_AudioCodec;
    int m_AudioStreamIndex;

    AVPacket *m_Pkt;

protected:

    int _Init() override;

    virtual int _UnInit() override;

    virtual void _DecodeLoop() override;

    virtual void _SeekToPosition(float timestamp) override;

    virtual int DecoderLoopOnce();

public:

    FFMediaInputEngine(const char *path, shared_ptr<MediaEventCallback> event_cb, DecoderCallback *decoder_cb);

    virtual ~FFMediaInputEngine();
};

#endif //HIKIKOMORI_FFMEDIAINPUTENGINE_H
