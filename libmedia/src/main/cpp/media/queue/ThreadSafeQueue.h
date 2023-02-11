//
// Created by bronyna on 2023/2/10.
//

#ifndef HIKIKOMORI_THREADSAFEQUEUE_H
#define HIKIKOMORI_THREADSAFEQUEUE_H

#include <thread>
#include <limits.h>

#include "MediaDef.h"

using namespace std;

typedef struct FrameNode {
    Frame *frame;
    FrameNode *next;
} FrameNode;

class ThreadSafeQueue {
public:
    ThreadSafeQueue();

    ThreadSafeQueue(int maxSize);

    ~ThreadSafeQueue();

    /**
     * 帧放入队列
     * @param frame 视频帧或者音频帧，注意不要传入空指针，如果一定要传空请构造一个空帧
     * @return
     */
    int put(Frame *frame);   // 线程不安全的实现方式

    Frame *pop();            // 线程不安全的实现方式

    /**
     * 帧放入队列
     * @param frame 视频帧或者音频帧，注意不要传入空指针，如果一定要传空请构造一个空帧
     * @return
     */
    int offer(Frame *frame); // 线程安全的实现方式

    Frame *poll();           // 线程安全的实现方式

    void abort();

    void flush();

    void start();

private:
    FrameNode *header, *tail;

    mutex m_Mutex;

    condition_variable m_CondVar;

    int m_Size;

    int m_MaxSize;

    volatile int abort_request;
};

#endif //HIKIKOMORI_THREADSAFEQUEUE_H
