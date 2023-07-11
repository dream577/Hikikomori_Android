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
    AVCodecContext *cc;
    const AVCodec *c;
    int stream_index;

    AVBufferRef *hw_ctx;
    AVPixelFormat pix_fmt;

    AVInputStream() {
        fc = nullptr;
        cc = nullptr;
        c = nullptr;
        hw_ctx = nullptr;
        stream_index = -1;
    }

    ~AVInputStream() {
        fc = nullptr;
        c = nullptr;
        if (cc) {
            avcodec_close(cc);
            avcodec_free_context(&cc);
            cc = nullptr;
        }
        if (hw_ctx) {
            av_buffer_unref(&hw_ctx);
            hw_ctx = nullptr;
        }
        stream_index = -1;
    }
};

class AVInputEngine : public Decoder {
private:

    char m_Path[MAX_PATH_LENGTH] = {0};               // 文件地址
    float m_SeekPosition = -1;                        // seek position

    AVFormatContext *m_AVFormatContext = nullptr;     // 封装格式上下文

    /*
     * Video相关
     */
    AVInputStream *m_VideoIst;
    bool m_VideoSeekFinish;
    bool m_AudioEnable;

    /*
     * Audio相关
     */
    AVInputStream *m_AudioIst;
    SwrContext *m_SwrContext;    // audio resample context
    int m_nbSamples = 0;         // number of sample per channel
    int m_DstFrameDataSize = 0;  // dst frame data size
    uint8_t *m_AudioOutBuffer;
    bool m_AudioSeekFinish;
    bool m_VideoEnable;

    AVFrame *m_Frame;
    AVPacket *m_Pkt;

protected:

    int _Init();

    virtual int _UnInit() override;

    virtual void _DecoderLoop() override;

    virtual void _SeekPosition(float timestamp) override;

    virtual int DecoderLoopOnce();

    int InitAudioSwrContext();

    Frame *AudioFrameAvailable();

    Frame *VideoFrameAvailable();

    int OpenCodec(AVInputStream *ist);

    int FindCodecAndAllocCtx(AVInputStream *ist);

public:

    AVInputEngine(const char *path, MediaEventCallback *event_cb,
                  DecoderCallback *decoder_cb) : Decoder(decoder_cb, event_cb) {
        strcpy(m_Path, path);
        m_VideoIst = nullptr;
        m_VideoEnable = false;
        m_VideoSeekFinish = false;

        m_AudioIst = nullptr;
        m_SwrContext = nullptr;
        m_AudioOutBuffer = nullptr;
        m_AudioSeekFinish = false;
        m_AudioEnable = false;

        m_Frame = nullptr;
        m_Pkt = nullptr;
    }

    virtual ~AVInputEngine() {
        LOGCATE("AVInputEngine::~AVInputEngine")
    }
};

static enum AVPixelFormat hw_pix_fmt;

static int HwDecoderInit(AVInputStream *ist, const enum AVHWDeviceType type);

static enum AVPixelFormat GetHwFormat(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);


#endif //HIKIKOMORI_AVINPUTENGINE_H
