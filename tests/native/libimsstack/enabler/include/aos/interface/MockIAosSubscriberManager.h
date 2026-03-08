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

#ifndef MOCK_I_AOS_SUBSCRIBER_MANAGER_H_
#define MOCK_I_AOS_SUBSCRIBER_MANAGER_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "interface/IAosSubscriber.h"
#include "interface/IAosSubscriberManagerListener.h"
#include "interface/IAosSubscriberManager.h"

class MockIAosSubscriberManager : public IAosSubscriberManager {
public:
    MOCK_METHOD(IMS_BOOL, IsReady, (IN IMS_BOOL bIsFake), (const, override));
    MOCK_METHOD(IMS_BOOL, IsIsim, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsUsim, (), (const, override));
    MOCK_METHOD(void, AddListener, (IN IAosSubscriberManagerListener* piListener), (override));
    MOCK_METHOD(void, RemoveListener, (IN IAosSubscriberManagerListener* piListener), (override));
    MOCK_METHOD(void, AddListenerForMonitor, (IN IAosSubscriberManagerListener* piListener),
            (override));
    MOCK_METHOD(void, RemoveListenerForMonitor, (IN IAosSubscriberManagerListener* piListener),
            (override));
    MOCK_METHOD(const AStringArray&, GetConfiguredImpus, (), (const, override));
    MOCK_METHOD(const AStringArray&, GetOrderedImpus, (), (const, override));
    MOCK_METHOD(const AStringArray&, GetConfiguredImpusForFake, (), (const, override));
    MOCK_METHOD(const AStringArray&, GetFakeImpus, (), (const, override));
    MOCK_METHOD(const ISubscriberConfig*, GetSubscriberConfig, (IN IMS_SINT32 nType),
            (const, override));
    MOCK_METHOD(SimState, GetSimState, (), (const, override));
};

#endif // MOCK_I_AOS_SUBSCRIBER_MANAGER_H_
