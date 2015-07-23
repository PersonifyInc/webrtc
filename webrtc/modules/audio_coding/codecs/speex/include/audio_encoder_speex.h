/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_AUDIO_CODING_CODECS_SPEEX_INTERFACE_AUDIO_ENCODER_SPEEX_H_
#define WEBRTC_MODULES_AUDIO_CODING_CODECS_SPEEX_INTERFACE_AUDIO_ENCODER_SPEEX_H_

#include <vector>

#include "webrtc/base/scoped_ptr.h"
#include "webrtc/modules/audio_coding/codecs/audio_encoder_mutable_impl.h"
#include "webrtc/modules/audio_coding/codecs/speex/include/speex_interface.h"
#include "webrtc/modules/audio_coding/codecs/audio_encoder.h"

namespace webrtc {

// NOTE: This class has neither ThreadChecker, nor locks. The owner of an
// AudioEncoderSpeex object must ensure that it is not accessed concurrently.

class AudioEncoderSpeex final : public AudioEncoder {
 public:

  struct Config {
    Config();
    bool IsOk() const;
    int bitrate;
    int frame_size_ms;
    int num_channels;
    int payload_type;
    int frequency_hz;
    int complexity;
    int quality;
    bool dtx_enabled;
  };

  explicit AudioEncoderSpeex(const Config& config);
  ~AudioEncoderSpeex() override;

  int SampleRateHz() const override;
  int NumChannels() const override;
  size_t MaxEncodedBytes() const override;
  int Num10MsFramesInNextPacket() const override;
  int Max10MsFramesInAPacket() const override;
  int GetTargetBitrate() const override;

  bool dtx_enabled() const { return dtx_enabled_; }

  EncodedInfo EncodeInternal(uint32_t rtp_timestamp,
                             const int16_t* audio,
                             size_t max_encoded_bytes,
                             uint8_t* encoded) override;

 private:
  const int num_10ms_frames_per_packet_;
  const int bitrate_;
  const int num_channels_;
  const int payload_type_;
  const bool dtx_enabled_;
  const int complexity_;
  const int quality_;
  const int frequency_hz_;
  const int samples_per_10ms_frame_;
  std::vector<int16_t> input_buffer_;
  SPEEX_encinst_t* inst_;
  uint32_t first_timestamp_in_buffer_;
};

struct CodecInst;

class AudioEncoderMutableSpeex
    : public AudioEncoderMutableImpl<AudioEncoderSpeex> {
 public:
  explicit AudioEncoderMutableSpeex(const CodecInst& codec_inst);
  bool SetFec(bool enable) override;

  // Set Speex DTX. Once enabled, Opus stops transmission, when it detects voice
  // being inactive. During that, it still sends 2 packets (one for content, one
  // for signaling) about every 400 ms.
  bool SetDtx(bool enable) override;

  bool SetMaxPlaybackRate(int frequency_hz) override;
  bool dtx_enabled() const {
    CriticalSectionScoped cs(encoder_lock_.get());
    return encoder()->dtx_enabled();
  }
};

}  // namespace webrtc
#endif  // WEBRTC_MODULES_AUDIO_CODING_CODECS_SPEEX_INTERFACE_AUDIO_ENCODER_SPEEX_H_
