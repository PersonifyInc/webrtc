# Copyright (c) 2015 Personify, Inc. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

{
  'targets': [
    {
      'target_name': 'webrtc_aac',
      'type': 'static_library',
      'include_dirs': [
        'include',
        '<(webrtc_root)',
        '<(DEPTH)'
      ],
      'sources': [
        'aac_interface.cc',
        'audio_encoder_aac.cc',
        'include/AacDefines.h',
        'include/aac_interface.h',
        'include/audio_encoder_aac.h',
      ],

      'conditions': [
        ['OS=="win"', {
          'sources': [
            'AacWindowsDecoder.cpp',
            'AacWindowsEncoder.cpp',
            'include/AacWindowsDecoder.h',
            'include/AacWindowsEncoder.h',
          ],
          'link_settings': {
            'libraries':[
              '-lmfplat.lib',
              '-lmfuuid.lib'
            ],
          },
        }],
        ['OS=="android"', {
          'dependencies': [
            '<(DEPTH)/third_party/vo-aacenc/vo-aacenc.gyp:*',
          ],
          'sources': [
            'AacVisualOnEncoder.cpp',
            'include/AacVisualOnEncoder.h',
          ],
        }],
      ],
    },
  ], # targets
}
