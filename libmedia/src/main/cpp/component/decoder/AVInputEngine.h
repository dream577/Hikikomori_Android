//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_AVINPUTENGINE_H
#define HIKIKOMORI_AVINPUTENGINE_H

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/frame.h"
#include "libavutil/time.h"
}

#include "InputEngine.h"
#include "VideoFFDecoder.h"
#include "AudioFFDecoder.h"

using namespace std;

class AVInputEngine : public InputEngine {
private:

    char m_Path[MAX_PATH_LENGTH] = {0};               // 文件地址
    float m_SeekPosition = -1;                        // seek position

    DecoderCallback *m_DecoderCallback;
    shared_ptr<MediaEventCallback> m_EventCallback;

    AVFormatContext *m_AVFormatContext = nullptr;     // 封装格式上下文
    int m_SeekFinish = 0x00;

    /*
     * Video相关
     */
    shared_ptr<VideoFFDecoder> m_VideoCodec;
    double m_VideoTimebase;
    int m_VideoStreamIndex;
    bool m_VideoEnable;

    /*
     * Audio相关
     */
    shared_ptr<AudioFFDecoder> m_AudioCodec;
    double m_AudioTimebase;
    int m_AudioStreamIndex;
    bool m_AudioEnable;

    AVPacket *m_Pkt;

protected:

    int _Init() override;

    virtual int _UnInit() override;

    virtual void _DecoderLoop() override;

    virtual void _SeekPosition(float timestamp) override;

    virtual int DecoderLoopOnce();

public:

    AVInputEngine(const char *path, shared_ptr<MediaEventCallback> event_cb, DecoderCallback *decoder_cb);

    virtual ~AVInputEngine();
};

#endif //HIKIKOMORI_AVINPUTENGINE_H
