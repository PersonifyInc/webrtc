/*
 *  Copyright (c) 2014 Personify, Inc. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_CODING_CODECS_SPEEX_MAIN_INTERFACE_SPEEX_INTERFACE_H_
#define MODULES_AUDIO_CODING_CODECS_SPEEX_MAIN_INTERFACE_SPEEX_INTERFACE_H_

#include "typedefs.h"

/*
 * Solution to support multiple instances
 */

// forward declaration
typedef struct SPEEX_encinst_t_ SPEEX_encinst_t;
typedef struct SPEEX_decinst_t_ SPEEX_decinst_t;

#ifdef __cplusplus
extern "C" {
#endif

int16_t WebRtcSpeex_CreateEnc(SPEEX_encinst_t** enc,
                              int32_t fs);

int16_t WebRtcSpeex_EncoderInit(SPEEX_encinst_t* enc,
                                int16_t vbr, int16_t complexity,
                                int16_t vad_enable);

int16_t WebRtcSpeex_FreeEnc(SPEEX_encinst_t* enc);

int16_t WebRtcSpeex_Encode(SPEEX_encinst_t* enc,
                           int16_t* speechIn,
                           int32_t rate);

int16_t WebRtcSpeex_GetBitstream(SPEEX_encinst_t* enc,
                                 int16_t* encoded);

int16_t WebRtcSpeex_CreateDec(SPEEX_decinst_t** dec,
                              int32_t fs,
                              int16_t enh_enabled);

int16_t WebRtcSpeex_DecoderInit(SPEEX_decinst_t* dec);

int16_t WebRtcSpeex_FreeDec(SPEEX_decinst_t* dec);

int16_t WebRtcSpeex_Decode(SPEEX_decinst_t* dec,
                           int16_t* encoded,
                           int16_t len,
                           int16_t* decoded,
                           int16_t* speechType);

int16_t WebRtcSpeex_Version(char* versionStr, short len);

int16_t WebRtcSpeex_DecodePlc(SPEEX_decinst_t* dec,
                              int16_t* decoded, int16_t noOfLostFrames);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_AUDIO_CODING_CODECS_SPEEX_MAIN_INTERFACE_SPEEX_INTERFACE_H_ */
