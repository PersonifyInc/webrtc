#ifndef AAC_WINDOWS_ENCODER_H
#define AAC_WINDOWS_ENCODER_H

#include "IAacEncoder.h"
#include <Mftransform.h>

class AacWindowsEncoder : public IAacEncoder
{

public:
    AacWindowsEncoder();

    ~AacWindowsEncoder();

    bool Init() override;

    int Encode(short* audioIn,
               int inLen,
               uint8_t* audioOut) override;

    void CleanUp() override;

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
