#include "AacDefines.h"
#include "AacWindowsEncoder.h"

#include <Mfapi.h>
#include <Mferror.h>
#include <Windows.h>

AacWindowsEncoder::AacWindowsEncoder() :
    mTransform(nullptr),
    mInputMediaType(nullptr),
    mOutputMediaType(nullptr),
    mAudioSpecificConfig(nullptr)
{
    mInputStreamInfo = { 0 };
    mOutputStreamInfo = { 0 };

    mDurationInHundredNanoSeconds =
        (NUM_PCM_SAMPLES * (LONGLONG)10000000) / DEFAULT_SAMPLE_RATE;
}

AacWindowsEncoder::~AacWindowsEncoder()
{
    CleanUp();
}

bool AacWindowsEncoder::Init()
{
    HRESULT hr = S_OK;
    UINT32 count = 0;

    IMFActivate** activate = NULL;

    // we'll be looking for an AAC encoder
    MFT_REGISTER_TYPE_INFO info = { 0 };
    info.guidMajorType = MFMediaType_Audio;
    info.guidSubtype = MFAudioFormat_AAC;

    UINT32 flags = MFT_ENUM_FLAG_SYNCMFT |
                   MFT_ENUM_FLAG_ASYNCMFT |
                   MFT_ENUM_FLAG_LOCALMFT |
                   MFT_ENUM_FLAG_TRANSCODE_ONLY |
                   MFT_ENUM_FLAG_SORTANDFILTER;

    hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        return false;
    }

    // look for encoders that satisfy our conditions
    hr = MFTEnumEx(MFT_CATEGORY_AUDIO_ENCODER,
                   flags,
                   nullptr,
                   &info,
                   &activate,
                   &count);

    if (SUCCEEDED(hr) && count > 0)
    {
        // activate AAC encoder
        activate[0]->ActivateObject(IID_PPV_ARGS(&mTransform));

        // set input media type
        hr = MFCreateMediaType(&mInputMediaType);

        if (SUCCEEDED(hr))
        {
            UINT32 in_block_align
                = MONO_NUM_CHANNELS * (DEFAULT_BITS_PER_SAMPLE / 8);

            UINT32 in_bytes_per_sec = in_block_align * DEFAULT_SAMPLE_RATE;

            // must be MFMediaType_Audio
            mInputMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);

            // must be MFAudioFormat_PCM
            mInputMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);

            // must be 16
            mInputMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE,
                                       DEFAULT_BITS_PER_SAMPLE);

            // the following values are supported: 44.1 kHz and 48 kHz
            mInputMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                       DEFAULT_SAMPLE_RATE);

            // must be 1 (mono) or 2 (stereo)
            mInputMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS,
                                       MONO_NUM_CHANNELS);

            mInputMediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, in_block_align);

            mInputMediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND,
                                       in_bytes_per_sec);

            mInputMediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, true);

            hr = mTransform->SetInputType(0, mInputMediaType, 0);

            // set output type
            if (SUCCEEDED(hr))
            {
                hr = MFCreateMediaType(&mOutputMediaType);

                if (SUCCEEDED(hr))
                {
                    // loop through available output types
                    for (DWORD i = 0;; i++)
                    {
                        mTransform->GetOutputAvailableType(0, i, &mOutputMediaType);

                        hr = mTransform->SetOutputType(0, mOutputMediaType, 0);
                        if (SUCCEEDED(hr))
                        {
                            // get AudioSpecificConfig() data
                            // http://wiki.multimedia.cx/index.php?title=MPEG-4_Audio#Audio_Specific_Config
                            uint8_t* user_data = nullptr;
                            uint32_t user_data_size = 0;

                            mOutputMediaType->GetAllocatedBlob(MF_MT_USER_DATA,
                                                               &user_data,
                                                               &user_data_size);

                            mAudioSpecificConfig =
                                new uint8_t[AUDIO_SPECIFIC_CONFIG_LEN_IN_BYTES];

                            // AudioSpecificConfig() data is the last 2 bytes of MF_MT_USER_DATA
                            memcpy(mAudioSpecificConfig,
                                   user_data + user_data_size - AUDIO_SPECIFIC_CONFIG_LEN_IN_BYTES,
                                   AUDIO_SPECIFIC_CONFIG_LEN_IN_BYTES);

                            CoTaskMemFree(user_data);
                            break;
                        }

                        SafeRelease(mOutputMediaType);
                    }

                    // begin streaming
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

int AacWindowsEncoder::Encode(short* audioIn,
                              int inLen,
                              uint8_t* audioOut)
{
    DWORD output_len = 0;

    if (audioIn == nullptr)
    {
        return output_len;
    }

    if (mTransform == nullptr)
    {
        return output_len;
    }

    int len_in_bytes = inLen * sizeof(short);

    // we need to create an IMFMediaBuffer for our IMFSample
    IMFMediaBuffer* input_buffer = nullptr;
    HRESULT hr = MFCreateMemoryBuffer(len_in_bytes, &input_buffer);
    if (FAILED(hr))
    {
        SafeRelease(input_buffer);
        return -1;
    }

    // copy audioIn to media buffer
    BYTE* input_buffer_ptr = nullptr;
    input_buffer->Lock(&input_buffer_ptr, NULL, NULL);

    memcpy(input_buffer_ptr, (BYTE*)audioIn, len_in_bytes);
    input_buffer->SetCurrentLength(len_in_bytes);

    input_buffer->Unlock();

    // create sample to encode
    IMFSample* input_sample = nullptr;
    hr = MFCreateSample(&input_sample);
    if (FAILED(hr))
    {
        SafeRelease(input_buffer);
        SafeRelease(input_sample);
        return output_len;
    }

    // set sample properties
    input_sample->AddBuffer(input_buffer);
    input_sample->SetSampleDuration(mDurationInHundredNanoSeconds);
    input_sample->SetSampleTime(0);

    // encode
    hr = mTransform->ProcessInput(0, input_sample, 0);
    if (FAILED(hr))
    {
        SafeRelease(input_buffer);
        SafeRelease(input_sample);
        return output_len;
    }

    // check if an output is available
    DWORD output_status_flags;
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
            return output_len;
        }

        IMFSample* output_sample = NULL;
        hr = MFCreateSample(&output_sample);
        if (FAILED(hr))
        {
            SafeRelease(input_buffer);
            SafeRelease(input_sample);
            SafeRelease(output_buffer);
            SafeRelease(output_sample);
            return output_len;
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

            output_buffer->Lock(&output_buffer_ptr, NULL, &output_len);
            memcpy(audioOut, output_buffer_ptr, output_len);
            output_buffer->Unlock();
        }

        // clean up output
        SafeRelease(output_buffer);
        SafeRelease(output_sample);
    }

    // clean up input
    SafeRelease(input_buffer);
    SafeRelease(input_sample);

    return output_len;
}

void AacWindowsEncoder::CleanUp()
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

    delete[] mAudioSpecificConfig;
    mAudioSpecificConfig = nullptr;
}

unsigned char* AacWindowsEncoder::getSpecificInfo()
{
    return mAudioSpecificConfig;
}

unsigned long AacWindowsEncoder::getSpecificInfoLen()
{
    return AUDIO_SPECIFIC_CONFIG_LEN_IN_BYTES;
}
