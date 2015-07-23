/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_AUDIO_CODING_CODECS_AAC_INTERFACE_AUDIO_ENCODER_AAC_H_
#define WEBRTC_MODULES_AUDIO_CODING_CODECS_AAC_INTERFACE_AUDIO_ENCODER_AAC_H_

#include <vector>

#include "webrtc/base/scoped_ptr.h"
#include "webrtc/modules/audio_coding/codecs/audio_encoder_mutable_impl.h"
#include "webrtc/modules/audio_coding/codecs/aac/include/aac_interface.h"
#include "webrtc/modules/audio_coding/codecs/audio_encoder.h"

namespace webrtc
{

// NOTE: This class has neither ThreadChecker, nor locks. The owner of an
// AudioEncoderOpus object must ensure that it is not accessed concurrently.

class AudioEncoderAac final : public AudioEncoder
{
public:

    struct Config
    {
        Config();
        bool IsOk() const;
        int num_channels;
        int payload_type;
        int bitrate_bps;
        bool fec_enabled;
        int max_playback_rate_hz;
        int complexity;
        bool dtx_enabled;
    };

    explicit AudioEncoderAac(const Config& config);
    ~AudioEncoderAac() override;

    int SampleRateHz() const override;
    int NumChannels() const override;
    size_t MaxEncodedBytes() const override;
    int Num10MsFramesInNextPacket() const override;
    int Max10MsFramesInAPacket() const override;
    int GetTargetBitrate() const override;
    void SetTargetBitrate(int bits_per_second) override;

    EncodedInfo EncodeInternal(uint32_t rtp_timestamp,
                               const int16_t* audio,
                               size_t max_encoded_bytes,
                               uint8_t* encoded) override;

private:
    const int num_channels_;
    const int payload_type_;
    const int samples_per_10ms_frame_;
    WebRtcAacEncInst* inst_;
    uint32_t first_timestamp_in_buffer_;
};

struct CodecInst;

class AudioEncoderMutableAac
    : public AudioEncoderMutableImpl<AudioEncoderAac>
{
public:
    explicit AudioEncoderMutableAac(const CodecInst& codec_inst);
};

}  // namespace webrtc
#endif  // WEBRTC_MODULES_AUDIO_CODING_CODECS_AAC_INTERFACE_AUDIO_ENCODER_AAC_H_
