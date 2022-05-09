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

#ifndef _CODEC_AVC_CONFIG_H_
#define _CODEC_AVC_CONFIG_H_

#include "config/CodecConfig.h"

class CodecAvcConfig : public CodecConfig
{
public:
    CodecAvcConfig(IN IMS_SINT32 nType_, IN IMS_SINT32 nPayloadTypeNum_);
    virtual ~CodecAvcConfig();

public:
    // CodecConfig class
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc);
    virtual void ToDebugString() const;

    IMS_SINT32 GetChannel() const;
    IMS_SINT32 GetResolutionWidth() const;
    IMS_SINT32 GetResolutionHeight() const;
    IMS_SINT32 GetFramerate() const;
    IMS_SINT32 GetBitrate() const;
    IMS_SINT32 GetPacketizationMode() const;
    IMS_BOOL GetIncludeSpropParameterSets() const;
    const AString& GetSpropParameterSets() const;
    const AString& GetProfileLevelId() const;
    const AString& GetImageAttr() const;
    const AString& GetFrameSize() const;

public:
    enum
    {
        SINGLE_NAL_UNIT_MODE = 0,
        NON_INTERLEAVED_MODE = 1,
    };

    static const IMS_SINT32 DEFAULT_CHANNEL = 0;
    static const IMS_SINT32 DEFAULT_RESOLUTION_WIDTH = 240;
    static const IMS_SINT32 DEFAULT_RESOLUTION_HEIGHT = 320;
    static const IMS_SINT32 DEFAULT_FRAMERATE = 15;
    static const IMS_SINT32 DEFAULT_BITRATE = 384;
    static const IMS_SINT32 DEFAULT_PACKETIZATION_MODE = NON_INTERLEAVED_MODE;
    static const IMS_BOOL DEFAULT_INCLUDE_SPROP = IMS_TRUE;
#define DEFAULT_AVC_SPROP_PARAMS "Z0LAFukDwKMg,aM4G4g=="
#define DEFAULT_AVC_PROFILE_ID   "42C00C"
#define DEFAULT_AVC_IMAGE_ATTR \
    "send [x=320,y=240] [x=640,y=480] recv [x=320,y=240] [x=640,y=480] [x=1280,y=720]"
#define DEFAULT_AVC_FRAME_SIZE "NEED_TO_CHECK"

private:
    IMS_SINT32 m_nChannel;

    IMS_SINT32 m_nResolutionWidth;
    IMS_SINT32 m_nResolutionHeight;
    IMS_SINT32 m_nFramerate;
    IMS_SINT32 m_nBitrate;
    IMS_SINT32 m_nPacketizationMode;

    IMS_BOOL m_bIncludeSpropParameterSets;
    AString m_strSpropParameterSets;
    AString m_strProfileLevelId;
    AString m_strImageAttr;
    AString m_strFrameSize;
};
#endif  // _CODEC_AVC_CONFIG_H_
