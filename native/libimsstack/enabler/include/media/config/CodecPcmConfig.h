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

#ifndef CODEC_PCM_CONFIG_H_
#define CODEC_PCM_CONFIG_H_

#include "config/CodecConfig.h"

class CodecPcmConfig : public CodecConfig
{
public:
    /**
     * @brief Construct a new codec pcm config
     *
     * @param nType codec type
     * @param nPayloadTypeNum payload type number
     */
    CodecPcmConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum);
    /**
     * @brief Destroy the codec pcm config
     *
     */
    virtual ~CodecPcmConfig() override;
    /**
     * @brief Create codec using the configuration
     *
     * @param piCc configuration object
     * @return IMS_BOOL Return true if the create function is executed without error
     * Return false if the create function is failed
     */
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc) override;
    /**
     * @brief Print debug string
     *
     */
    virtual void ToDebugString() const override;
};

#endif
