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

#ifndef CODEC_AVC_CONFIG_H_
#define CODEC_AVC_CONFIG_H_

#include "config/CodecVideoConfig.h"

class CodecAvcConfig : public CodecVideoConfig
{
public:
    /**
     * @brief Construct a new codec avc config object
     *
     * @param nType video codec type - avc (H.264)
     * @param nPayloadTypeNum payload type number
     */
    CodecAvcConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum);
    /**
     * @brief Destroy the codec avc config object
     *
     */
    ~CodecAvcConfig() override;
    /**
     * @brief Create codec using the configuration
     *
     * @param piCc configuration
     * @return IMS_BOOL Return true if the create function is executed without error
     * Return false if the create function is failed
     */
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc) override;
    /**
     * @brief Print debug strings
     */
    virtual void ToDebugString() const override;
    /**
     * @brief Get the include sprop parameter sets
     *
     * @return IMS_BOOL Return true if m_bIncludeSpropParameterSets is enabled
     * Return false if m_bIncludeSpropParameterSets is disabled
     */
    IMS_BOOL GetIncludeSpropParameterSets() const;
    /**
     * @brief Get the profile level id
     *
     * @return const AString& Return profile level-id
     */
    const AString& GetProfileLevelId() const;
    /**
     * @brief Generate default AVC codec when AVC bundle description is missing
     */
    virtual void CreateDefaultAvcCodec();

public:
    static const IMS_SINT32 DEFAULT_AVC_RESOLUTION_WIDTH = 240;
    static const IMS_SINT32 DEFAULT_AVC_RESOLUTION_HEIGHT = 320;
    static const IMS_SINT32 DEFAULT_AVC_FRAMERATE = 15;
    static const IMS_SINT32 DEFAULT_AVC_BITRATE = 384;
    static const IMS_BOOL DEFAULT_INCLUDE_SPROP = IMS_FALSE;

protected:
    IMS_BOOL m_bIncludeSpropParameterSets;
    AString m_strProfileLevelId;
};

#endif
