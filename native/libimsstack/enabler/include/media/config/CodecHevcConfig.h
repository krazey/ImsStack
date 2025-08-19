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

#include "config/CodecVideoConfig.h"

class CodecHevcConfig : public CodecVideoConfig
{
public:
    /**
     * @brief Construct a new codec hevc config
     *
     * @param nType video codec type - hevc (H.265)
     * @param nPayloadTypeNum payload type number
     */
    CodecHevcConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum);
    /**
     * @brief Destroy the codec hevc config
     *
     */
    ~CodecHevcConfig() override;
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
     * @brief Generate default Hevc codec when Hevc bundle description is missing
     */
    virtual void CreateDefaultHevcCodec();

public:
    static const IMS_SINT32 DEFAULT_HEVC_RESOLUTION_WIDTH = 480;
    static const IMS_SINT32 DEFAULT_HEVC_RESOLUTION_HEIGHT = 640;
    static const IMS_SINT32 DEFAULT_HEVC_FRAMERATE = 30;
    static const IMS_SINT32 DEFAULT_HEVC_BITRATE = 512;
    static const IMS_SINT32 DEFAULT_HEVC_PROFILE = 1;
    static const IMS_SINT32 DEFAULT_HEVC_LEVEL = 93;

protected:
    IMS_SINT32 m_nHevcProfile;
    IMS_SINT32 m_nHevcLevel;
};

#endif
