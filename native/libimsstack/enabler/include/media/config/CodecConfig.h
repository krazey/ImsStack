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

#ifndef CODEC_CONFIG_H_
#define CODEC_CONFIG_H_

#include "CarrierConfig.h"
#include "ICarrierConfig.h"
#include "config/ImsCodec.h"

class CodecConfig
{
public:
    /**
     * @brief Construct a new codec config object
     *
     * @param nCodec_ codec type
     * @param nPayloadTypeNum_ payload type number
     */
    CodecConfig(IN IMS_SINT32 nCodec_, IN IMS_SINT32 nPayloadTypeNum_);
    /**
     * @brief Destroy the codec config object
     *
     */
    virtual ~CodecConfig();
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
     * @brief Get the current codec type
     *
     * @return IMS_SINT32 Return codec type
     */
    virtual IMS_SINT32 GetCodec() const;
    /**
     * @brief Get the payload type
     *
     * @return IMS_SINT32 Return the payload type number
     */
    virtual IMS_SINT32 GetPayloadType() const;

protected:
    IMS_SINT32 m_nCodec;
    IMS_SINT32 m_nPayloadType;
};

#endif
