# Copyright (c) 2014 Personify, Inc. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

{
  'targets': [
    {
      'target_name': 'SPEEX',
      'type': 'static_library',
      'include_dirs': [
        'include',
        '<(DEPTH)/../../../psydependencies/speex-1.2rc1/include',
        '<(webrtc_root)',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          'include',
          '<(webrtc_root)',
        ],
      },
      'sources': [
        'include/speex_interface.h',
        'speex_interface.c',
      ],
      'link_settings': {
        'libraries':[
          '<(DEPTH)/../../../psydependencies/speex-1.2rc1/lib/win32-static/libspeex.lib'
        ],
      },
    },
  ], # targets
}
