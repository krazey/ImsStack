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

#ifndef MOCK_SUBSCRIPTION_INTERFACE_HOLDER_H_
#define MOCK_SUBSCRIPTION_INTERFACE_HOLDER_H_

#include <gmock/gmock.h>

#include "helper/sipinterfaceholder/SubscriptionInterfaceHolder.h"

class ISession;
class ICoreService;
class ISubscription;
class IInterfaceHolderListener;

class MockSubscriptionInterfaceHolder : public SubscriptionInterfaceHolder
{
public:
    explicit MockSubscriptionInterfaceHolder(IN IInterfaceHolderListener& objListener) :
            SubscriptionInterfaceHolder(objListener)
    {
    }
    ~MockSubscriptionInterfaceHolder() {}
    MOCK_METHOD(void, SubscriptionForkedNotify, (IN ISubscription*, IN ISubscription*), (override));
    MOCK_METHOD(void, SubscriptionNotify, (IN ISubscription*, IN IMessage*), (override));
    MOCK_METHOD(void, SubscriptionStarted, (IN ISubscription*), (override));
    MOCK_METHOD(void, SubscriptionStartFailed, (IN ISubscription*), (override));
    MOCK_METHOD(void, SubscriptionTerminated, (IN ISubscription * piSubscription), (override));
    MOCK_METHOD(void, Timer_TimerExpired, (IN ITimer * piTimer), (override));
    MOCK_METHOD(ISubscription*, GetISubscription,
            (IN ISession * piSession, IN const AString& strEvent), (override));
    MOCK_METHOD(ISubscription*, GetISubscription,
            (IN ICoreService * piCoreService, IN const AString& strFrom, IN const AString& strTo,
                    IN const AString& strEvent),
            (override));
    MOCK_METHOD(void, ReleaseISubscription,
            (IN ISubscription * piSubscription, IN IMS_BOOL bTerminated), (override));
    MOCK_METHOD(IMS_BOOL, IsReadyToDestroy, (IN ISubscription * piSubscription), ());
    MOCK_METHOD(void, ClearISubscriptions, (), ());
    MOCK_METHOD(IMS_RESULT, StartTimer,
            (IN ISubscription * piSubscription, IN IMS_SINT32 nDuration), ());
    MOCK_METHOD(void, StopTimer, (IN ITimer * piTimer), ());
    MOCK_METHOD(ITimer*, GetTimer, (IN ISubscription * piSubscription), ());
};

#endif
