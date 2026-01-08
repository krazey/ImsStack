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

#ifndef MOCK_I_AOS_LOCATION_STARTER_H_
#define MOCK_I_AOS_LOCATION_STARTER_H_

#include <gmock/gmock.h>

#include "interface/IAosBlock.h"
#include "interface/IAosLocationStarter.h"

class MockIAosLocationStarter : public IAosLocationStarter
{
public:
    MOCK_METHOD(IMS_SINT32, GetSlotId, (), (const, override));
    MOCK_METHOD(void, SetSlotId, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(void, Init, (IN IAosAppContext * piContext, IN IMS_UINT32 nPolicy), (override));
    MOCK_METHOD(IMS_BOOL, SetPolicy, (IN IMS_UINT32 nPolicy, IN IMS_SINT32 nOperation), (override));
    MOCK_METHOD(IMS_BOOL, IsPolicyEnabled, (IN IMS_UINT32 nPolicy), (override));
    MOCK_METHOD(void, AddBlockReason, (IN BLOCK_REASON nReason, IN IMS_SINT32 nType), (override));
    MOCK_METHOD(IMS_BOOL, SetUpdateInterval, (IN IMS_UINT32 nInterval), (override));
    MOCK_METHOD(void, StartLocationInfoUpdate, (), (override));
    MOCK_METHOD(void, StopLocationInfoUpdate, (), (override));
};

#endif  // MOCK_I_AOS_LOCATION_STARTER_H_
