//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_AVINPUTENGINE_H
#define HIKIKOMORI_AVINPUTENGINE_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/frame.h"
#include "libavutil/time.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
}

#include "Decoder.h"

using namespace std;

#define AUDIO_DST_SAMPLE_RATE       44100  // 音频编码采样率
#define AUDIO_DST_CHANNEL_COUNTS    2      // 音频编码通道数
#define AUDIO_DST_CHANNEL_LAYOUT    AV_CH_LAYOUT_STEREO  // 音频编码声道格式
#define AUDIO_DST_BIT_RATE          64000  // 音频编码比特率
#define ACC_NB_SAMPLES              1024   // ACC音频一帧采样数

class AVInputStream {
public:
    AVFormatContext *fc;
    AVCodecContext *c;
    int stream_index;

    AVInputStream() {
        fc = nullptr;
        c = nullptr;
        stream_index = -1;
    }

    ~AVInputStream() {
        fc = nullptr;
        if (c) {
            avcodec_close(c);
            avcodec_free_context(&c);
            c = nullptr;
        }
        stream_index = -1;
    }
};

class AVInputEngine : public Decoder {
private:
    virtual int decodeLoopOnce();

    float m_SeekPosition = -1;               // seek position

protected:
    AVFormatContext *m_AVFormatContext = nullptr;     // 封装格式上下文

    AVInputStream *m_AudioIst;
    AVInputStream *m_VideoIst;

    SwrContext *m_SwrContext;    // audio resample context
    int m_nbSamples = 0;         // number of sample per channel
    int m_DstFrameDataSize = 0;  // dst frame data size
    uint8_t *m_AudioOutBuffer = nullptr;

    AVFrame *m_Frame;
    AVPacket *m_Pkt;

    char m_Path[MAX_PATH_LENGTH] = {0};               // 文件地址

    bool video_seek_finish = false;
    bool audio_seek_finish = false;

    int _init();

    virtual int _unInit() override;

    virtual void _decodeLoop() override;

    virtual void _seekPosition(float timestamp) override;

    int InitAudioSwrContext();

    Frame *AudioFrameAvailable();

    Frame *VideoFrameAvailable();

    int OpenDecoder(AVInputStream *_Ist);

public:

    AVInputEngine(const char *path, MediaEventCallback *event_cb,
                  DecoderCallback *decoder_cb) : Decoder(decoder_cb, event_cb) {
        strcpy(m_Path, path);
        m_VideoIst = nullptr;
        m_AudioIst = nullptr;
    }

    virtual ~AVInputEngine() {
        LOGCATE("AVInputEngine::~AVInputEngine")
    }

};


#endif //HIKIKOMORI_AVINPUTENGINE_H
