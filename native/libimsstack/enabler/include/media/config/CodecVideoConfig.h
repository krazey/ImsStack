/**
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef CODEC_VIDEO_CONFIG_H_
#define CODEC_VIDEO_CONFIG_H_

#include "config/CodecConfig.h"

class CodecVideoConfig : public CodecConfig
{
public:
    /**
     * @brief Construct a new common video codec config
     *
     * @param nType video codec type
     * @param nPayloadTypeNum payload type number
     * @param nResolutionWidth video resolution width
     * @param nResolutionHeight video resolution height
     * @param nFramerate video frame rate
     * @param nBitrate video bitrate
     * @param strSpropParameterSets sprop parameter sets
     * @param strImageAttr image attribute for the video resolution
     * @param strFrameSize framesize
     */
    CodecVideoConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum,
            IN IMS_SINT32 nResolutionWidth, IN IMS_SINT32 nResolutionHeight,
            IN IMS_SINT32 nFramerate, IN IMS_SINT32 nBitrate, IN AString strSpropParameterSets,
            IN AString strImageAttr, IN AString strFrameSize);

    /**
     * @brief Destroy the codec video config
     */
    ~CodecVideoConfig() override;

    /**
     * @brief Create codec using the configuration
     *
     * @param piCc configuration
     * @return IMS_BOOL Return true if the create function is executed without error
     * Return false if the create function is failed
     */
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc) override;

    /**
     * @brief Print debug string
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
     * @return IMS_SINT32 Return the resolution configuration - width for avc/hevc
     */
    IMS_SINT32 GetResolutionWidth() const;

    /**
     * @brief Get the resolution height
     *
     * @return IMS_SINT32 Return the resolution configuration - height for avc/hevc
     */
    IMS_SINT32 GetResolutionHeight() const;

    /**
     * @brief Get the framerate for avc/hevc
     *
     * @return IMS_SINT32 Return avc/hevc framerate
     */
    IMS_SINT32 GetFramerate() const;
    /**
     * @brief Get the bitrate for avc/hevc
     *
     * @return IMS_SINT32 Return avc/hevc bitrate
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
     * @brief Get the image attribute
     *
     * @return const AString& Return a string of the image attribute for video resolution
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
    static const IMS_SINT32 DEFAULT_PACKETIZATION_MODE = NON_INTERLEAVED_MODE;

protected:
    IMS_SINT32 m_nChannel;
    IMS_SINT32 m_nResolutionWidth;
    IMS_SINT32 m_nResolutionHeight;
    IMS_SINT32 m_nFramerate;
    IMS_SINT32 m_nBitrate;
    IMS_SINT32 m_nPacketizationMode;
    AString m_strSpropParameterSets;
    AString m_strImageAttr;
    AString m_strFrameSize;
};

#endif
