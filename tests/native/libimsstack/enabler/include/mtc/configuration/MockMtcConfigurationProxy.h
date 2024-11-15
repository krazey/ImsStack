/*
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

#ifndef MOCK_MTC_CONFIGURATION_PROXY_H_
#define MOCK_MTC_CONFIGURATION_PROXY_H_

#include "AString.h"
#include "ImsTypeDef.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include <gmock/gmock.h>

class MockMtcConfigurationProxy : public MtcConfigurationProxy
{
public:
    explicit MockMtcConfigurationProxy() :
            MtcConfigurationProxy()
    {
    }
    ~MockMtcConfigurationProxy() {}

    MOCK_METHOD(IMS_BOOL, GetBoolean, (IN const IMS_CHAR*), (const, override));
    MOCK_METHOD(IMS_SINT32, GetInt, (IN const IMS_CHAR*), (const, override));
    MOCK_METHOD(AString, GetString, (IN const IMS_CHAR*), (const, override));
    MOCK_METHOD(ImsVector<IMS_SINT32>, GetIntArray, (IN const IMS_CHAR*), (const, override));
    MOCK_METHOD(ImsVector<AString>, GetStringArray, (IN const IMS_CHAR*), (const, override));
    MOCK_METHOD(IMS_SINT32, GetIntFromArray, (IN const IMS_CHAR*, IN const IMS_UINT32),
            (const, override));
    MOCK_METHOD(AString, GetStringFromArray, (IN const IMS_CHAR*, IN const IMS_UINT32),
            (const, override));
    MOCK_METHOD(IMS_BOOL, Contains, (IN const IMS_CHAR*, IN IMS_SINT32), (const, override));
    MOCK_METHOD(IMS_BOOL, Contains, (IN const IMS_CHAR*, IN const IMS_CHAR*), (const, override));
    MOCK_METHOD(void, PutCache, (IN const IMS_CHAR*, IN IMS_BOOL), (override));
    MOCK_METHOD(void, PutCache, (IN const IMS_CHAR*, IN IMS_SINT32), (override));
    MOCK_METHOD(void, PutCache, (IN const IMS_CHAR*, IN const IMS_CHAR*), (override));
    MOCK_METHOD(void, OnRegistrationRefreshed, (), (override));
};

#endif
