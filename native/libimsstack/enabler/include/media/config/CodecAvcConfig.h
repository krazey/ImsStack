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
    /**
     * @brief Construct a new codec avc config object
     *
     * @param nType_ video codec type - avc (H.264)
     * @param nPayloadTypeNum_ payload type number
     */
    CodecAvcConfig(IN IMS_SINT32 nType_, IN IMS_SINT32 nPayloadTypeNum_);
    /**
     * @brief Destroy the codec avc config object
     *
     */
    virtual ~CodecAvcConfig();
    /**
     * @brief Create codec using the configuration
     *
     * @param piCc configuration
     * @param nCodecIdx codec index within each codec type
     * @return IMS_BOOL Return true if the create function is executed without error
     * Return false if the create function is failed
     */
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc, IN IMS_SINT32 nCodecIdx);
    /**
     * @brief Print debug strings
     *
     */
    virtual void ToDebugString() const;
    /**
     * @brief Get the channel
     *
     * @return IMS_SINT32 Return the channel id - default : 0
     */
    IMS_SINT32 GetChannel() const;
    /**
     * @brief Get the resolution width
     *
     * @return IMS_SINT32 Return the resolution configuration - width
     */
    IMS_SINT32 GetResolutionWidth() const;
    /**
     * @brief Get the resolution height
     *
     * @return IMS_SINT32 Return the resolution configuration - height
     */
    IMS_SINT32 GetResolutionHeight() const;
    /**
     * @brief Get the framerate
     *
     * @return IMS_SINT32 Return the framerate
     */
    IMS_SINT32 GetFramerate() const;
    /**
     * @brief Get the bitrate
     *
     * @return IMS_SINT32 Return the bitrate configuration
     */
    IMS_SINT32 GetBitrate() const;
    /**
     * @brief Get the packetization mode (the packetization rule for the incoming stream connection)
     *
     * @return IMS_SINT32 Return the packetization mode
     */
    IMS_SINT32 GetPacketizationMode() const;
    /**
     * @brief Get the include sprop parameter sets
     *
     * @return IMS_BOOL Return true if m_bIncludeSpropParameterSets is enabled
     * Return false if m_bIncludeSpropParameterSets is disabled
     */
    IMS_BOOL GetIncludeSpropParameterSets() const;
    /**
     * @brief Get the sprop-parameter-sets
     *
     * @return const AString& Return the sprop parameter sets
     */
    const AString& GetSpropParameterSets() const;
    /**
     * @brief Get the profile level id
     *
     * @return const AString& Return profile level-id
     */
    const AString& GetProfileLevelId() const;
    /**
     * @brief Get the image attribute
     *
     * @return const AString& Return image-attribute
     */
    const AString& GetImageAttr() const;
    /**
     * @brief Get the frame size
     *
     * @return const AString& Return framesize
     */
    const AString& GetFrameSize() const;

public:
    /** Enum for packetization mode */
    enum
    {
        /** must contain only one NAL unit of the types, default value */
        SINGLE_NAL_UNIT_MODE = 0,
        /** should be supported, only single NAL unit packets, STAP-As, and FU-As MAY be used in
           this mode*/
        NON_INTERLEAVED_MODE = 1,
        /** may be supported, STAP-Bs, MTAPs, FU-As, and FU-Bs MAY be use*/
        INTERLEAVED_MODE = 2,
    };

    static const IMS_SINT32 DEFAULT_CHANNEL = 0;
    static const IMS_SINT32 DEFAULT_RESOLUTION_WIDTH = 240;
    static const IMS_SINT32 DEFAULT_RESOLUTION_HEIGHT = 320;
    static const IMS_SINT32 DEFAULT_FRAMERATE = 15;
    static const IMS_SINT32 DEFAULT_BITRATE = 384;
    static const IMS_SINT32 DEFAULT_PACKETIZATION_MODE = NON_INTERLEAVED_MODE;
    static const IMS_BOOL DEFAULT_INCLUDE_SPROP = IMS_TRUE;

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
