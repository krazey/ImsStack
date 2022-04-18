/**
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _CODEC_HEVC_CONFIG_H_
#define _CODEC_HEVC_CONFIG_H_

#include "config/CodecConfig.h"

class CodecHevcConfig :
        public CodecConfig
{
public:
    CodecHevcConfig(IN IMS_SINT32 nType_, IN IMS_SINT32 nPayloadTypeNum_);
    virtual ~CodecHevcConfig();

public:
    // CodecConfig class
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc);
    virtual void ToDebugString() const;

    IMS_SINT32 GetResolutionWidth() const;
    IMS_SINT32 GetResolutionHeight() const;
    IMS_SINT32 GetFramerate() const;
    IMS_SINT32 GetBitrate() const;
    IMS_SINT32 GetPacketizationMode() const;
    IMS_BOOL GetIncludeSpropParameterSets() const;
    IMS_SINT32 GetHevcProfile() const;
    IMS_SINT32 GetHevcLevel() const;
    const AString& GetImageAttr() const;
    const AString& GetFrameSize() const;

public:
    static const IMS_SINT32 NEED_TO_CHECK_I = 0;

    static const IMS_SINT32 DEFAULT_RESOLUTION_WIDTH = NEED_TO_CHECK_I;
    static const IMS_SINT32 DEFAULT_RESOLUTION_HEIGHT = NEED_TO_CHECK_I;
    static const IMS_SINT32 DEFAULT_FRAMERATE = NEED_TO_CHECK_I;
    static const IMS_SINT32 DEFAULT_BITRATE = NEED_TO_CHECK_I;
    static const IMS_SINT32 DEFAULT_PACKETIZATION_MODE = NEED_TO_CHECK_I;
    static const IMS_BOOL DEFAULT_INCLUDE_SPROP = IMS_FALSE;
    static const IMS_SINT32 DEFAULT_HEVC_PROFILE = 1;
    static const IMS_SINT32 DEFAULT_HEVC_LEVEL = 93;
    #define DEFAULT_IMAGE_ATTR "NEED_TO_CHECK"
    #define DEFAULT_FRAME_SIZE "NEED_TO_CHECK"

private:
    IMS_SINT32 m_nResolutionWidth;
    IMS_SINT32 m_nResolutionHeight;
    IMS_SINT32 m_nFramerate;
    IMS_SINT32 m_nBitrate;
    IMS_SINT32 m_nPacketizationMode;
    IMS_BOOL m_bIncludeSpropParameterSets;
    IMS_SINT32 m_nHevcProfile;
    IMS_SINT32 m_nHevcLevel;
    AString m_strImageAttr;
    AString m_strFrameSize;
};
#endif                                              // _CODEC_HEVC_CONFIG_H_
