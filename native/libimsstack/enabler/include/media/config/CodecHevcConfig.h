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

#ifndef CODEC_HEVC_CONFIG_H_
#define CODEC_HEVC_CONFIG_H_

#include "config/CodecConfig.h"

class CodecHevcConfig : public CodecConfig
{
public:
    /**
     * @brief Construct a new codec hevc config
     *
     * @param nType_ video codec type - hevc (H.265)
     * @param nPayloadTypeNum_ payload type number
     */
    CodecHevcConfig(IN IMS_SINT32 nType_, IN IMS_SINT32 nPayloadTypeNum_);
    /**
     * @brief Destroy the codec hevc config
     *
     */
    virtual ~CodecHevcConfig();
    /**
     * @brief Create codec using the configuration
     *
     * @param piCc configuration
     * @param nCodecIdx codec index within each codec type
     * @return IMS_BOOL Return true if the create function is executed without error
     * Return false if the create function is failed
     */
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc, IN IMS_SINT32 nCodecIdx) override;
    /**
     * @brief Print debug string
     *
     */
    virtual void ToDebugString() const override;
    /**
     * @brief Get the channel
     *
     * @return IMS_SINT32 Return the channel id - default : 0
     */
    IMS_SINT32 GetChannel() const;
    /**
     * @brief Get the resolution width
     *
     * @return IMS_SINT32 Return the resolution configuration - width for hevc
     */
    IMS_SINT32 GetResolutionWidth() const;
    /**
     * @brief Get the resolution height
     *
     * @return IMS_SINT32 Return the resolution configuration - height for hevc
     */
    IMS_SINT32 GetResolutionHeight() const;
    /**
     * @brief Get the framerate for hevc
     *
     * @return IMS_SINT32 Return hevc framerate
     */
    IMS_SINT32 GetFramerate() const;
    /**
     * @brief Get the bitrate for hevc
     *
     * @return IMS_SINT32 Return hevc bitrate
     */
    IMS_SINT32 GetBitrate() const;
    /**
     * @brief Get the packetization mode
     *
     * @return IMS_SINT32 Return the packetization mode
     */
    IMS_SINT32 GetPacketizationMode() const;
    /**
     * @brief Get the SpropParameterSets
     *
     * @return const AString& SpropParameterSets
     */
    const AString& GetSpropParameterSets() const;
    /**
     * @brief Get the hevc profile
     *
     * @return IMS_SINT32 Return hevc profile
     */
    IMS_SINT32 GetHevcProfile() const;
    /**
     * @brief Get the hevc level
     *
     * @return IMS_SINT32 Return hevc level
     */
    IMS_SINT32 GetHevcLevel() const;
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
    static const IMS_SINT32 NEED_TO_CHECK_I = 0;
    static const IMS_SINT32 DEFAULT_CHANNEL = 0;
    static const IMS_SINT32 DEFAULT_HEVC_RESOLUTION_WIDTH = 720;
    static const IMS_SINT32 DEFAULT_HEVC_RESOLUTION_HEIGHT = 1280;
    static const IMS_SINT32 DEFAULT_HEVC_FRAMERATE = 30;
    static const IMS_SINT32 DEFAULT_HEVC_BITRATE = NEED_TO_CHECK_I;
    static const IMS_SINT32 DEFAULT_PACKETIZATION_MODE = NEED_TO_CHECK_I;
    static const IMS_SINT32 DEFAULT_HEVC_PROFILE = 1;
    static const IMS_SINT32 DEFAULT_HEVC_LEVEL = 93;

private:
    IMS_SINT32 m_nChannel;
    IMS_SINT32 m_nResolutionWidth;
    IMS_SINT32 m_nResolutionHeight;
    IMS_SINT32 m_nFramerate;
    IMS_SINT32 m_nBitrate;
    IMS_SINT32 m_nPacketizationMode;
    AString m_strSpropParameterSets;
    IMS_SINT32 m_nHevcProfile;
    IMS_SINT32 m_nHevcLevel;
    AString m_strImageAttr;
    AString m_strFrameSize;
};

#endif
