//
// Created by bronyna on 2023/2/7.
//

#include "VideoNativeRender.h"
#include "LogUtil.h"

void VideoNativeRender::onSurfaceCreated() {
    LOGCATE("VideoNativeRender::OnSurfaceCreated");

}

void VideoNativeRender::onSurfaceChanged() {
    if (m_NativeWindow == nullptr) return;
    if (mWindowWidth < mWindowHeight * mVideoWidth / mVideoHeight) {
        mRenderWidth = mWindowWidth;
        mRenderHeight = mWindowWidth * mVideoHeight / mVideoWidth;
    } else {
        mRenderWidth = mWindowHeight * mVideoWidth / mVideoHeight;
        mRenderHeight = mWindowHeight;
    }
    LOGCATE("VideoNativeRender::onSurfaceChanged m_NativeWindow=%p, video[w,h]=[%d, %d]",
            m_NativeWindow, mVideoWidth, mVideoHeight)

    LOGCATE("VideoNativeRender::onSurfaceChanged window[w,h]=[%d, %d],DstSize[w, h]=[%d, %d]",
            mWindowWidth, mWindowHeight, mRenderWidth, mRenderHeight);
    ANativeWindow_setBuffersGeometry(m_NativeWindow, mRenderWidth, mRenderHeight,
                                     WINDOW_FORMAT_RGBA_8888);

    m_RGBAFrame = av_frame_alloc();
    m_BufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, mRenderWidth, mRenderHeight, 1);

    m_FrameBuffer = (uint8_t *) av_malloc(m_BufferSize * sizeof(uint8_t));

    av_image_fill_arrays(m_RGBAFrame->data, m_RGBAFrame->linesize,
                         m_FrameBuffer, AV_PIX_FMT_RGBA, mRenderWidth,
                         mRenderHeight, 1);

    m_SwsContext = sws_getCachedContext(m_SwsContext, mVideoWidth, mVideoHeight,
                                        m_PixelFormat, mRenderWidth, mRenderHeight,
                                        AV_PIX_FMT_RGBA, SWS_BICUBIC, nullptr,
                                        nullptr, nullptr);
}

void VideoNativeRender::onDrawFrame() {
    LOGCATE("VideoNativeRender::onDrawFrame");
    Frame *frame = m_Callback->GetOneFrame(MEDIA_TYPE_VIDEO);
    if (m_NativeWindow == nullptr || frame == nullptr) return;
    auto *videoFrame = (VideoFrame *) frame;
    ANativeWindow_lock(m_NativeWindow, &m_NativeWindowBuffer, nullptr);

    sws_scale(m_SwsContext, videoFrame->yuvBuffer, videoFrame->planeSize,
              0, mVideoHeight, m_RGBAFrame->data, m_RGBAFrame->linesize);

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
    LOGCATE("VideoNativeRender::onSurfaceDestroyed start")
    release();
}

void VideoNativeRender::release() {
    m_Callback->SetPlayerState(STATE_STOP);
    if (m_NativeWindow) {
        ANativeWindow_release(m_NativeWindow);
        m_NativeWindow = nullptr;
    }
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
}
