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

#include "looper.h"

#include <cassert>
#include <cerrno>
#include <fcntl.h>
#include <jni.h>
#include <climits>
#include <pthread.h>
#include <semaphore.h>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "LogUtil.h"

struct loopermessage;
typedef struct loopermessage loopermessage;

struct loopermessage {
    int what;
    void *obj;
    loopermessage *next;
    bool quit;
};

void *looper::trampoline(void *p) {
    ((looper *) p)->loop();
    return NULL;
}

looper::looper() {
    sem_init(&headdataavailable, 0, 0);
    sem_init(&headwriteprotect, 0, 1);
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    pthread_create(&worker, &attr, trampoline, this);
    running = true;
}

looper::~looper() {
    if (running) {
        LOGCATE("Looper deleted while still running. Some messages will not be ""processed")
        quit();
    }
}

void looper::post(int what, void *data, bool flush) {
    loopermessage *msg = new loopermessage();
    msg->what = what;
    msg->obj = data;
    msg->next = NULL;
    msg->quit = false;
    addmsg(msg, flush);
}

void looper::addmsg(loopermessage *msg, bool flush) {
    sem_wait(&headwriteprotect);
    loopermessage *h = head;

    if (flush) {
        while (h) {
            loopermessage *next = h->next;
            delete h;
            h = next;
        }
        h = NULL;
    }
    if (h) {
        while (h->next) {
            h = h->next;
        }
        h->next = msg;
    } else {
        head = msg;
    }
//    LOGCATE("post msg %d", msg->what);
    sem_post(&headwriteprotect);
    sem_post(&headdataavailable);
}

void looper::loop() {
    while (true) {
        // wait for available message
        sem_wait(&headdataavailable);

        // get next available message
        sem_wait(&headwriteprotect);
        loopermessage *msg = head;
        if (msg == NULL) {
//            LOGV("no msg");
            sem_post(&headwriteprotect);
            if (autoLoopEnable) {
                handle(autoLoopMsg, nullptr);
                sem_post(&headdataavailable);
            }
            continue;
        }
        head = msg->next;
        sem_post(&headwriteprotect);

        if (msg->quit) {
            LOGCATE("Lopper quitting");
            delete msg;
            return;
        }
//        LOGCATE("processing msg %d", msg->what);
        handle(msg->what, msg->obj);
        delete msg;
    }
}

void looper::quit() {
    LOGCATE("Looper m_UnInit")
    loopermessage *msg = new loopermessage();
    msg->what = 0;
    msg->obj = NULL;
    msg->next = NULL;
    msg->quit = true;
    addmsg(msg, false);
    void *retval;
    pthread_join(worker, &retval);
    sem_destroy(&headdataavailable);
    sem_destroy(&headwriteprotect);
    running = false;
}

void looper::handle(int what, void *obj) {
//    LOGV("dropping msg %d %p", what, obj);
    if (what == ENABLE_AUTO_LOOP_MSG) {
        int *p = (int *) obj;
        if (p) {
            autoLoopMsg = *p;
        }
        autoLoopEnable = true;
        sem_post(&headdataavailable);
    }
    if (what == DISABLE_AUTO_LOOP_MSG) {
        autoLoopMsg = -1;
        autoLoopEnable = false;
    }
}

void looper::enableAutoLoop(int *loopMsg) {
    post(ENABLE_AUTO_LOOP_MSG, loopMsg);
}

void looper::disableAutoLoop() {
    post(DISABLE_AUTO_LOOP_MSG, nullptr);
}
