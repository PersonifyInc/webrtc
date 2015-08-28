#include "webrtc/modules/video_coding/codecs/h264/h264_media_codec_encoder.h"

namespace webrtc
{

H264MediaCodecEncoder::H264MediaCodecEncoder()
{
    mImpl = mEncoderFactory.CreateVideoEncoder(webrtc::VideoCodecType::kVideoCodecH264);
}

H264MediaCodecEncoder::~H264MediaCodecEncoder()
{
    mEncoderFactory.DestroyVideoEncoder(mImpl);
}

int H264MediaCodecEncoder::InitEncode(const VideoCodec* codec_settings,
                                      int number_of_cores,
                                      size_t max_payload_size)
{
    if (mImpl == nullptr)
    {
        return -1;
    }

    return mImpl->InitEncode(codec_settings,
                             number_of_cores,
                             max_payload_size);
}

int H264MediaCodecEncoder::Encode(const VideoFrame& input_image,
                                  const CodecSpecificInfo* codec_specific_info,
                                  const std::vector<VideoFrameType>* frame_types)
{
    if (mImpl == nullptr)
    {
        return -1;
    }

    return mImpl->Encode(input_image,
                         codec_specific_info,
                         frame_types);
}

int H264MediaCodecEncoder::RegisterEncodeCompleteCallback(EncodedImageCallback* callback)
{
    if (mImpl == nullptr)
    {
        return -1;
    }

    mCallback = callback;
    return mImpl->RegisterEncodeCompleteCallback(this);
}

int H264MediaCodecEncoder::SetChannelParameters(uint32_t packet_loss, int64_t rtt)
{
    if (mImpl == nullptr)
    {
        return -1;
    }

    return mImpl->SetChannelParameters(packet_loss, rtt);
}

int H264MediaCodecEncoder::SetRates(uint32_t new_bitrate_kbit, uint32_t frame_rate)
{
    if (mImpl == nullptr)
    {
        return -1;
    }

    return mImpl->SetRates(new_bitrate_kbit, frame_rate);
}

int H264MediaCodecEncoder::Release()
{
    if (mImpl == nullptr)
    {
        return -1;
    }

    return mImpl->Release();
}

int32_t H264MediaCodecEncoder::Encoded(const EncodedImage& encoded_image,
                                       const CodecSpecificInfo* codec_specific_info,
                                       const RTPFragmentationHeader* fragmentation)
{
    if (mCallback == nullptr)
    {
        return -1;
    }

    return mCallback->Encoded(encoded_image,
                              codec_specific_info,
                              fragmentation);
}

}
