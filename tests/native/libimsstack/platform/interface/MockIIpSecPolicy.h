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

#ifndef MOCK_I_IP_SEC_POLICY_H_
#define MOCK_I_IP_SEC_POLICY_H_

#include <gmock/gmock.h>

#include "ImsList.h"
#include "IIpSecPolicy.h"

class MockIIpSecPolicy : public IIpSecPolicy
{
public:
    MockIIpSecPolicy() = default;
    ~MockIIpSecPolicy() override = default;

    MOCK_METHOD(IMS_SINT32, GetId, (), (const, override));
    MOCK_METHOD(IIpSecSp*, CreateSp, (), (override));
    MOCK_METHOD(void, DestroySp, (IN IIpSecSp * piSp), (override));
    MOCK_METHOD(IIpSecSa*, CreateSa, (), (override));
    MOCK_METHOD(void, DestroySa, (IN IIpSecSa * piSa), (override));
    MOCK_METHOD(void, ManageLifetime, (IN IMS_UINT32 nDuration), (override));
    MOCK_METHOD(void, SetListener, (IN IIpSecPolicyListener * piListener), (override));
};

#endif
