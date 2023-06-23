package com.violet.libmedia.codec.decoder

import com.violet.libmedia.demuxer.Demuxer
import com.violet.libmedia.model.MediaFrame
import com.violet.libmedia.util.RecycledPool.Element

interface Decoder {

    /**
     * 开始解码
     */
    fun startDecoder(path: String)


    fun prepareDemuxer(): Demuxer

    /**
     * 配置解码器, 请实现配置解码器的方法
     * @return 返回配置结果，后续可通过isConfigured方法获取配置状态
     */
    fun configureCodec(): Boolean

    /**
     * 解码器是否已配置
     */
    fun isConfigured(): Boolean

    /**
     * 从解码器输出一帧数据, 输出为空代表失败
     */
    fun outputOneFrame(): Element<MediaFrame>?

    /**
     * 释放解码器
     */
    fun release()
}