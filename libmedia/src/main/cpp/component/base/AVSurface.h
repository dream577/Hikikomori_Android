//
// Created by 郝留凯 on 2023/7/2.
//

#ifndef HIKIKOMORI_AVSURFACE_H
#define HIKIKOMORI_AVSURFACE_H

#include <jni.h>

class AVSurface {
public:
    virtual void OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) = 0;

    virtual void OnSurfaceChanged(int width, int height) = 0;

    virtual void OnSurfaceDestroyed() = 0;
};

#endif //HIKIKOMORI_AVSURFACE_H
