/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef MOCK_AUDIO_PROFILE_GENERATOR_H_
#define MOCK_AUDIO_PROFILE_GENERATOR_H_

#include <gmock/gmock.h>
#include <media/MediaProfileGenerator.h>

class MockMediaProfileGenerator : public MediaProfileGenerator
{
public:
    MockMediaProfileGenerator() {}
    ~MockMediaProfileGenerator() {}

    MOCK_METHOD(MediaBaseProfile*, Generate,
            (MEDIA_SERVICE_TYPE eServiceType, IN IService* pIService,
                    IN MediaConfiguration* pConfig, IN IMS_SINT32 nSlotId),
            (override));

    MOCK_METHOD(MediaBaseProfile*, SetProfile,
            (IN MediaBaseProfile * pProfile, IN MediaConfiguration* pConfig,
                    MEDIA_SERVICE_TYPE eServiceType, IN IService* pIService, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(void, SetCommonProfile,
            (IN MediaBaseProfile * pProfile, IN MediaConfiguration* pConfig, IN IService* pIService,
                    IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(MediaBaseProfile*, SetPayloads,
            (IN MediaBaseProfile * pProfile, IN MediaConfiguration* pConfig), (override));
    MOCK_METHOD(void, CreateCodecPayloads,
            (IN MediaBaseProfile * pProfile, IN IMS_SINT32 nCodec, IN CodecConfig* pCodecConfig,
                    IN MediaConfiguration* pConfig),
            (override));
};

#endif
