//
// Created by 郝留凯 on 2023/2/26.
//

#ifndef HIKIKOMORI_BASEEXAMPLE_H
#define HIKIKOMORI_BASEEXAMPLE_H

#include "VioletEGLSurface.h"

namespace opengles_example {

    class BaseExample {
    private:
        VioletEGLSurface *mSurface;

    protected:
        virtual void onSurfaceCreated() {}

        virtual void onSurfaceChanged() {}

        virtual void onSurfaceDestroyed() {}

    public:
        void Init() {};

        void OnSurfaceCreated() {}

        void OnSurfaceChanged() {}

        void OnSurfaceDestroyed() {}

        void UnInit() {};
    };
}

#endif //HIKIKOMORI_BASEEXAMPLE_H
