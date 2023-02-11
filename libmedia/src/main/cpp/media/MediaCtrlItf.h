//
// Created by bronyna on 2023/2/9.
//

#ifndef HIKIKOMORI_MEDIACTRLITF_H
#define HIKIKOMORI_MEDIACTRLITF_H

class MediaCtrlItf {

    virtual void Play() = 0;

    virtual void Pause() = 0;

    virtual void Stop() = 0;

    virtual void SeekToPosition(float position) = 0;

};

#endif //HIKIKOMORI_MEDIACTRLITF_H
