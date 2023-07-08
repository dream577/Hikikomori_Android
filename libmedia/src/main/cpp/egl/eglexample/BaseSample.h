//
// Created by bronyna on 2023/2/26.
//

#ifndef HIKIKOMORI_BASESAMPLE_H
#define HIKIKOMORI_BASESAMPLE_H

#include "GLUtils.h"
#include "NativeAssertManager.h"

namespace opengles_example {

    class BaseSample {

    public:

        BaseSample() {}

        virtual ~BaseSample() {}

        virtual void Init() = 0;

        virtual void OnDrawFrame() = 0;

        virtual void UnInit() = 0;
    };

}

#endif //HIKIKOMORI_BASESAMPLE_H
