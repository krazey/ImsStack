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

#ifndef CODEC_T140_CONFIG_H_
#define CODEC_T140_CONFIG_H_

#include "config/CodecConfig.h"

class CodecT140Config : public CodecConfig
{
public:
    /**
     * @brief Construct a new codec T140 config
     *
     * @param nType codec type
     * @param nPayloadTypeNum payload type number
     */
    CodecT140Config(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum);
    /**
     * @brief Destroy the codec T140 config
     *
     */
    ~CodecT140Config() override;
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
     *
     */
    virtual void ToDebugString() const override;
    /**
     * @brief Get the red level
     *
     * @return IMS_SINT32 Return the red-level
     */
    IMS_SINT32 GetRedLevel() const;
    /**
     * @brief Get the sampling rate
     *
     * @return IMS_SINT32 Return the sampling rate for T140
     */
    IMS_SINT32 GetSamplingRate() const;

    static const IMS_SINT32 DEFAULT_RED_LEVEL = 3;
    static const IMS_SINT32 DEFAULT_RED_LEVEL_NONE = 1;
    static const IMS_SINT32 DEFAULT_TEXT_SAMPLING_RATE = 1000;

private:
    IMS_SINT32 m_nRedLevel;
    IMS_SINT32 m_nTextSamplingRate;
};

#endif
