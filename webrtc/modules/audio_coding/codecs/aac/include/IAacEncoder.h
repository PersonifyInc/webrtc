#ifndef I_AAC_ENCODER_H
#define I_AAC_ENCODER_H

#include "webrtc/typedefs.h"

class IAacEncoder
{
public:
    virtual bool Init() = 0;

    virtual int Encode(short* audioIn,
                       int inLen,
                       uint8_t* audioOut) = 0;

    virtual void CleanUp() = 0;

    virtual ~IAacEncoder() {}
};
#endif
