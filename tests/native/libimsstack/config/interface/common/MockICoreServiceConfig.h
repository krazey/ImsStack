/*
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
#ifndef MOCK_I_CORE_SERVICE_CONFIG_H_
#define MOCK_I_CORE_SERVICE_CONFIG_H_

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "common/ICoreServiceConfig.h"
#include <gmock/gmock.h>

class AString;
class ServiceIdentifier;

class MockICoreServiceConfig : public ICoreServiceConfig
{
public:
    ~MockICoreServiceConfig() override = default;

    MOCK_METHOD(const AString&, GetServiceId, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsIariSupported, (), (const, override));
    MOCK_METHOD(const ServiceIdentifier&, GetIari, (), (const, override));
    MOCK_METHOD(const ImsList<ServiceIdentifier>&, GetIcsis, (), (const, override));
    MOCK_METHOD(const ImsList<ServiceIdentifier>&, GetFeatureTags, (), (const, override));
    MOCK_METHOD(const AString&, GetMediaProfile, (), (const, override));
};

#endif
