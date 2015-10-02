# Copyright (c) 2015 Personify, Inc. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

{
  'includes': [
    '../../webrtc/build/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'vo_aacenc',
      'type': 'static_library',
      'include_dirs': [
        'aacenc/basic_op',
        'aacenc/inc',
        'common/include',
      ],
      'sources': [
        'aacenc/basic_op/basicop2.c',
        'aacenc/basic_op/oper_32b.c',
        'aacenc/src/aac_rom.c',
        'aacenc/src/aacenc.c',
        'aacenc/src/aacenc_core.c',
        'aacenc/src/adj_thr.c',
        'aacenc/src/band_nrg.c',
        'aacenc/src/bit_cnt.c',
        'aacenc/src/bitbuffer.c',
        'aacenc/src/bitenc.c',
        'aacenc/src/block_switch.c',
        'aacenc/src/channel_map.c',
        'aacenc/src/dyn_bits.c',
        'aacenc/src/grp_data.c',
        'aacenc/src/interface.c',
        'aacenc/src/line_pe.c',
        'aacenc/src/memalign.c',
        'aacenc/src/ms_stereo.c',
        'aacenc/src/pre_echo_control.c',
        'aacenc/src/psy_configuration.c',
        'aacenc/src/psy_main.c',
        'aacenc/src/qc_main.c',
        'aacenc/src/quantize.c',
        'aacenc/src/sf_estim.c',
        'aacenc/src/spreading.c',
        'aacenc/src/stat_bits.c',
        'aacenc/src/tns.c',
        'aacenc/src/transform.c',
        'common/cmnMemory.c'
      ],
      'conditions': [
        ['target_arch=="arm"', {
          'include_dirs': [
            'aacenc/src/asm/ARMV5E',
          ],
          'sources': [
            'aacenc/src/asm/ARMV5E/AutoCorrelation_v5.s',
            'aacenc/src/asm/ARMV5E/band_nrg_v5.s',
            'aacenc/src/asm/ARMV5E/CalcWindowEnergy_v5.s',
          ],
          'conditions': [
            ['build_with_neon == 0', {
              'sources': [
                'aacenc/src/asm/ARMV5E/PrePostMDCT_v5.s',
                'aacenc/src/asm/ARMV5E/R4R8First_v5.s',
                'aacenc/src/asm/ARMV5E/Radix4FFT_v5.s',
              ],
             }],
             ['build_with_neon == 1', {
              'includes': ['../../webrtc/build/arm_neon.gypi',],
              'include_dirs': [
                'aacenc/src/asm/ARMV7',
              ],
              'sources': [
                'aacenc/src/asm/ARMV7/PrePostMDCT_v7.s',
                'aacenc/src/asm/ARMV7/R4R8First_v7.s',
                'aacenc/src/asm/ARMV7/Radix4FFT_v7.s',
              ],
             }],
          ],
        }],
      ],
    },
  ], # targets
}
