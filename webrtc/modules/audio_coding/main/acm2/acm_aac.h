#ifndef WEBRTC_MODULES_AUDIO_CODING_MAIN_ACM2_ACM_AAC_H_
#define WEBRTC_MODULES_AUDIO_CODING_MAIN_ACM2_ACM_AAC_H_

#include "webrtc/modules/audio_coding/main/acm2/acm_generic_codec.h"

// forward declaration
struct WebRtcAacEncInst;

namespace webrtc
{
namespace acm2
{
class ACMAAC : public ACMGenericCodec
{
public:
    ACMAAC(int16_t codec_id);

    ~ACMAAC();

    ACMGenericCodec* CreateInstance();

    EXCLUSIVE_LOCKS_REQUIRED(codec_wrapper_lock_);

    int16_t InternalEncode(uint8_t* bitstream,
                           int16_t* bitstream_len_byte);

    int16_t InternalInitEncoder(WebRtcACMCodecParams* codec_params);

protected:
    int16_t InternalCreateEncoder();

    void DestructEncoderSafe();

private:
    WebRtcAacEncInst* encoder_inst_ptr_;
    uint16_t samples_in_10ms_audio_;
};

}
}
#endif
