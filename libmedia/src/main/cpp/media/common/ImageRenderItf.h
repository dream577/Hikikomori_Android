//
// Created by 郝留凯 on 2023/7/2.
//

#ifndef HIKIKOMORI_IMAGERENDERITF_H
#define HIKIKOMORI_IMAGERENDERITF_H

#include <jni.h>

class ImageRenderItf {
public:
    virtual void OnSurfaceCreated(JNIEnv *jniEnv, jobject surface) = 0;

    virtual void OnSurfaceChanged(int width, int height) = 0;

    virtual void OnSurfaceDestroyed() = 0;
};

#endif //HIKIKOMORI_IMAGERENDERITF_H
