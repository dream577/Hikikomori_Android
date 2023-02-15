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
    if (m_WindowSize[0] < m_WindowSize[1] * m_VideoSize[0] / m_VideoSize[1]) {
        m_RenderSize[0] = m_WindowSize[0];
        m_RenderSize[1] = m_WindowSize[0] * m_VideoSize[1] / m_VideoSize[0];
    } else {
        m_RenderSize[0] = m_WindowSize[1] * m_VideoSize[0] / m_VideoSize[1];
        m_RenderSize[1] = m_WindowSize[1];
    }
    LOGCATE("VideoNativeRender::onSurfaceChanged m_NativeWindow=%p, video[w,h]=[%d, %d]",
            m_NativeWindow, m_VideoSize[0], m_VideoSize[1])

    LOGCATE("VideoNativeRender::onSurfaceChanged window[w,h]=[%d, %d],DstSize[w, h]=[%d, %d]",
            m_WindowSize[0], m_WindowSize[1], m_RenderSize[0], m_RenderSize[1]);
    ANativeWindow_setBuffersGeometry(m_NativeWindow, m_RenderSize[0], m_RenderSize[1],
                                     WINDOW_FORMAT_RGBA_8888);

    m_RGBAFrame = av_frame_alloc();
    m_BufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, m_RenderSize[0], m_RenderSize[1], 1);

    m_FrameBuffer = (uint8_t *) av_malloc(m_BufferSize * sizeof(uint8_t));

    av_image_fill_arrays(m_RGBAFrame->data, m_RGBAFrame->linesize,
                         m_FrameBuffer, AV_PIX_FMT_RGBA, m_RenderSize[0],
                         m_RenderSize[1], 1);

    m_SwsContext = sws_getCachedContext(m_SwsContext, m_VideoSize[0], m_VideoSize[1],
                                        m_PixelFormat, m_RenderSize[0], m_RenderSize[1],
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
              0, m_VideoSize[1], m_RGBAFrame->data, m_RGBAFrame->linesize);

    auto *dstBuffer = static_cast<uint8_t *>(m_NativeWindowBuffer.bits);

    int srcLineSize = m_RenderSize[0] * 4;
    int dstLineSize = m_NativeWindowBuffer.stride * 4;
    for (int i = 0; i < m_RenderSize[1]; ++i) {
        memcpy(dstBuffer + i * dstLineSize, m_RGBAFrame->data[0] + i * srcLineSize,
               srcLineSize);
    }
    ANativeWindow_unlockAndPost(m_NativeWindow);
    delete frame;
}

void VideoNativeRender::onSurfaceDestroyed() {
    LOGCATE("VideoNativeRender::onSurfaceDestroyed start")
    release();
    LOGCATE("VideoNativeRender::onSurfaceDestroyed finish")
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
