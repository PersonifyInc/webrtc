#ifndef AAC_VISUAL_ON_ENCODER_H
#define AAC_VISUAL_ON_ENCODER_H

#include "third_party/vo-aacenc/common/include/cmnMemory.h"
#include "third_party/vo-aacenc/common/include/voAAC.h"
#include "webrtc/typedefs.h"

class AacVisualOnEncoder
{

public:
    AacVisualOnEncoder();

    ~AacVisualOnEncoder();

    bool Init();

    int Encode(short* audioIn,
               int inLen,
               uint8_t* audioOut);

    void CleanUp();

    unsigned char* getSpecificInfo();

    unsigned long  getSpecificInfoLen();

protected:
    VO_AUDIO_CODECAPI mCodecApi;
    VO_HANDLE mHandle;
    VO_MEM_OPERATOR mMemOperator;
    VO_CODEC_INIT_USERDATA mUserData;
};

#endif
