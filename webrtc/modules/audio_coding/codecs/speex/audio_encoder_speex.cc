/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/modules/audio_coding/codecs/speex/include/audio_encoder_speex.h"

#include "webrtc/base/checks.h"
#include "webrtc/common_types.h"
#include "webrtc/modules/audio_coding/codecs/speex/include/speex_interface.h"

namespace webrtc {

static const int kDefaultComplexity = 5;
static const int kDefaultQuality = 5;
static const int kMaxEncodedBytes = 70;
static const int kDefaultBitrate = 22000;

AudioEncoderSpeex::Config::Config()
    : bitrate(kDefaultBitrate),
      frame_size_ms(20),
      num_channels(1),
      payload_type(120),
      frequency_hz(16000),
      complexity(kDefaultComplexity),
      quality(kDefaultQuality),
      dtx_enabled(false) {
}

bool AudioEncoderSpeex::Config::IsOk() const {
  if (frame_size_ms <= 0 || frame_size_ms % 10 != 0)
    return false;
  if (num_channels != 1)
    return false;
  if (complexity < 1 || complexity > 10)
    return false;
  if (quality < 0 || quality > 10)
    return false;
  if (frequency_hz != 16000)
    return false;
  return true;
}

AudioEncoderSpeex::AudioEncoderSpeex(const Config& config)
    : num_10ms_frames_per_packet_(
          rtc::CheckedDivExact(config.frame_size_ms, 10)),
      bitrate_(config.bitrate),
      num_channels_(config.num_channels),
      payload_type_(config.payload_type),
      dtx_enabled_(config.dtx_enabled),
      complexity_(config.complexity),
      quality_(config.quality),
      frequency_hz_(config.frequency_hz),
      samples_per_10ms_frame_(rtc::CheckedDivExact(frequency_hz_, 100) *
                              num_channels_) {
  CHECK(config.IsOk());
  input_buffer_.reserve(num_10ms_frames_per_packet_ * samples_per_10ms_frame_);
  CHECK_EQ(0, WebRtcSpeex_CreateEnc(&inst_, frequency_hz_));

  CHECK_EQ(0,
           WebRtcSpeex_EncoderInit(inst_, 1, complexity_, dtx_enabled_ ? 1 : 0));
}

AudioEncoderSpeex::~AudioEncoderSpeex() {
  CHECK_EQ(0, WebRtcSpeex_FreeEnc(inst_));
}

int AudioEncoderSpeex::SampleRateHz() const {
  return frequency_hz_;
}

int AudioEncoderSpeex::NumChannels() const {
  return num_channels_;
}

size_t AudioEncoderSpeex::MaxEncodedBytes() const {
  // Calculate the number of bytes we expect the encoder to produce,
  // then multiply by two to give a wide margin for error.
    return kMaxEncodedBytes;
}

int AudioEncoderSpeex::Num10MsFramesInNextPacket() const {
  return num_10ms_frames_per_packet_;
}

int AudioEncoderSpeex::Max10MsFramesInAPacket() const {
  return num_10ms_frames_per_packet_;
}

int AudioEncoderSpeex::GetTargetBitrate() const {
  return -1;
}

AudioEncoder::EncodedInfo AudioEncoderSpeex::EncodeInternal(
    uint32_t rtp_timestamp,
    const int16_t* audio,
    size_t max_encoded_bytes,
    uint8_t* encoded)
{
  if (input_buffer_.empty())
    first_timestamp_in_buffer_ = rtp_timestamp;

  input_buffer_.insert(input_buffer_.end(), audio,
                       audio + samples_per_10ms_frame_);

  // If we don't have enough samples yet, just return.
  if (input_buffer_.size() < (static_cast<size_t>(num_10ms_frames_per_packet_) *
                              samples_per_10ms_frame_)) {
    return EncodedInfo();
  }

  CHECK_EQ(input_buffer_.size(),
           static_cast<size_t>(num_10ms_frames_per_packet_) *
           samples_per_10ms_frame_);

  int status = WebRtcSpeex_Encode(inst_, 
                                  &input_buffer_[0],
                                  bitrate_);
  CHECK_EQ(status, 1);

  // re-use the status variable for the return byte count
  status = WebRtcSpeex_GetBitstream(inst_, reinterpret_cast<int16_t*>(encoded));
  CHECK_GE(status, 0);

  input_buffer_.clear();

  EncodedInfo info;
  info.encoded_bytes = static_cast<size_t>(status);
  info.encoded_timestamp = first_timestamp_in_buffer_;
  info.payload_type = payload_type_;
  info.send_even_if_empty = false;  // Eventually change this when we use DTX
  info.speech = (status > 0);
  return info;
}

namespace {
AudioEncoderSpeex::Config CreateConfig(const CodecInst& codec_inst) {
  AudioEncoderSpeex::Config config;
  config.num_channels = codec_inst.channels;
  config.payload_type = codec_inst.pltype;
  config.frequency_hz = codec_inst.plfreq;
  return config;
}
}  // namespace

AudioEncoderMutableSpeex::AudioEncoderMutableSpeex(const CodecInst& codec_inst)
    : AudioEncoderMutableImpl<AudioEncoderSpeex>(CreateConfig(codec_inst)) {
}

bool AudioEncoderMutableSpeex::SetFec(bool enable) {
  return false;
}

bool AudioEncoderMutableSpeex::SetDtx(bool enable) {
  return false;
}

bool AudioEncoderMutableSpeex::SetMaxPlaybackRate(int frequency_hz) {
  return false;
}

}  // namespace webrtc
