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
#ifndef MOCK_I_REG_INFO_MANAGER_H_
#define MOCK_I_REG_INFO_MANAGER_H_

#include <gmock/gmock.h>

#include "IRegInfoManager.h"

class MockIRegInfoManager : public IRegInfoManager
{
public:
    MockIRegInfoManager() = default;
    ~MockIRegInfoManager() override = default;

    MOCK_METHOD(IMS_BOOL, CreateRegInfo, (IN const RegKey& objRegKey), (override));
    MOCK_METHOD(void, DestroyRegInfo, (IN const RegKey& objRegKey), (override));
    MOCK_METHOD(RegInfo*, GetRegInfo, (IN const RegKey& objRegKey), (override));
    MOCK_METHOD(const RegInfo*, GetRegInfo, (IN const RegKey& objRegKey), (const, override));
    MOCK_METHOD(IMS_BOOL, Update, (IN const RegKey& objRegKey, IN const AString& strRegInfo),
            (override));
    MOCK_METHOD(void, DisplayRegInfo, (), (const, override));
};

#endif
