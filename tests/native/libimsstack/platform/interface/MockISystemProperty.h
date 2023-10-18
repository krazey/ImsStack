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
#ifndef MOCK_I_SYSTEM_PROPERTY_H_
#define MOCK_I_SYSTEM_PROPERTY_H_

#include <gmock/gmock.h>

#include "ISystemProperty.h"

class MockISystemProperty : public ISystemProperty
{
public:
    inline MockISystemProperty() {}
    inline virtual ~MockISystemProperty() {}

    MOCK_METHOD(AString, Get, (IN const AString& strName), (override));
    MOCK_METHOD(IMS_BOOL, Set, (IN const AString& strName, IN const AString& strValue), (override));
    MOCK_METHOD(const AString&, GetChipsetVendor, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsDebugMode, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsServerInfoHiddenInLog, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsUserMode, (), (const, override));
};

#endif
