/*
 *  Copyright (c) 2014 Personify, Inc. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */



#include <stdlib.h>
#include <string.h>
#include "speex_interface.h"

#include <speex/speex.h>

struct SPEEX_encinst_t_
{
    void* state;
    SpeexBits bitstream;
    int frame_size;
    int sampling_rate;
};

struct SPEEX_decinst_t_
{
    void* state;
    SpeexBits bitstream;
    int frame_size;
    int sampling_rate;
    int enh_enabled;
};

int16_t WebRtcSpeex_CreateEnc(SPEEX_encinst_t** enc,
                              int32_t fs)
{
    *enc = (SPEEX_encinst_t*)malloc(sizeof(SPEEX_encinst_t));
    if (*enc == NULL)
    {
        return -1;
    }
    memset(*enc, 0, sizeof(SPEEX_encinst_t));
    (*enc)->sampling_rate = fs;

    return 0;
}

int16_t WebRtcSpeex_EncoderInit(SPEEX_encinst_t* enc,
                                int16_t vbr, int16_t complexity,
                                int16_t vad_enable)
{
    const SpeexMode* mode = NULL;
    int quality = 8; //Set the quality to 8 (15 kbps)

    speex_bits_init(&(enc->bitstream));
    if (enc->sampling_rate == 16000)
    {
        mode = speex_lib_get_mode(SPEEX_MODEID_WB);
    }
    else if (enc->sampling_rate == 8000)
    {
        mode = speex_lib_get_mode(SPEEX_MODEID_NB);
    }
    enc->state = speex_encoder_init(mode);
    if (speex_encoder_ctl(enc->state, SPEEX_SET_SAMPLING_RATE, &(enc->sampling_rate)) < 0)
    {
        return -1;
    }

    if (speex_encoder_ctl(enc->state, SPEEX_SET_QUALITY, (void*)&quality) < 0)
    {
        return -1;
    }

    if (speex_encoder_ctl(enc->state, SPEEX_SET_VBR, (void*)&vbr) < 0)
    {
        return -1;
    }

    if (speex_encoder_ctl(enc->state, SPEEX_SET_COMPLEXITY, (void*)&complexity) < 0)
    {
        return -1;
    }

    if (speex_encoder_ctl(enc->state, SPEEX_SET_VAD, (void*)&vad_enable) < 0)
    {
        return -1;
    }

    if (speex_encoder_ctl(enc->state, SPEEX_SET_DTX, (void*)&vad_enable) < 0)
    {
        return -1;
    }

    //Frame size = the number of audio samples
    speex_encoder_ctl(enc->state, SPEEX_GET_FRAME_SIZE, &(enc->frame_size));

    return 0;
}

int16_t WebRtcSpeex_FreeEnc(SPEEX_encinst_t* enc)
{
    if (enc)
    {
        speex_bits_destroy(&(enc->bitstream));
        speex_encoder_destroy(enc->state);
        free(enc);
        return 0;
    }
    else
    {
        return -1;
    }
}

int16_t WebRtcSpeex_Encode(SPEEX_encinst_t* enc,
                           int16_t* speechIn,
                           int32_t rate)
{
    speex_bits_reset(&(enc->bitstream));
    //Encode the frame
    return speex_encode_int(enc->state, speechIn, &(enc->bitstream));
}

int16_t WebRtcSpeex_GetBitstream(SPEEX_encinst_t* enc,
                                 int16_t* encoded)
{
    return speex_bits_write(&(enc->bitstream), (char*)encoded, 70);
}

int16_t WebRtcSpeex_CreateDec(SPEEX_decinst_t** dec,
                              int32_t fs,
                              int16_t enh_enabled)
{
    *dec = (SPEEX_decinst_t*)malloc(sizeof(SPEEX_decinst_t));

    if (*dec == NULL)
    {
        return -1;
    }
    memset(*dec, 0, sizeof(SPEEX_decinst_t));

    (*dec)->enh_enabled = enh_enabled;
    (*dec)->sampling_rate = fs;

    return 0;
}


int16_t WebRtcSpeex_DecoderInit(SPEEX_decinst_t* dec)
{
    const SpeexMode* mode = NULL;
    speex_bits_init(&(dec->bitstream));
    if (dec->sampling_rate == 16000)
    {
        mode = speex_lib_get_mode(SPEEX_MODEID_WB);
    }
    else if (dec->sampling_rate == 8000)
    {
        mode = speex_lib_get_mode(SPEEX_MODEID_NB);
    }
    dec->state = speex_decoder_init(mode);
    if (dec->state == NULL)
    {
        return -1;
    }
    if (speex_decoder_ctl(dec->state, SPEEX_SET_SAMPLING_RATE, &(dec->sampling_rate)) < 0)
    {
        return -1;
    }
    if (speex_decoder_ctl(dec->state, SPEEX_SET_ENH, &(dec->enh_enabled)) < 0)
    {
        return -1;
    }
    //Frame size = the number of audio samples
    if (speex_decoder_ctl(dec->state, SPEEX_GET_FRAME_SIZE, &(dec->frame_size)) < 0)
    {
        return -1;
    }

    return 0;
}

int16_t WebRtcSpeex_FreeDec(SPEEX_decinst_t* dec)
{
    if (dec)
    {
        speex_bits_destroy(&(dec->bitstream));
        speex_decoder_destroy(dec->state);

        free(dec);
        return 0;
    }
    else
    {
        return -1;
    }
}

int16_t WebRtcSpeex_Decode(SPEEX_decinst_t* dec,
                           int16_t* encoded,
                           int16_t len,
                           int16_t* decoded,
                           int16_t* speechType)
{
    int ret = 0;
    int n_decoded = 0;
    speex_bits_read_from(&(dec->bitstream), (char*)encoded, len);
    while (ret == 0)
    {
        ret = speex_decode_int(dec->state, &(dec->bitstream), decoded + n_decoded);
        if (ret == 0)
        {
            n_decoded += dec->frame_size;
        }
    }
    return n_decoded;
}

int16_t WebRtcSpeex_Version(char* versionStr, short len)
{
    // Get version string
    char version[30] = "1.2rc1\n";
    if (strlen(version) < (unsigned int)len)
    {
        strcpy(versionStr, version);
        return 0;
    }
    else
    {
        return -1;
    }
}

int16_t WebRtcSpeex_DecodePlc(SPEEX_decinst_t* dec,
                              int16_t* decoded, int16_t noOfLostFrames)
{
    return -1;
}
