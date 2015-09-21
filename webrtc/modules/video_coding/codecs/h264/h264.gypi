# Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

{
  'includes': [
    '../../../../build/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'webrtc_h264',
      'type': 'static_library',
      'conditions': [
        ['OS=="ios"', {
          'dependencies': [
            'webrtc_h264_video_toolbox',
          ],
          'sources': [
            'h264_objc.mm',
          ],
        }],
        ['OS=="android"', {
          'dependencies': [
            'webrtc_h264_media_codec',
          ],
        }],
      ],
      'sources': [
        'h264.cc',
        'include/h264.h',
      ],
    }, # webrtc_h264
  ],
  'conditions': [
    ['OS=="ios"', {
      'targets': [
        {
          'target_name': 'webrtc_h264_video_toolbox',
          'type': 'static_library',
          'dependencies': [
            '<(DEPTH)/third_party/libyuv/libyuv.gyp:libyuv',
          ],
          'link_settings': {
            'xcode_settings': {
              'OTHER_LDFLAGS': [
                '-framework CoreMedia',
                '-framework CoreVideo',
                '-framework VideoToolbox',
              ],
            },
          },
          'sources': [
            'h264_video_toolbox_decoder.cc',
            'h264_video_toolbox_decoder.h',
            'h264_video_toolbox_encoder.cc',
            'h264_video_toolbox_encoder.h',
            'h264_video_toolbox_nalu.cc',
            'h264_video_toolbox_nalu.h',
          ],
        }, # webrtc_h264_video_toolbox
      ], # targets
    }], # OS=="ios"
    ['OS=="android"', {
      'targets': [
        {
          'target_name': 'webrtc_h264_media_codec',
          'type': 'static_library',
          'dependencies': [
            '<(DEPTH)/third_party/libyuv/libyuv.gyp:libyuv',
            '<(DEPTH)/third_party/icu/icu.gyp:icuuc',
            '<(webrtc_root)/common_video/common_video.gyp:common_video',
            '<(webrtc_root)/modules/video_coding/utility/video_coding_utility.gyp:video_coding_utility',
            '<(webrtc_root)/system_wrappers/system_wrappers.gyp:system_wrappers'
          ],
          'sources': [
            'h264_media_codec_encoder.cc',
            'h264_media_codec_encoder.h',
            '<(DEPTH)/talk/app/webrtc/java/jni/androidmediacodeccommon.h',
            '<(DEPTH)/talk/app/webrtc/java/jni/androidmediaencoder_jni.cc',
            '<(DEPTH)/talk/app/webrtc/java/jni/androidmediaencoder_jni.h',
            '<(DEPTH)/talk/app/webrtc/java/jni/classreferenceholder.cc',
            '<(DEPTH)/talk/app/webrtc/java/jni/classreferenceholder.h',
            '<(DEPTH)/talk/app/webrtc/java/jni/jni_helpers.cc',
            '<(DEPTH)/talk/app/webrtc/java/jni/jni_helpers.h',
            '<(DEPTH)/talk/app/webrtc/java/jni/native_handle_impl.h',
          ],
        }, # webrtc_h264_media_codec
      ], # targets
    }], # OS=="android"
  ], # conditions
}
