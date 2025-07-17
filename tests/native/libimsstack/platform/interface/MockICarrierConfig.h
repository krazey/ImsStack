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

#ifndef MOCK_I_CARRIER_CONFIG_H_
#define MOCK_I_CARRIER_CONFIG_H_

#include <gmock/gmock.h>

#include "ICarrierConfig.h"

class MockICarrierConfig : public ICarrierConfig
{
public:
    MockICarrierConfig() = default;
    ~MockICarrierConfig() override = default;

    MOCK_METHOD(IMS_BOOL, GetBoolean, (IN const IMS_CHAR*, IN IMS_BOOL), (const, override));
    MOCK_METHOD(IMS_SINT32, GetInt, (IN const IMS_CHAR*, IN IMS_SINT32), (const, override));
    MOCK_METHOD(IMS_SLONG, GetLong, (IN const IMS_CHAR*, IN IMS_SLONG), (const, override));
    MOCK_METHOD(AString, GetString, (IN const IMS_CHAR*, IN const AString&), (const, override));
    MOCK_METHOD(ImsVector<IMS_BOOL>, GetBooleanArray,
            (IN const IMS_CHAR*, OUT IMS_BOOL& bKeyExists), (const, override));
    MOCK_METHOD(ImsVector<IMS_SINT32>, GetIntArray, (IN const IMS_CHAR*, OUT IMS_BOOL& bKeyExists),
            (const, override));
    MOCK_METHOD(ImsVector<IMS_SLONG>, GetLongArray, (IN const IMS_CHAR*, OUT IMS_BOOL& bKeyExists),
            (const, override));
    MOCK_METHOD(ImsVector<AString>, GetStringArray, (IN const IMS_CHAR*, OUT IMS_BOOL& bKeyExists),
            (const, override));
    MOCK_METHOD(ICarrierConfig*, GetBundle, (IN const IMS_CHAR*), (const, override));
    MOCK_METHOD(void, ReleaseBundle, (), (override));
    MOCK_METHOD(void, AddListener, (IN ICarrierConfigListener*), (override));
    MOCK_METHOD(void, RemoveListener, (IN ICarrierConfigListener*), (override));
};

#endif
