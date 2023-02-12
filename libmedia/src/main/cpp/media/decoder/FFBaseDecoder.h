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
    thread *m_Thread = nullptr;

protected:

    AVFormatContext *m_AVFormatContext = nullptr;     // 封装格式上下文

    AVCodecContext *m_AVCodecContext = nullptr;       // 编解码器上下文

    AVCodec *m_AVCodec = nullptr;                     // 解码器

    AVPacket *m_AVPacket = nullptr;                   // 编码的数据包

    AVFrame *m_AVFrame = nullptr;                     // 解码的帧

    int m_StreamIndex = -1;                           // 数据流索引

    char m_Path[MAX_PATH_LENGTH] = {0};               // 文件地址

    AVMediaType m_MediaType = AVMEDIA_TYPE_UNKNOWN;

    volatile float m_SeekPosition = -1;                         // seek position

    static void DoAVDecoding(FFBaseDecoder *decode);

    virtual int decode() override;

    virtual int unInit() override;

public:

    FFBaseDecoder(const char *path, AVMediaType type, DecoderCallback *callback)
            : Decoder(type, callback) {
        strcpy(m_Path, path);
        m_MediaType = type;
    }

    virtual ~FFBaseDecoder();

    virtual int init() override;

    virtual int destroy() override;

    virtual void startDecodeThread() override;

    virtual void seekPosition(float timestamp) override;

    virtual void decodingLoop() override;

};


#endif //HIKIKOMORI_FFBASEDECODER_H
