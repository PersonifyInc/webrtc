#include "AacDefines.h"
#include "AacWindowsDecoder.h"

#include <Mfapi.h>
#include <Mferror.h>
#include <Windows.h>

AacWindowsDecoder::AacWindowsDecoder() :
    mInputMediaType(nullptr),
    mOutputMediaType(nullptr),
    mTransform(nullptr)
{
    mInputStreamInfo = { 0 };
    mOutputStreamInfo = { 0 };
}

AacWindowsDecoder::~AacWindowsDecoder()
{
    CleanUp();
}

bool AacWindowsDecoder::Init()
{
    HRESULT hr = S_OK;

    UINT32 count = 0;
    IMFActivate** activate = NULL;

    // we'll be looking for an AAC decoder
    MFT_REGISTER_TYPE_INFO info =
    {
        MFMediaType_Audio,
        MFAudioFormat_AAC
    };

    hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        return false;
    }

    // look for transforms that match AAC Decoder specs
    hr = MFTEnumEx(MFT_CATEGORY_AUDIO_DECODER,
                   0,
                   &info,
                   nullptr,
                   &activate,
                   &count);

    if (SUCCEEDED(hr) && count > 0)
    {
        hr = activate[0]->ActivateObject(IID_PPV_ARGS(&mTransform));

        // set input type
        hr = MFCreateMediaType(&mInputMediaType);

        if (SUCCEEDED(hr))
        {
            UINT32 in_block_align
                = MONO_NUM_CHANNELS * AAC_BLOCK_ALIGNMENT;

            // must be MFMediaType_Audio
            mInputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);

            // Raw AAC or ADTS AAC
            mInputMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);

            // we default to AAC Low Complexity
            mInputMediaType->SetUINT32(MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION,
                                       AAC_LC_LEVEL);

            // 16 bit-depth for decoded PCM audio
            mInputMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE,
                                       DEFAULT_BITS_PER_SAMPLE);

            // the stream contains raw_data_block() elements only
            mInputMediaType->SetUINT32(MF_MT_AAC_PAYLOAD_TYPE,
                                       RAW_AAC_PAYLOAD);

            // 48 kHz sample rate
            mInputMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                       DEFAULT_SAMPLE_RATE);

            // Mono
            mInputMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS,
                                       MONO_NUM_CHANNELS);

            mInputMediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT,
                                       in_block_align);

            mInputMediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND,
                                       DEFAULT_BITRATE);

            // set MF_MT_USER_DATA
            uint8_t data[MF_MT_USER_DATA_SIZE];
            memset(data, 0, MF_MT_USER_DATA_SIZE);

            // first 12 bytes correspond to HEAACWAVEINFO
            // https://msdn.microsoft.com/en-us/library/windows/desktop/dd757806%28v=vs.85%29.aspx
            data[0] = RAW_AAC_PAYLOAD;
            data[2] = AAC_LC_LEVEL;

            // last 2 bytes correspond to AudioSpecificConfig() data
            // http://wiki.multimedia.cx/index.php?title=MPEG-4_Audio#Audio_Specific_Config
            short audio_specific_config = (AUDIO_OBJECT_AAC_LC << 11) |
                                          (DEFAULT_SAMPLE_FREQUENCY_IDX << 7) |
                                          (MONO_NUM_CHANNELS << 3);

            // place audio_specific_config in the last 2 bytes
            data[MF_MT_USER_DATA_SIZE - 2] = (audio_specific_config & 0XFF00) >> 8;
            data[MF_MT_USER_DATA_SIZE - 1] = audio_specific_config & 0X00FF;

            mInputMediaType->SetBlob(MF_MT_USER_DATA, data, MF_MT_USER_DATA_SIZE);

            hr = mTransform->SetInputType(0, mInputMediaType, 0);

            // set output type
            if (SUCCEEDED(hr))
            {
                hr = MFCreateMediaType(&mOutputMediaType);

                if (SUCCEEDED(hr))
                {
                    UINT32 out_block_align
                        = MONO_NUM_CHANNELS * (DEFAULT_BITS_PER_SAMPLE / 8);

                    UINT32 out_bytes_per_sec = out_block_align * DEFAULT_SAMPLE_RATE;

                    // must be MFMediaType_Audio
                    mOutputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);

                    // 16-bit PCM audio
                    mOutputMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);

                    // match input
                    mOutputMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS,
                                                MONO_NUM_CHANNELS);

                    // match input
                    mOutputMediaType->SetUINT32(MF_MT_AUDIO_CHANNEL_MASK,
                                                SPEAKER_FRONT_CENTER);

                    // 48 kHz
                    mOutputMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                                DEFAULT_SAMPLE_RATE);

                    mOutputMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE,
                                                DEFAULT_BITS_PER_SAMPLE);

                    mOutputMediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT,
                                                out_block_align);

                    mOutputMediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND,
                                                out_bytes_per_sec);

                    mOutputMediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT,
                                                true);

                    hr = mTransform->SetOutputType(0, mOutputMediaType, 0);

                    // start processing input samples
                    if (SUCCEEDED(hr))
                    {
                        mTransform->GetInputStreamInfo(0, &mInputStreamInfo);
                        mTransform->GetOutputStreamInfo(0, &mOutputStreamInfo);

                        hr = mTransform->ProcessMessage(
                                 MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
                    }
                }
            }
        }
    }

    // clean up
    for (UINT32 i = 0; i < count; i++)
    {
        activate[i]->Release();
    }

    CoTaskMemFree(activate);

    return SUCCEEDED(hr);
}

