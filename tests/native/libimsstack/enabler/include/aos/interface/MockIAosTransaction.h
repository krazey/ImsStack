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

#ifndef MOCK_I_AOS_TRANSACTION_H_
#define MOCK_I_AOS_TRANSACTION_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "interface/IAosTransaction.h"

class MockIAosTransaction : public IAosTransaction
{
public:
    MOCK_METHOD(void, SetListener, (IN IMS_UINT32 nType, IN IAosTransactionListener* piListener),
            (override));
    MOCK_METHOD(void, RemoveListener, (IN IMS_UINT32 nType, IN IAosTransactionListener* piListener),
            (override));
    MOCK_METHOD(IMS_BOOL, IsTransactionAllowed, (IN IMS_UINT32 nType), (override));
    MOCK_METHOD(
            IMS_BOOL, StartTraffic, (IN IMS_UINT32 nType, IN IMS_UINT32 nRadioType), (override));
    MOCK_METHOD(void, StartEmergencyTraffic, (IN IMS_UINT32 nRadioType), (override));
    MOCK_METHOD(void, StopTraffic, (IN IMS_UINT32 nType), (override));
    MOCK_METHOD(void, StopEmergencyTraffic, (), (override));
};

#endif  // MOCK_I_AOS_TRANSACTION_H_
