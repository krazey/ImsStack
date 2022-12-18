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

#ifndef MOCK_USSI_DATA_H_
#define MOCK_USSI_DATA_H_

#include "ImsTypeDef.h"
#include "ussi/UssiData.h"
#include "ussi/UssiDef.h"
#include <gmock/gmock.h>

class MockAnyExtension : public UssiData::AnyExtension
{
public:
    MockAnyExtension() :
            AnyExtension()
    {
    }
    ~MockAnyExtension() {}

    MOCK_METHOD(UssiModeType, GetUssiModeType, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetAlertingPattern, (), (const, override));
};

class MockUssiData : public UssiData
{
public:
    MockUssiData() :
            UssiData()
    {
    }
    ~MockUssiData() {}

    MOCK_METHOD(AString&, GetLanguage, (), (const, override));
    MOCK_METHOD(AString&, GetUssdString, (), (const, override));
    MOCK_METHOD(UssiError, GetErrorCode, (), (const, override));
    MOCK_METHOD(const AnyExtension&, GetAnyExtension, (), (const, override));
    MOCK_METHOD(IMS_BOOL, Parse, (IN const AString&), (override));
};

class MockUssiDataParser : public UssiDataParser
{
public:
    MockUssiDataParser() :
            UssiDataParser()
    {
    }
    ~MockUssiDataParser() {}

    MOCK_METHOD(UssiData*, Parse, (IN const AString&), (override));
};

#endif
