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

#ifndef CODEC_AMR_CONFIG_H_
#define CODEC_AMR_CONFIG_H_

#include "config/CodecAudioConfig.h"

class CodecAmrConfig : public CodecAudioConfig
{
public:
    /**
     * @brief Construct a new codec amr config
     *
     * @param nType audio codec type (AUDIO_AMR, AUDIO_AMR_WB)
     * @param nPayloadTypeNum payload type number
     */
    CodecAmrConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum);
    /**
     * @brief Destroy the codec amr config object
     *
     */
    virtual ~CodecAmrConfig();
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
     * @brief Get the payload format
     *
     * @return IMS_SINT32 Return bandwidth-efficient or octet-align
     */
    IMS_SINT32 GetOctetAlign() const;
    /**
     * @brief Get the sampling rate
     *
     * @return IMS_SINT32 Return the audio codec sampling rate
     */
    IMS_SINT32 GetSamplingRate() const;

public:
    enum
    {
        /** Full payload is octet aligned */
        BANDWIDTH_EFFICIENT = 0,
        /** All the fields are individually aligned to octet boundaries */
        OCTET_ALIGN = 1
    };

    static const IMS_SINT32 DEFAULT_OCTET_ALIGN = BANDWIDTH_EFFICIENT;
    static const IMS_SINT32 DEFAULT_SAMPLING_RATE_AMR = 8000;
    static const IMS_SINT32 DEFAULT_SAMPLING_RATE_AMRWB = 16000;

private:
    IMS_SINT32 m_nOctetAlign;
    IMS_SINT32 m_nSamplingRate;
};

#endif
