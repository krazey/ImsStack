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
#ifndef MOCK_I_CONFIGURATION_H_
#define MOCK_I_CONFIGURATION_H_

#include <gmock/gmock.h>

#include "IConfiguration.h"

class MockIConfiguration : public IConfiguration
{
public:
    MockIConfiguration() = default;
    ~MockIConfiguration() override = default;

    MOCK_METHOD(AStringArray, GetLocalAppIds, (IN IMS_SINT32 nSlotId), (const, override));
    MOCK_METHOD(const IAppConfig*, GetAppConfig,
            (IN const AString& strAppId, IN IMS_SINT32 nSlotId), (const, override));
    MOCK_METHOD(IMS_BOOL, HasAppConfig, (IN const AString& strAppId, IN IMS_SINT32 nSlotId),
            (const, override));
    MOCK_METHOD(
            void, RemoveAppConfig, (IN const AString& strAppId, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_RESULT, SetAppConfig, (IN const AString& strAppId, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(IMS_RESULT, SetAppConfig,
            (IN const AString& strAppId, IN const AString& strClassName,
                    IN const ImsRegistry& objRegistry, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(const IMediaConfig*, GetMediaConfig, (IN IMS_SINT32 nSlotId), (const, override));
    MOCK_METHOD(const ISipConfig*, GetSipConfig, (IN IMS_SINT32 nSlotId), (const, override));
    MOCK_METHOD(ISubscriberConfig*, GetSubscriberConfig,
            (IN IMS_SINT32 nSlotId, IN const AString& strId /* = AString::ConstNull()*/),
            (const, override));
    MOCK_METHOD(IMS_UINT32, GetTraceModule, (IN IMS_SINT32 nSlotId), (const, override));
    MOCK_METHOD(IMS_UINT32, GetTraceOption, (IN IMS_SINT32 nSlotId), (const, override));
    MOCK_METHOD(IMS_BOOL, IsServerInfoHiddenInLog, (IN IMS_SINT32 nSlotId), (const, override));
    MOCK_METHOD(void, InitConfigs, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(void, RefreshConfigs, (IN IMS_SINT32 nSlotId), (override));
};

#endif
