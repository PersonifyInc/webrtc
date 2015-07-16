/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/modules/audio_coding/codecs/aac/include/audio_encoder_aac.h"

#include "webrtc/base/checks.h"
#include "webrtc/common_types.h"
#include "webrtc/modules/audio_coding/codecs/aac/include/aac_interface.h"

namespace webrtc
{

static const int kSampleRateHz = 48000;

AudioEncoderAac::Config::Config()
    : num_channels(1),
      payload_type(83)
{
}

bool AudioEncoderAac::Config::IsOk() const
{
    if (num_channels != 1)
    {
        return false;
    }
    return true;
}

AudioEncoderAac::AudioEncoderAac(const Config& config)
    : num_channels_(config.num_channels),
      payload_type_(config.payload_type),
      samples_per_10ms_frame_(rtc::CheckedDivExact(kSampleRateHz, 100) *
                              num_channels_)
{
    CHECK(config.IsOk());
    CHECK_EQ(0, WebRtcAac_EncoderCreate(&inst_));
    CHECK_EQ(0, WebRtcAac_EncoderInit(inst_));
}

AudioEncoderAac::~AudioEncoderAac()
{
    CHECK_EQ(0, WebRtcAac_EncoderFree(inst_));
}

int AudioEncoderAac::SampleRateHz() const
{
    return kSampleRateHz;
}

int AudioEncoderAac::NumChannels() const
{
    return num_channels_;
}

size_t AudioEncoderAac::MaxEncodedBytes() const
{
    return 0;
}

int AudioEncoderAac::Num10MsFramesInNextPacket() const
{
    return -1;
}

int AudioEncoderAac::Max10MsFramesInAPacket() const
{
    return -1;
}

int AudioEncoderAac::GetTargetBitrate() const
{
    return -1;
}

void AudioEncoderAac::SetTargetBitrate(int bits_per_second)
{
    // TODO?
}

AudioEncoder::EncodedInfo AudioEncoderAac::EncodeInternal(
    uint32_t rtp_timestamp,
    const int16_t* audio,
    size_t max_encoded_bytes,
    uint8_t* encoded)
{
    if (first_timestamp_in_buffer_ == 0)
    {
        first_timestamp_in_buffer_ = rtp_timestamp;
    }

    int status = WebRtcAac_Encode(inst_, const_cast<int16_t*>(audio),
                                  samples_per_10ms_frame_,
                                  encoded);

    if (status == 0)
    {
        // Encoder buffers these samples internally, but doesn't have enough
        // to produce output on this pass. Keep calling EncodeInternal!
        return EncodedInfo();
    }

    EncodedInfo info;
    info.encoded_bytes = static_cast<size_t>(status);
    info.encoded_timestamp = first_timestamp_in_buffer_;
    info.payload_type = payload_type_;
    info.send_even_if_empty = false;
    info.speech = (status > 0);

    first_timestamp_in_buffer_ = 0; // reset this for next pass.
    return info;
}

namespace
{
AudioEncoderAac::Config CreateConfig(const CodecInst& codec_inst)
{
    AudioEncoderAac::Config config;
    config.num_channels = codec_inst.channels;
    config.payload_type = codec_inst.pltype;
    return config;
}
}  // namespace

AudioEncoderMutableAac::AudioEncoderMutableAac(const CodecInst& codec_inst)
    : AudioEncoderMutableImpl<AudioEncoderAac>(CreateConfig(codec_inst))
{
}

}  // namespace webrtc
