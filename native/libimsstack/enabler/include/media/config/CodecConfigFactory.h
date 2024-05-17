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

#ifndef CODEC_CONFIG_FACTORY_H_
#define CODEC_CONFIG_FACTORY_H_

#include "config/CodecAmrConfig.h"
#include "config/CodecPcmConfig.h"
#include "config/CodecTelephoneEventConfig.h"
#include "config/CodecEvsConfig.h"
#include "config/CodecAvcConfig.h"
#include "config/CodecHevcConfig.h"
#include "config/CodecT140Config.h"

class CodecConfig;

class CodecConfigFactory
{
private:
    CodecConfigFactory();

public:
    /**
     * @brief Create an audio payload config
     *
     * @param piCc configuration
     * @param nCodec codec type
     * @param nPayloadTypeNum payload type number
     * @return CodecConfig* Return the codec config
     */
    static CodecConfig* CreateAudioPayloadConfig(
            ICarrierConfig* piCc, IMS_SINT32 nCodec, IMS_SINT32 nPayloadTypeNum);
    /**
     * @brief Create a video payload config
     *
     * @param piCc configuration
     * @param nCodec codec type
     * @param nPayloadTypeNum payload type number
     * @return CodecConfig* Return the codec config
     */
    static CodecConfig* CreateVideoPayloadConfig(
            ICarrierConfig* piCc, IMS_SINT32 nCodec, IMS_SINT32 nPayloadTypeNum);
    /**
     * @brief Create a text payload config
     *
     * @param piCc configuration
     * @param nCodec codec type
     * @param nPayloadTypeNum payload type number
     * @return CodecConfig* Return the codec config
     */
    static CodecConfig* CreateTextPayloadConfig(
            ICarrierConfig* piCc, IMS_SINT32 nCodec, IMS_SINT32 nPayloadTypeNum);
};

#endif
