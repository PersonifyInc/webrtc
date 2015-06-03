#ifndef AAC_WINDOWS_ENCODER_H
#define AAC_WINDOWS_ENCODER_H

#include "webrtc/typedefs.h"

#include <Mftransform.h>

class AacWindowsEncoder
{

public:
    AacWindowsEncoder();

    ~AacWindowsEncoder();

    bool Init();

    int Encode(short* audioIn,
               int inLen,
               uint8_t* audioOut);

    void CleanUp();

    unsigned char* getSpecificInfo();

    unsigned long  getSpecificInfoLen();

protected:

    LONGLONG mDurationInHundredNanoSeconds;

    MFT_INPUT_STREAM_INFO mInputStreamInfo;
    MFT_OUTPUT_STREAM_INFO mOutputStreamInfo;

    IMFMediaType* mInputMediaType;
    IMFMediaType* mOutputMediaType;

    IMFTransform* mTransform;

    uint8_t* mAudioSpecificConfig;
};

#endif
