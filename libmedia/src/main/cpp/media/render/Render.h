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

public:
    Render(RenderCallback *callback) {
        m_Callback = callback;
    }

    virtual ~Render() {
        m_Callback = nullptr;
    }

    virtual int Init() { return 0; };

    virtual int UnInit() { return 0; };

    virtual void StartRenderLoop() = 0;

};


#endif //HIKIKOMORI_RENDER_H
