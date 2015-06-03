#include "acm_aac.h"

#ifdef WEBRTC_CODEC_AAC
// NOTE! AAC is not included in the open-source package. Modify this file or
// your codec API to match the function calls and names of used AAC API file.
#include "webrtc/modules/audio_coding/codecs/aac/include/aac_interface.h"
#include "webrtc/modules/audio_coding/main/acm2/acm_codec_database.h"
#include "webrtc/modules/audio_coding/main/acm2/acm_common_defs.h"
#include "webrtc/system_wrappers/interface/trace.h"
#endif

namespace webrtc
{
namespace acm2
{

#ifndef WEBRTC_CODEC_AAC

ACMAAC::ACMAAC(int16_t codec_id) :
    encoder_inst_ptr_(nullptr)
{
    return;
}

ACMAAC::~ACMAAC()
{
    return;
}

ACMGenericCodec* ACMAAC::CreateInstance()
{
    return nullptr;
}

int16_t ACMAAC::InternalEncode(uint8_t* bitstream,
                               int16_t* bitstream_len_byte)
{
    return -1;
}

int16_t ACMAAC::InternalInitEncoder(WebRtcACMCodecParams* codec_params)
{
    return -1;
}

int16_t ACMAAC::InternalCreateEncoder()
{
    return -1;
}

void ACMAAC::DestructEncoderSafe()
{
    return;
}

#else

ACMAAC::ACMAAC(int16_t codec_id) :
    encoder_inst_ptr_(nullptr),
    samples_in_10ms_audio_(480) // 48 kHz by default
{
    codec_id_ = codec_id;
    return;
}

ACMAAC::~ACMAAC()
{
    if (encoder_inst_ptr_ != nullptr)
    {
        WebRtcAac_EncoderFree(encoder_inst_ptr_);
        encoder_inst_ptr_ = nullptr;
    }
}

ACMGenericCodec* ACMAAC::CreateInstance()
{
    return nullptr;
}

int16_t ACMAAC::InternalEncode(uint8_t* bitstream,
                               int16_t* bitstream_len_byte)
{
    *bitstream_len_byte = WebRtcAac_Encode(encoder_inst_ptr_,
                                           &in_audio_[in_audio_ix_read_],
                                           samples_in_10ms_audio_,
                                           bitstream);

    in_audio_ix_read_ += samples_in_10ms_audio_;

    return *bitstream_len_byte;
}

int16_t ACMAAC::InternalInitEncoder(WebRtcACMCodecParams* codec_params)
{
    if (encoder_inst_ptr_ == nullptr)
    {
        return -1;
    }

    return WebRtcAac_EncoderInit(encoder_inst_ptr_);
}

int16_t ACMAAC::InternalCreateEncoder()
{
    return WebRtcAac_EncoderCreate(&encoder_inst_ptr_);
}

void ACMAAC::DestructEncoderSafe()
{
    if (encoder_inst_ptr_ != nullptr)
    {
        WebRtcAac_EncoderFree(encoder_inst_ptr_);
        encoder_inst_ptr_ = nullptr;

        encoder_exist_ = false;
        encoder_initialized_ = false;
    }
}

#endif
}
}
