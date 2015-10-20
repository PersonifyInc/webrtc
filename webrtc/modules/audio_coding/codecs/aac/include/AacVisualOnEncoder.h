#ifndef AAC_VISUAL_ON_ENCODER_H
#define AAC_VISUAL_ON_ENCODER_H

#include "IAacEncoder.h"
#include "webrtc/modules/audio_coding/codecs/aac/vo-aacenc/common/include/cmnMemory.h"
#include "webrtc/modules/audio_coding/codecs/aac/vo-aacenc/common/include/voAAC.h"

class AacVisualOnEncoder : public IAacEncoder
{

public:
    AacVisualOnEncoder();

    ~AacVisualOnEncoder();

    bool Init() override;

    int Encode(short* audioIn,
               int inLen,
               uint8_t* audioOut) override;

    void CleanUp() override;

protected:
    VO_AUDIO_CODECAPI mCodecApi;
    VO_HANDLE mHandle;
    VO_MEM_OPERATOR mMemOperator;
    VO_CODEC_INIT_USERDATA mUserData;
    uint8_t* mOutput;
};

#endif
