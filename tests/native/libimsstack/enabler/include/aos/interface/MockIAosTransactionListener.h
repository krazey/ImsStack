/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef MOCK_I_AOS_TRANSACTION_LISTENER_H_
#define MOCK_I_AOS_TRANSACTION_LISTENER_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "interface/IAosTransaction.h"

class MockIAosTransactionListener : public IAosTransactionListener
{
public:
    MOCK_METHOD(void, Transaction_OnConnectionFailed,
            (IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis),
            (override));
    MOCK_METHOD(void, Transaction_OnConnectionSetupPrepared, (), (override));
    MOCK_METHOD(void, Transaction_OnTrafficPriorityChanged, (), (override));
};

#endif  // MOCK_I_AOS_TRANSACTION_LISTENER_H_
