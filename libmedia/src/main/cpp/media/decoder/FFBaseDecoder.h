//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_FFBASEDECODER_H
#define HIKIKOMORI_FFBASEDECODER_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/frame.h"
#include "libavutil/time.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
};

#include "Decoder.h"

#include <thread>

using namespace std;

class FFBaseDecoder : public Decoder {
private:
    virtual int decodeLoopOnce();

//    thread *m_thread = nullptr;

protected:
    AVFormatContext *m_AVFormatContext = nullptr;     // 封装格式上下文

    AVCodecContext *m_AVCodecContext = nullptr;       // 编解码器上下文

    AVCodec *m_AVCodec = nullptr;                     // 解码器

    AVPacket *m_AVPacket = nullptr;                   // 编码的数据包

    AVFrame *m_AVFrame = nullptr;                     // 解码的帧

    int m_StreamIndex = -1;                           // 数据流索引

    char m_Path[MAX_PATH_LENGTH] = {0};               // 文件地址

    AVMediaType m_MediaType = AVMEDIA_TYPE_UNKNOWN;

    virtual void decodeLoop() override;

public:

    FFBaseDecoder(const char *path, AVMediaType type, DecoderCallback *callback) : Decoder(
            callback) {
        strcpy(m_Path, path);
        m_MediaType = type;
    }

    AVCodecContext *GetAVCodecContext() {
        return m_AVCodecContext;
    }

    virtual ~FFBaseDecoder();

    virtual int Init() override;

    virtual int UnInit() override;

    virtual void seekPosition(float timestamp) override;

//    virtual void StartDecoderThread() override {
//        m_thread = new std::thread(startDecode, this);
//    }
//
//    static void startDecode(FFBaseDecoder *decoder) {
//        decoder->decodeLoop();
//    }

};


#endif //HIKIKOMORI_FFBASEDECODER_H
