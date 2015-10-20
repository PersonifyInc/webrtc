#ifndef I_AAC_DECODER_H
#define I_AAC_DECODER_H

#include "webrtc/typedefs.h"

class IAacDecoder
{
public:
    virtual bool Init() = 0;

    virtual int Decode(const uint8_t* audioIn,
                       int numBytes,
                       int16_t* decoded) = 0;

    virtual void CleanUp() = 0;

    virtual ~IAacDecoder() {}
};

#endif
