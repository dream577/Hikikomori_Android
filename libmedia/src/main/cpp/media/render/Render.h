//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_RENDER_H
#define HIKIKOMORI_RENDER_H

#include "MediaDef.h"

class RenderCallback {
public:
    virtual Frame *GetOneFrame(int type) = 0;
};


class Render {
protected:
    volatile bool stop = false;

    RenderCallback *m_Callback;
public:
    Render(RenderCallback *callback) {
        m_Callback = callback;
    }

    virtual ~Render() {
        m_Callback = nullptr;
    }

    virtual int init() = 0;

    virtual int unInit() = 0;

    virtual void startRenderThread() = 0;

};


#endif //HIKIKOMORI_RENDER_H
