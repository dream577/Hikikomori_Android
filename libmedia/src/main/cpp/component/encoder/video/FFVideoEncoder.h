//
// Created by 郝留凯 on 2023/10/21.
//

#ifndef HIKIKOMORI_FFVIDEOENCODER_H
#define HIKIKOMORI_FFVIDEOENCODER_H

#include "FFBaseEncoder.h"

class FFVideoEncoder : public FFBaseEncoder {
private:
    /**
    * 视频帧的宽和高
    */
    int m_ImageWidth, m_ImageHeight;

    /**
     * 视频帧率
     */
    int m_VideoFrameRate;

    /**
     * 视频码率
     */
    int64_t m_VideoBitRate;

    /**
     * 用于视频格式转换: RGB->NV12
     */
    SwsContext *m_SwsCtx = nullptr;
public:
    FFVideoEncoder(AVCodecID encoderId, int imageWidth, int imageHeight,
                   int videoFrameRate, int64_t videoBitRate);

    int OpenCodec(AVCodecParameters *parameters) override;

    int EncodeFrame() override;

    ~FFVideoEncoder();
};


#endif //HIKIKOMORI_FFVIDEOENCODER_H
