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
      ],
      'sources': [
        'AacWindowsDecoder.cpp',
        'AacWindowsEncoder.cpp',
        'aac_interface.cc',
        'include/AacDefines.h',
        'include/AacWindowsDecoder.h',
        'include/AacWindowsEncoder.h',
        'include/aac_interface.h'
      ],
      'link_settings': {
        'libraries':[
          '-lmfplat.lib',
          '-lmfuuid.lib'
        ],
      },
    },
  ], # targets
}
