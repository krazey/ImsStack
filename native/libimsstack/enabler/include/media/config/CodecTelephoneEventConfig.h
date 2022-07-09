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

#ifndef _CODEC_TELEPHONE_EVENT_CONFIG_H_
#define _CODEC_TELEPHONE_EVENT_CONFIG_H_

#include "AString.h"
#include "config/CodecConfig.h"

class CodecTelephoneEventConfig : public CodecConfig
{
public:
    /**
     * @brief Construct a new codec telephone event config
     *
     * @param nType_ codec type
     * @param nPayloadTypeNum_ payload type number
     */
    CodecTelephoneEventConfig(IN IMS_SINT32 nType_, IN IMS_SINT32 nPayloadTypeNum_);
    /**
     * @brief Destroy the codec telephone cvent config
     *
     */
    virtual ~CodecTelephoneEventConfig();
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
     * @brief Print debug string
     *
     */
    virtual void ToDebugString() const;
    /**
     * @brief Get the events (such as ringing or busy tone)
     *
     * @return const AString& Return the events string - default: "0-15"
     */
    const AString& GetEvents() const;
    /**
     * @brief Get the redundancy count
     *
     * @return IMS_SINT32 Return the redundancy count
     */
    IMS_SINT32 GetRedundancyCount() const;
    /**
     * @brief Get the sampling rate
     *
     * @return IMS_SINT32 Return the sample rate for telephoneEvent
     */
    IMS_SINT32 GetSamplingRate() const;

    static const IMS_SINT32 DEFAULT_REDUNDANT_COUNT = 3;
    static const IMS_SINT32 DEFAULT_SAMPLING_RATE = 8000;
    static const IMS_SINT32 DEFAULT_SAMPLING_RATE_WB = 16000;

private:
    AString m_strEvents;
    IMS_SINT32 m_nRedundancyCount;
    IMS_SINT32 m_nSamplingRate;
};
#endif  // _CODEC_TELEPHONE_EVENT_CONFIG_H_
