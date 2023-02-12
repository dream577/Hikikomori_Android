//
// Created by bronyna on 2023/2/7.
//

#include "VideoNativeRender.h"
#include "LogUtil.h"

void VideoNativeRender::OnSurfaceCreated() {

}

void VideoNativeRender::OnSurfaceChanged(int windowWidth, int windowHeight) {
    if (m_NativeWindow == nullptr) return;
    LOGCATE("VideoNativeRender::OnSurfaceChanged m_NativeWindow=%p, video[w,h]=[%d, %d]", m_NativeWindow,
            windowWidth, windowHeight);
    m_WindowWidth = windowWidth;
    m_WindowHeight = windowHeight;

    if (m_WindowWidth < m_WindowHeight * m_VideoWidth / m_VideoHeight) {
        m_RenderWidth = m_WindowWidth;
        m_RenderHeight = m_WindowWidth * m_VideoHeight / m_VideoWidth;
    } else {
        m_RenderWidth = m_WindowHeight * m_VideoWidth / m_VideoHeight;
        m_RenderHeight = m_WindowHeight;
    }

    LOGCATE("VideoNativeRender::OnSurfaceChanged window[w,h]=[%d, %d],DstSize[w, h]=[%d, %d]", windowWidth,
            windowHeight, m_RenderWidth, m_RenderHeight);
    ANativeWindow_setBuffersGeometry(m_NativeWindow, m_RenderWidth, m_RenderHeight,
                                     WINDOW_FORMAT_RGBA_8888);

    m_RGBAFrame = av_frame_alloc();
    m_BufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, m_RenderWidth, m_RenderHeight, 1);

    m_FrameBuffer = (uint8_t *) av_malloc(m_BufferSize * sizeof(uint8_t));

    av_image_fill_arrays(m_RGBAFrame->data, m_RGBAFrame->linesize,
                         m_FrameBuffer, AV_PIX_FMT_RGBA, m_RenderWidth, m_RenderHeight, 1);

    m_SwsContext = sws_getCachedContext(m_SwsContext, m_VideoWidth, m_VideoHeight, m_PixelFormat,
                                        m_RenderWidth, m_RenderHeight, AV_PIX_FMT_RGBA,
                                        SWS_BICUBIC, nullptr, nullptr, nullptr);
}

void VideoNativeRender::OnSurfaceDestroyed() {
    VideoNativeRender::destroy();
}

int VideoNativeRender::init() {
    return 0;
}

int VideoNativeRender::unInit() {
    LOGCATE("VideoNativeRender::unInit start")
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
    LOGCATE("VideoNativeRender::unInit finish")
    return 0;
}

int VideoNativeRender::destroy() {
    m_Callback->SetPlayerState(STATE_STOP);
    if (m_thread) {
        m_thread->join();
        delete m_thread;
        m_thread = nullptr;
    }
    return 0;
}

VideoNativeRender::~VideoNativeRender() {
//    VideoNativeRender::unInit();
}

void VideoNativeRender::renderVideoFrame(Frame *frame) {
    LOGCATE("VideoNativeRender::renderVideoFrame");
    if (m_NativeWindow == nullptr || frame == nullptr) return;
    auto *videoFrame = (VideoFrame *) frame;
    ANativeWindow_lock(m_NativeWindow, &m_NativeWindowBuffer, nullptr);

    sws_scale(m_SwsContext, videoFrame->yuvBuffer, videoFrame->planeSize, 0, m_VideoHeight,
              m_RGBAFrame->data, m_RGBAFrame->linesize);

    auto *dstBuffer = static_cast<uint8_t *>(m_NativeWindowBuffer.bits);

    int srcLineSize = m_RenderWidth * 4;
    int dstLineSize = m_NativeWindowBuffer.stride * 4;
    for (int i = 0; i < m_RenderHeight; ++i) {
        memcpy(dstBuffer + i * dstLineSize, m_RGBAFrame->data[0] + i * srcLineSize,
               srcLineSize);
    }
    ANativeWindow_unlockAndPost(m_NativeWindow);
    delete frame;
}

void VideoNativeRender::startRenderThread() {
    LOGCATE("VideoNativeRender::startRenderThread");
    m_thread = new thread(StartRenderLoop, this);
}

void VideoNativeRender::StartRenderLoop(VideoNativeRender *render) {
    LOGCATE("VideoNativeRender::StartRenderLoop");
    render->doRenderLoop();
    render->unInit();
}

void VideoNativeRender::doRenderLoop() {
    LOGCATE("VideoNativeRender::doRenderLoop");
    while (m_Callback->GetPlayerState() != STATE_STOP) {
        Frame *frame = m_Callback->GetOneFrame(MEDIA_TYPE_VIDEO);
        renderVideoFrame(frame);
    }
}
