/*
 *  Copyright (c) 2015 Personify, Inc. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_AUDIO_CODING_CODECS_AAC_MAIN_INTERFACE_AAC_INTERFACE_H_
#define WEBRTC_MODULES_AUDIO_CODING_CODECS_AAC_MAIN_INTERFACE_AAC_INTERFACE_H_

#include "webrtc/typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

// Opaque wrapper types for the codec state.
typedef struct WebRtcAacEncInst AacEncInst;
typedef struct WebRtcAacDecInst AacDecInst;

int16_t WebRtcAac_EncoderCreate(AacEncInst** enc);

int16_t WebRtcAac_EncoderFree(AacEncInst* enc);

int16_t WebRtcAac_EncoderInit(AacEncInst* enc);

int16_t WebRtcAac_Encode(AacEncInst* enc,
                         int16_t* audioIn,
                         int16_t numSamples,
                         uint8_t* encoded);

int16_t WebRtcAac_DecoderCreate(AacDecInst** dec);

int16_t WebRtcAac_DecoderFree(AacDecInst* dec);

int16_t WebRtcAac_DecoderInit(AacDecInst* dec);

int16_t WebRtcAac_Decode(AacDecInst* dec,
                         const uint8_t* audioIn,
                         int16_t numBytes,
                         int16_t* decoded);

#ifdef __cplusplus
}
#endif

#endif