int AacWindowsDecoder::Decode(const uint8_t* audioIn,
                              int numBytes,
                              int16_t* decoded)
{
    if ((audioIn == nullptr) ||
        (numBytes == 0))
    {
        return 0;
    }

    if (mTransform == nullptr)
    {
        return 0;
    }

    int num_samples_decoded = 0;

    // we need to create an IMFMediaBuffer for our IMFSample
    IMFMediaBuffer* input_buffer = nullptr;
    HRESULT hr = MFCreateMemoryBuffer(numBytes, &input_buffer);
    if (FAILED(hr))
    {
        SafeRelease(input_buffer);
        return num_samples_decoded;
    }

    // copy audioIn to media buffer
    BYTE* input_buffer_ptr = nullptr;
    input_buffer->Lock(&input_buffer_ptr, NULL, NULL);

    memcpy(input_buffer_ptr, audioIn, numBytes);
    input_buffer->SetCurrentLength(numBytes);

    input_buffer->Unlock();

    // create sample to decode
    IMFSample* input_sample = nullptr;
    hr = MFCreateSample(&input_sample);
    if (FAILED(hr))
    {
        SafeRelease(input_buffer);
        SafeRelease(input_sample);
        return num_samples_decoded;
    }

    // set sample properties
    input_sample->AddBuffer(input_buffer);

    DWORD output_status_flags;
    mTransform->GetOutputStatus(&output_status_flags);

    // decode
    hr = mTransform->ProcessInput(0, input_sample, 0);
    if (FAILED(hr))
    {
        SafeRelease(input_buffer);
        SafeRelease(input_sample);
        return num_samples_decoded;
    }

    // check if an output is available
    mTransform->GetOutputStatus(&output_status_flags);
    if ((output_status_flags & MFT_OUTPUT_STATUS_SAMPLE_READY) == 1)
    {
        // buffer for output sample
        IMFMediaBuffer* output_buffer = NULL;
        hr = MFCreateMemoryBuffer(mOutputStreamInfo.cbSize, &output_buffer);
        if (FAILED(hr))
        {
            SafeRelease(input_buffer);
            SafeRelease(input_sample);
            SafeRelease(output_buffer);
            return num_samples_decoded;
        }

        IMFSample* output_sample = NULL;
        hr = MFCreateSample(&output_sample);
        if (FAILED(hr))
        {
            SafeRelease(input_buffer);
            SafeRelease(input_sample);
            SafeRelease(output_buffer);
            SafeRelease(output_sample);
            return num_samples_decoded;
        }

        output_sample->AddBuffer(output_buffer);

        // get output samples from transform
        DWORD process_output_status;

        MFT_OUTPUT_DATA_BUFFER output_data = { 0 };
        output_data.pSample = output_sample;
        output_data.dwStreamID = 0;

        bool got_samples = false;
        do
        {
            hr = mTransform->ProcessOutput(0,
                                           1,
                                           &output_data,
                                           &process_output_status);

            if (SUCCEEDED(hr))
            {
                got_samples = true;
            }
        }
        while (hr != MF_E_TRANSFORM_NEED_MORE_INPUT);

        // copy to output buffer
        if (got_samples)
        {
            BYTE* output_buffer_ptr = nullptr;
            DWORD output_len = 0;

            output_buffer->Lock(&output_buffer_ptr, NULL, &output_len);

            num_samples_decoded = output_len / sizeof(short);
            memcpy((BYTE*) decoded, output_buffer_ptr, output_len);

            output_buffer->Unlock();
        }

        // clean up output
        SafeRelease(output_buffer);
        SafeRelease(output_sample);
    }

    return num_samples_decoded;
}

void AacWindowsDecoder::CleanUp()
{
    // clear out transform
    if (mTransform != nullptr)
    {
        mTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0);
        mTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);

        // TODO: maybe drain and process remaining output
    }

    SafeRelease(mInputMediaType);
    SafeRelease(mOutputMediaType);
    SafeRelease(mTransform);

    mInputStreamInfo = { 0 };
    mOutputStreamInfo = { 0 };
}
