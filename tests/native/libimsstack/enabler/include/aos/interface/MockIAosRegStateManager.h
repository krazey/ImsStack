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

#ifndef MOCK_I_AOS_REG_STATE_MANAGER_H_
#define MOCK_I_AOS_REG_STATE_MANAGER_H_

#include <gmock/gmock.h>
#include "interface/IAosRegStateManager.h"

class MockIAosRegStateManager : public IAosRegStateManager
{
public:
    MOCK_METHOD(IMS_SINT32, GetSlotId, (), (const, override));
    MOCK_METHOD(void, SetSlotId, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(void, SetImsRegState, (IN IMS_UINT32 nState, IN IMS_BOOL bLimited), (override));
    MOCK_METHOD(IMS_SINT32, GetImsRegState, (), (override));
    MOCK_METHOD(void, SetEImsRegState, (IN IMS_UINT32 nState), (override));
    MOCK_METHOD(void, SetRegState, (IN IMS_UINT32 nServiceType, IN IMS_UINT32 nState), (override));
};

#endif  // MOCK_I_AOS_REG_STATE_MANAGER_H_
