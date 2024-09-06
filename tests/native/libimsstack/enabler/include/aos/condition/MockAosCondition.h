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
#ifndef MOCK_AOS_CONDITION_H_
#define MOCK_AOS_CONDITION_H_

#include <gmock/gmock.h>

#include "condition/AosCondition.h"

class MockAosCondition : public AosCondition
{
public:
    MockAosCondition() :
            AosCondition()
    {
    }
    ~MockAosCondition() override {}

    MOCK_METHOD(void, Start, (), (override));
    MOCK_METHOD(void, Stop, (), (override));
    MOCK_METHOD(void, SetListener, (IN IAosConditionListener * piListener), (override));
    MOCK_METHOD(void, SetBlock, (IN BLOCK_REASON eReason, IN IMS_BOOL bNotify), (override));
    MOCK_METHOD(void, ResetBlock, (IN BLOCK_REASON eReason, IN IMS_BOOL bNotify), (override));
    MOCK_METHOD(IMS_BOOL, IsReasonBlocked, (IN BLOCK_REASON eReason), (const, override));
    MOCK_METHOD(IMS_BOOL, IsReady, (), (override));
    MOCK_METHOD(IMS_UINT32, CheckServiceAvailable, (IN SERVICE_TYPE eType), (override));
    MOCK_METHOD(IMS_BOOL, CheckBadNetwork, (IN SERVICE_TYPE eType), (override));
    MOCK_METHOD(void, PrintBlockReasons, (), (const, override));
    MOCK_METHOD(void, AddServiceAvailable, (), (override));
    MOCK_METHOD(void, RemoveServiceAvailable, (), (override));
    MOCK_METHOD(void, AddAosServiceListener, (), (override));
    MOCK_METHOD(void, RemoveAosServiceListener, (), (override));
    MOCK_METHOD(void, AddEventListener, (), (override));
    MOCK_METHOD(void, RemoveEventListener, (), (override));
};

#endif  // MOCK_AOS_CONDITION_H_
