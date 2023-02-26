//
// Created by bronyna on 2023/2/26.
//

#ifndef HIKIKOMORI_TRIANGLESAMPLE_H
#define HIKIKOMORI_TRIANGLESAMPLE_H

#include "BaseSample.h"

namespace opengles_example {
    class TriangleSample : public BaseSample {
    private:
        GLuint m_Program = GL_NONE;

    protected:

        virtual ~TriangleSample() {}

        virtual void Init();

        virtual void OnDrawFrame();

        virtual void UnInit();

    public:
        TriangleSample() {}
    };
}


#endif //HIKIKOMORI_TRIANGLESAMPLE_H
