#ifndef AAC_DEFINES_H
#define AAC_DEFINES_H

#include "webrtc/typedefs.h"

static const uint32_t AAC_BLOCK_ALIGNMENT = 1;
static const uint32_t AAC_LC_LEVEL = 0x29;
static const uint32_t AUDIO_OBJECT_AAC_LC = 2;
static const uint32_t AUDIO_SPECIFIC_CONFIG_LEN_IN_BYTES = 2;
static const uint32_t DEFAULT_BITRATE = 12000;
static const uint32_t DEFAULT_BITS_PER_SAMPLE = 16;
static const uint32_t DEFAULT_SAMPLE_FREQUENCY_IDX = 3;
static const uint32_t DEFAULT_SAMPLE_RATE = 48000;
static const uint32_t MF_MT_USER_DATA_SIZE = 14;
static const uint32_t MONO_NUM_CHANNELS = 1;
static const uint32_t NUM_PCM_SAMPLES = 1024;
static const uint32_t RAW_AAC_PAYLOAD = 0;
static const uint32_t STEREO_NUM_CHANNELS = 2;

template <class T> void SafeRelease(T*& ppT)
{
    if (ppT)
    {
        ppT->Release();
        ppT = nullptr;
    }
}

#endif
