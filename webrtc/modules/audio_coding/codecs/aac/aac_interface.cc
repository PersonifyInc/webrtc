/*
*  Copyright (c) 2015 Personify, Inc. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include "AacWindowsDecoder.h"
#include "AacWindowsEncoder.h"
#include "aac_interface.h"

struct WebRtcAacEncInst
{
    AacWindowsEncoder* encoder;
};

struct WebRtcAacDecInst
{
    AacWindowsDecoder* decoder;
};

int16_t WebRtcAac_EncoderCreate(AacEncInst** enc)
{
    *enc = (AacEncInst*)malloc(sizeof(AacEncInst));
    if (*enc == nullptr)
    {
        return -1;
    }

    (*enc)->encoder = new AacWindowsEncoder();
    if ((*enc)->encoder == nullptr)
    {
        return -1;
    }

    return 0;
}

int16_t WebRtcAac_EncoderFree(AacEncInst* enc)
{
    if (enc != nullptr)
    {
        enc->encoder->CleanUp();

        delete enc->encoder;
        enc->encoder = nullptr;

        free(enc);
        return 0;
    }

    return -1;
}

int16_t WebRtcAac_EncoderInit(AacEncInst* enc)
{
    if (enc != nullptr)
    {
        if (enc->encoder->Init())
        {
            return 0;
        }
    }

    return -1;
}

int16_t WebRtcAac_Encode(AacEncInst* enc,
                         int16_t* audioIn,
                         int16_t numSamples,
                         uint8_t* encoded)
{
    if (enc == nullptr)
    {
        return 0;
    }

    return enc->encoder->Encode(audioIn,
                                numSamples,
                                encoded);
}

int16_t WebRtcAac_DecoderCreate(AacDecInst** dec)
{
    *dec = (AacDecInst*)malloc(sizeof(AacDecInst));
    if (*dec == nullptr)
    {
        return -1;
    }

    (*dec)->decoder = new AacWindowsDecoder();
    if ((*dec)->decoder == nullptr)
    {
        return -1;
    }

    return 0;
}

int16_t WebRtcAac_DecoderFree(AacDecInst* dec)
{
    if (dec != nullptr)
    {
        dec->decoder->CleanUp();

        delete dec->decoder;
        dec->decoder = nullptr;

        free(dec);
        return 0;
    }

    return -1;
}

int16_t WebRtcAac_DecoderInit(AacDecInst* dec)
{
    if (dec != nullptr)
    {
        if (dec->decoder->Init())
        {
            return 0;
        }
    }

    return -1;
}

int16_t WebRtcAac_Decode(AacDecInst* dec,
                         const uint8_t* audioIn,
                         int16_t numBytes,
                         int16_t* decoded)
{
    if (dec == nullptr)
    {
        return 0;
    }

    return dec->decoder->Decode(audioIn, numBytes, decoded);
}
