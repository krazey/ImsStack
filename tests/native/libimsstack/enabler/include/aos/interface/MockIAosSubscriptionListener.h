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

#ifndef MOCK_I_AOS_SUBSCRIPTION_LISTENER_H_
#define MOCK_I_AOS_SUBSCRIPTION_LISTENER_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"

#include "interface/IAosSubscriptionListener.h"

class MockIAosSubscriptionListener : public IAosSubscriptionListener
{
public:
    MOCK_METHOD(void, Subscription_StateChanged, (IN IMS_SINT32 nState, IN IMS_SINT32 nReason),
            (override));
    MOCK_METHOD(IMS_BOOL, Subscription_CanBeTransmitted, (), (override));
    MOCK_METHOD(void, Subscription_NotifyReceived, (IN IMS_SINT32 nEvent), (override));
    MOCK_METHOD(void, Subscription_Request,
            (IN IMS_SINT32 nCommand, IN IMS_SINT32 nRetryAfter, IN IMS_BOOL bAwt), (override));
};

#endif  // MOCK_I_AOS_SUBSCRIPTION_LISTENER_H_
