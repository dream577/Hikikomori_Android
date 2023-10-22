//
// Created by 郝留凯 on 2023/10/21.
//

#ifndef HIKIKOMORI_FFBASEENCODER_H
#define HIKIKOMORI_FFBASEENCODER_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavcodec/codec.h"
#include "libavformat/avformat.h"
#include "libavutil/frame.h"
#include "libavutil/time.h"
#include "libavutil/opt.h"
#include "libavutil/avassert.h"
#include "libavutil/mathematics.h"
#include "libavutil/timestamp.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include "libavutil/log.h"
#include "libavformat/avio.h"
}

#include <stdio.h>
#include <pthread.h>

#include "MediaDef.h"
#include "LogUtil.h"

class EncoderCallback {
public:
    virtual void OnFrameEncoded(AVPacket *pkt, AVMediaType) = 0;
};

class FFBaseEncoder {
protected:
    /**
     * 描述编码器上下文的数据结构，包含了众多编码器需要的参数信息
     */
    AVCodecContext *m_CodecCtx = nullptr;

    /**
     * 编码器的ID
     */
    AVCodecID m_EncoderId;

    /**
     * 编码器
     */
    const AVCodec *m_Codec = nullptr;

    /**
     * 原始帧数据，对于音频来说是PCM数据，对于视频来说是RGB格式的数据
     */
    AVFrame *m_SrcFrame = nullptr;

    /**
     * 转换后的数据
     */
    AVFrame *m_DstFrame = nullptr;

    /**
     * 下一帧的pts
     */
    long m_NextPts = 0;

    /**
     * 用于存放原始数据的队列
     */
    shared_ptr<LinkedBlockingQueue<MediaFrame>> m_EncoderQueue;

    /**
     * 编码回调
     */
    EncoderCallback *callback = nullptr;

public:

    FFBaseEncoder(AVCodecID encodec_id) {
        this->m_EncoderQueue = make_shared<LinkedBlockingQueue<MediaFrame>>(INT_MAX);
        this->m_EncoderId = encodec_id;
    }

    /**
     * 打开编码器
     * @param param 不能为空，在打开编码器时会填充参数
     * @return
     */
    virtual int OpenCodec(AVCodecParameters *param) = 0;

    /**
     * 输入一帧数据到编码队列中
     * @param frame 原始音视频数据
     */
    int InputFrame(shared_ptr<MediaFrame> frame) {
        return m_EncoderQueue->offer(frame);
    }

    /**
     * 编码一帧数据
     * @return
     */
    virtual int EncodeFrame() = 0;

    /**
     * 在停止编码时调用，防止线程一直阻塞
     */
    void Flush() {
        if (m_EncoderQueue) {
            m_EncoderQueue->overrule();
        }
    }

    void SetEncodeCallback(EncoderCallback *callback) {
        this->callback = callback;
    }


    double GetCurrentTimestamp() {
        return m_NextPts * av_q2d(m_CodecCtx->time_base);
    }

    ~FFBaseEncoder() {
        callback = nullptr;
        if (m_EncoderQueue) {
            m_EncoderQueue->overrule();
            m_EncoderQueue.reset();
        }
        if (m_DstFrame) {
            av_frame_free(&m_DstFrame);
            m_DstFrame = nullptr;
        }
        if (m_SrcFrame) {
            av_frame_free(&m_SrcFrame);
            m_SrcFrame = nullptr;
        }
        if (m_CodecCtx) {
            avcodec_free_context(&m_CodecCtx);
            m_Codec = nullptr;
        }
    }
};

#endif //HIKIKOMORI_FFBASEENCODER_H
