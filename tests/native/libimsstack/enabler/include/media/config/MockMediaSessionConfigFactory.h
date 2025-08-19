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

#ifndef MOCK_MEDIA_SESSION_CONFIG_FACTORY_H_
#define MOCK_MEDIA_SESSION_CONFIG_FACTORY_H_

#include <gmock/gmock.h>

#include "config/MediaSessionConfigFactory.h"

class MockMediaSessionConfigFactory : public MediaSessionConfigFactory
{
public:
    MockMediaSessionConfigFactory() = default;
    ~MockMediaSessionConfigFactory() override = default;

    // mocking the public methods
    MOCK_METHOD(void, CreateMediaSessionConfig,
            (IMS_SINT32 nSlotId, MEDIA_SERVICE_TYPE eServiceType), (override));
    MOCK_METHOD(void, AddMediaSessionConfig,
            (IMS_SINT32 nSlotId, MediaSessionConfig* mediaSessionConfig), (override));
    MOCK_METHOD(void, DestroyListSessionConfig, (IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(
            ImsList<MediaSessionConfig*>*, GetListSessionConfig, (IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(MediaSessionConfig*, FindMediaSessionConfig,
            (IMS_SINT32 nSlotId, MEDIA_SERVICE_TYPE eServiceType), (override));
    MOCK_METHOD(void, DestroySessionConfig, (const MediaSessionConfig* pMediaSessionConfig),
            (override));
};

#endif  // MOCK_MEDIA_SESSION_CONFIG_FACTORY_H_
