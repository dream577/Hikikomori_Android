//
// Created by bronyna on 2023/2/7.
//

#ifndef HIKIKOMORI_AUDIORENDER_H
#define HIKIKOMORI_AUDIORENDER_H

#include "Render.h"

class AudioRender : public Render {
public:
    AudioRender(RenderCallback *callback) : Render(callback) {}

    virtual ~AudioRender() {}

    virtual int init() = 0;

    virtual void renderAudioFrame() = 0;

protected:
    virtual int unInit() = 0;
};


#endif //HIKIKOMORI_AUDIORENDER_H