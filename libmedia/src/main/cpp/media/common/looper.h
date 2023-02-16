/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HIKIKOMORI_LOOPER_H
#define HIKIKOMORI_LOOPER_H

#include <pthread.h>
#include <semaphore.h>

struct loopermessage;

#define  ENABLE_AUTO_LOOP_MSG    10000
#define  DISABLE_AUTO_LOOP_MSG   10001

class looper {
public:
    looper();

    looper &operator=(const looper &) = delete;

    looper(looper &) = delete;

    virtual ~looper();

    void post(int what, void *data, bool flush = false);

    void quit();

    virtual void handle(int what, void *data);

protected:
    void enableAutoLoop(int *what);

    void disableAutoLoop();

private:
    void addmsg(loopermessage *msg, bool flush);

    static void *trampoline(void *p);

    void loop();

    loopermessage *head = nullptr;
    pthread_t worker;
    sem_t headwriteprotect;
    sem_t headdataavailable;
    bool running;
    bool autoLoopEnable = false;  // 是否允许自动循环执行指定的msg类型
    int autoLoopMsg = -1;         // 自动循环执行的消息类型
};

#endif
