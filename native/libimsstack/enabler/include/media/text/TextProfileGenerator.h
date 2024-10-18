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

#ifndef TEXT_PROFILE_GENERATOR_H_
#define TEXT_PROFILE_GENERATOR_H_

#include "MediaProfileGenerator.h"
#include "text/TextProfileUtil.h"

class CodecTextConfig;
class TextConfiguration;

/**
 * This class is to generate a text profile by parsing a text configuration
 */
class TextProfileGenerator : public MediaProfileGenerator
{
public:
    TextProfileGenerator();
    virtual ~TextProfileGenerator();

protected:
    TextProfile* SetProfile(IN MediaBaseProfile* pProfile, IN MediaConfiguration* pConfig,
            IN MediaEnvironment* pEnvironment, IN IMS_SINT32 nSlotId) override;
    void CreateCodecPayloads(IN MediaBaseProfile* pProfile, IN IMS_SINT32 nCodec,
            IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig) override;
    TextProfile::Payload* CreateT140Payload(
            IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig);
};
#endif
