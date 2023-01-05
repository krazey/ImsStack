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
    inline MockIIsim() {}
    inline virtual ~MockIIsim() {}

    MOCK_METHOD(void, ClearRecords, (), (override));
    MOCK_METHOD(IDigestAka*, CreateDigestAka, (), (override));
    MOCK_METHOD(IMS_RESULT, GetField, (IN IMS_SINT32 nField), (override));
    MOCK_METHOD(IMS_RESULT, GetHomeDomainName, (), (override));
    MOCK_METHOD(IMS_RESULT, GetImpi, (), (override));
    MOCK_METHOD(IMS_RESULT, GetImpu, (), (override));
    MOCK_METHOD(IMS_SINT32, GetState, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsReady, (), (override));
    MOCK_METHOD(void, AddListener, (IN IIsimListener * piListener), (override));
    MOCK_METHOD(void, RemoveListener, (IN IIsimListener * piListener), (override));
    MOCK_METHOD(IMS_RESULT, Start, (IN IMS_SINT32 nEFs), (override));
    MOCK_METHOD(IMS_RESULT, Init, (), (override));
    MOCK_METHOD(void, Release, (), (override));
};

#endif
