#ifndef AAC_WINDOWS_DECODER_H
#define AAC_WINDOWS_DECODER_H

#include "IAacDecoder.h"
#include <Mftransform.h>

class AacWindowsDecoder : public IAacDecoder
{
public:
    AacWindowsDecoder();

    ~AacWindowsDecoder();

    bool Init() override;

    int Decode(const uint8_t* audioIn,
               int numBytes,
               int16_t* decoded) override;

    void CleanUp() override;

private:


    MFT_INPUT_STREAM_INFO mInputStreamInfo;
    MFT_OUTPUT_STREAM_INFO mOutputStreamInfo;

    IMFMediaType* mInputMediaType;
    IMFMediaType* mOutputMediaType;

    IMFTransform* mTransform;
};

#endif
