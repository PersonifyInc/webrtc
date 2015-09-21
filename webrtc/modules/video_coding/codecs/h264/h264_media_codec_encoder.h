
#ifndef WEBRTC_MODULES_VIDEO_CODING_CODECS_H264_H264_MEDIA_CODEC_ENCODER_H_
#define WEBRTC_MODULES_VIDEO_CODING_CODECS_H264_H264_MEDIA_CODEC_ENCODER_H_

#include "talk/app/webrtc/java/jni/androidmediaencoder_jni.h"
#include "talk/media/webrtc/webrtcvideoencoderfactory.h"
#include "webrtc/modules/video_coding/codecs/h264/include/h264.h"

#include <vector>

namespace webrtc
{

class H264MediaCodecEncoder : public H264Encoder, public EncodedImageCallback
{
public:
    H264MediaCodecEncoder();

    ~H264MediaCodecEncoder() override;

    int InitEncode(const VideoCodec* codec_settings,
                   int number_of_cores,
                   size_t max_payload_size) override;

    int Encode(const VideoFrame& input_image,
               const CodecSpecificInfo* codec_specific_info,
               const std::vector<VideoFrameType>* frame_types) override;

    int RegisterEncodeCompleteCallback(EncodedImageCallback* callback) override;

    int SetChannelParameters(uint32_t packet_loss, int64_t rtt) override;

    int SetRates(uint32_t new_bitrate_kbit, uint32_t frame_rate) override;

    int Release() override;

    int32_t Encoded(const EncodedImage& encoded_image,
                          const CodecSpecificInfo* codec_specific_info,
                          const RTPFragmentationHeader* fragmentation) override;

private:
    webrtc_jni::MediaCodecVideoEncoderFactory mEncoderFactory;
    webrtc::VideoEncoder* mImpl;
    webrtc::EncodedImageCallback* mCallback;
};  // H264MediaCodecEncoder

}  // namespace webrtc

#endif  // WEBRTC_MODULES_VIDEO_CODING_CODECS_H264_H264_MEDIA_CODEC_ENCODER_H_
