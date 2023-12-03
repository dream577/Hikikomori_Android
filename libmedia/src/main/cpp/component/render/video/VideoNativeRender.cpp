//
// Created by bronyna on 2023/2/7.
//

#include "VideoNativeRender.h"
#include "LogUtil.h"

void VideoNativeRender::onSurfaceCreated() {
    LOGCATE("VideoNativeRender::OnSurfaceCreated");

}

void VideoNativeRender::onSurfaceChanged(int width, int height) {
    if (m_NativeWindow == nullptr) return;
    mWindowWidth = width;
    mWindowHeight = height;

    if (mWindowWidth < mWindowHeight * mImageWidth / mImageHeight) {
        mRenderWidth = mWindowWidth;
        mRenderHeight = mWindowWidth * mImageHeight / mImageWidth;
    } else {
        mRenderWidth = mWindowHeight * mImageWidth / mImageHeight;
        mRenderHeight = mWindowHeight;
    }
    LOGCATE("VideoNativeRender::_OnSurfaceChanged m_NativeWindow=%p, video[w,h]=[%d, %d]",
            m_NativeWindow, mImageWidth, mImageHeight)

    LOGCATE("VideoNativeRender::_OnSurfaceChanged window[w,h]=[%d, %d],DstSize[w, h]=[%d, %d]",
            mWindowWidth, mWindowHeight, mRenderWidth, mRenderHeight);
    ANativeWindow_setBuffersGeometry(m_NativeWindow, mRenderWidth, mRenderHeight,
                                     WINDOW_FORMAT_RGBA_8888);

    m_RGBAFrame = av_frame_alloc();
    m_BufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, mRenderWidth, mRenderHeight, 1);

    m_FrameBuffer = (uint8_t *) av_malloc(m_BufferSize * sizeof(uint8_t));

    av_image_fill_arrays(m_RGBAFrame->data, m_RGBAFrame->linesize,
                         m_FrameBuffer, AV_PIX_FMT_RGBA, mRenderWidth,
                         mRenderHeight, 1);

    m_SwsContext = sws_getCachedContext(m_SwsContext, mImageWidth, mImageHeight,
                                        m_PixelFormat, mRenderWidth, mRenderHeight,
                                        AV_PIX_FMT_RGBA, SWS_BICUBIC, nullptr,
                                        nullptr, nullptr);
}

void VideoNativeRender::onDrawFrame() {
    LOGCATE("VideoNativeRender::_OnDrawFrame");
    shared_ptr<MediaFrame> p =  m_Callback->GetOneFrame(AVMEDIA_TYPE_VIDEO);
    MediaFrame *frame = p.get();
    if (m_NativeWindow == nullptr || frame == nullptr) return;
    ANativeWindow_lock(m_NativeWindow, &m_NativeWindowBuffer, nullptr);

    sws_scale(m_SwsContext, frame->plane, frame->planeSize,
              0, mImageHeight, m_RGBAFrame->data, m_RGBAFrame->linesize);

    auto *dstBuffer = static_cast<uint8_t *>(m_NativeWindowBuffer.bits);

    int srcLineSize = mRenderWidth * 4;
    int dstLineSize = m_NativeWindowBuffer.stride * 4;
    for (int i = 0; i < mRenderHeight; ++i) {
        memcpy(dstBuffer + i * dstLineSize, m_RGBAFrame->data[0] + i * srcLineSize,
               srcLineSize);
    }
    ANativeWindow_unlockAndPost(m_NativeWindow);
    delete frame;
}

void VideoNativeRender::onSurfaceDestroyed() {
    LOGCATE("VideoNativeRender::_OnSurfaceDestroyed start")
}

int VideoNativeRender::init() {
    return 0;
}

int VideoNativeRender::unInit() {
    LOGCATE("VideoNativeRender::unInit start")
    if (m_SwsContext) {
        sws_freeContext(m_SwsContext);
        m_SwsContext = nullptr;
    }
    if (m_FrameBuffer) {
        free(m_FrameBuffer);
        m_FrameBuffer = nullptr;
    }
    if (m_RGBAFrame) {
        av_frame_free(&m_RGBAFrame);
        m_RGBAFrame = nullptr;
    }
    LOGCATE("VideoNativeRender::unInit finish")
    return 0;
}

void VideoNativeRender::updateMVPMatrix() {

}

