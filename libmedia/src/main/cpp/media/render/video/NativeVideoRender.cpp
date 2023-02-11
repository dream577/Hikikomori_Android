//
// Created by bronyna on 2023/2/7.
//

#include "NativeVideoRender.h"
#include "LogUtil.h"

int NativeVideoRender::init() {
    LOGCATE("NativeVideoRender::Init m_NativeWindow=%p, video[w,h]=[%d, %d]", m_NativeWindow,
            m_VideoWidth, m_VideoHeight);
    if (m_NativeWindow == nullptr) return -1;
    int windowWidth = ANativeWindow_getWidth(m_NativeWindow);
    int windowHeight = ANativeWindow_getHeight(m_NativeWindow);

    if (windowWidth < windowHeight * m_VideoWidth / m_VideoHeight) {
        m_RenderWidth = windowWidth;
        m_RenderHeight = windowWidth * m_VideoHeight / m_VideoWidth;
    } else {
        m_RenderWidth = windowHeight * m_VideoWidth / m_VideoHeight;
        m_RenderHeight = windowHeight;
    }

    LOGCATE("NativeVideoRender::Init window[w,h]=[%d, %d],DstSize[w, h]=[%d, %d]", windowWidth,
            windowHeight, m_RenderWidth, m_RenderHeight);
    ANativeWindow_setBuffersGeometry(m_NativeWindow, m_RenderWidth, m_RenderHeight,
                                     WINDOW_FORMAT_RGBA_8888);

    return 0;
}

int NativeVideoRender::unInit() {
    stop = true;
    if (m_thread) {
        m_thread->join();
        delete m_thread;
        m_thread = nullptr;
    }
    if (m_NativeWindow) {
        ANativeWindow_release(m_NativeWindow);
        m_NativeWindow = nullptr;
    }
    return 0;
}

NativeVideoRender::~NativeVideoRender() {
    NativeVideoRender::unInit();
}

void NativeVideoRender::renderVideoFrame(Frame *frame) {
    LOGCATE("NativeVideoRender::renderVideoFrame");
    if (m_NativeWindow == nullptr || frame == nullptr) return;
    auto *videoFrame = (VideoFrame *) frame;
    ANativeWindow_lock(m_NativeWindow, &m_NativeWindowBuffer, nullptr);
    auto *dstBuffer = static_cast<uint8_t *>(m_NativeWindowBuffer.bits);

    int srcLineSize = videoFrame->width * 4;
    int dstLineSize = m_NativeWindowBuffer.stride * 4;
    for (int i = 0; i < m_RenderHeight; ++i) {
        memcpy(dstBuffer + i * dstLineSize, videoFrame->yuvBuffer[0] + i * srcLineSize,
               srcLineSize);
    }
    ANativeWindow_unlockAndPost(m_NativeWindow);
    delete frame;
}

void NativeVideoRender::startRenderThread() {
    LOGCATE("NativeVideoRender::startRenderThread");
    m_thread = new thread(StartRenderLoop, this);
}

void NativeVideoRender::StartRenderLoop(NativeVideoRender *render) {
    LOGCATE("NativeVideoRender::StartRenderLoop");
    render->doRenderLoop();
}

void NativeVideoRender::doRenderLoop() {
    LOGCATE("NativeVideoRender::doRenderLoop");
    while (!stop) {
        Frame *frame = m_Callback->GetOneFrame(FRAME_TYPE_VIDEO);
        renderVideoFrame(frame);
    }
}
