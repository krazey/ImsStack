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
#ifndef MOCK_I_ISIM_H_
#define MOCK_I_ISIM_H_

#include <gmock/gmock.h>

#include "IIsim.h"

class MockIIsim : public IIsim
{
public:
    MockIIsim() = default;
    ~MockIIsim() override = default;

    MOCK_METHOD(IDigestAka*, CreateDigestAka, (), (override));
    MOCK_METHOD(AString, GetHomeDomainName, (), (const, override));
    MOCK_METHOD(AString, GetImpi, (), (const override));
    MOCK_METHOD(AStringArray, GetImpu, (), (const override));
    MOCK_METHOD(IMS_SINT32, GetState, (), (const, override));
    MOCK_METHOD(AStringArray, GetPcscf, (), (const override));
    MOCK_METHOD(IMS_BOOL, IsLoadCompleted, (), (const override));
    MOCK_METHOD(void, AddListener, (IN IIsimListener * piListener), (override));
    MOCK_METHOD(void, RemoveListener, (IN IIsimListener * piListener), (override));
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, Release, (), (override));
};

#endif
