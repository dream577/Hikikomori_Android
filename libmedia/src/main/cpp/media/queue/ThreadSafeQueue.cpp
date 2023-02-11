//
// Created by bronyna on 2023/2/10.
//

#include "ThreadSafeQueue.h"

ThreadSafeQueue::ThreadSafeQueue() {
    header = tail = nullptr;
    m_Size = 0;
    m_MaxSize = INT_MAX;
    abort_request = 0;
}

ThreadSafeQueue::ThreadSafeQueue(int maxSize) : ThreadSafeQueue() {
    m_MaxSize = maxSize;
}

ThreadSafeQueue::~ThreadSafeQueue() {
    abort();
    flush();
}

void ThreadSafeQueue::abort() {
    unique_lock<mutex> lock(m_Mutex);
    abort_request = 1;
    m_CondVar.notify_all();
}

void ThreadSafeQueue::start() {
    unique_lock<mutex> lock(m_Mutex);
    abort_request = 0;
    m_CondVar.notify_all();
}

void ThreadSafeQueue::flush() {
    unique_lock<mutex> lock(m_Mutex);
    FrameNode *temp;
    while (header) {
        temp = header;
        header = header->next;
        if (temp->frame) {
            delete temp->frame;
            temp->frame = nullptr;
            free(temp);
        }
    }
    header = tail = nullptr;
    m_Size = 0;
    m_MaxSize = INT_MAX;
    abort_request = 0;
    m_CondVar.notify_all();
}

int ThreadSafeQueue::put(Frame *frame) {
    if (frame == nullptr) return -1;
    if (m_Size >= m_MaxSize) return -1;
    FrameNode *node = (FrameNode *) malloc(sizeof(FrameNode));
    node->frame = frame;
    node->next = nullptr;
    if (header == nullptr) {
        header = node;
        tail = node;
    } else {
        tail->next = node;
        tail = node;
    }
    m_Size++;
    return 0;
}

Frame *ThreadSafeQueue::pop() {
    Frame *frame = nullptr;
    if (m_Size != 0) {
        if (header != nullptr) {
            frame = header->frame;
            FrameNode *temp = header;
            header = header->next;
            free(temp);
            m_Size--;
            if (m_Size == 0) {
                tail = nullptr;
            }
        }
    }
    return frame;
}

int ThreadSafeQueue::offer(Frame *frame) {
    int ret = -1;
    unique_lock<mutex> lock(m_Mutex);
    if (m_Size >= m_MaxSize) {
        m_CondVar.wait(lock);
    }
    if (!abort_request) {
        ret = put(frame);
    }
    m_CondVar.notify_all();
    lock.unlock();
    return ret;
}

Frame *ThreadSafeQueue::poll() {
    Frame *frame = nullptr;
    unique_lock<mutex> lock(m_Mutex);
    if (m_Size == 0) {
        m_CondVar.wait(lock);
    }
    if (!abort_request) {
        frame = pop();
    }
    m_CondVar.notify_all();
    lock.unlock();
    return frame;
}
