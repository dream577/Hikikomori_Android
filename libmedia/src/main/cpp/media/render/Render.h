//
// Created by bronyna on 2023/2/5.
//

#ifndef HIKIKOMORI_RENDER_H
#define HIKIKOMORI_RENDER_H

#include "MediaDef.h"

class RenderCallback {
public:
    virtual Frame *GetOneFrame(int type) = 0;

    virtual int GetPlayerState() = 0;

    virtual void SetPlayerState(PlayerState state) = 0;
};


class Render {
protected:
    RenderCallback *m_Callback;

    virtual int unInit() = 0;

public:
    Render(RenderCallback *callback) {
        m_Callback = callback;
    }

    virtual ~Render() {
        m_Callback = nullptr;
    }

    virtual int init() = 0;

    virtual int destroy() = 0;

    virtual void startRenderThread() = 0;

};


#endif //HIKIKOMORI_RENDER_H
