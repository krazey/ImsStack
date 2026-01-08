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

#ifndef MOCK_REFERENCE_INTERFACE_HOLDER_H_
#define MOCK_REFERENCE_INTERFACE_HOLDER_H_

#include "helper/sipinterfaceholder/ReferenceInterfaceHolder.h"
#include <gmock/gmock.h>

class ISession;
class IReference;
class IInterfaceHolderListener;

class MockReferenceInterfaceHolder : public ReferenceInterfaceHolder
{
public:
    explicit MockReferenceInterfaceHolder(IN IInterfaceHolderListener& objListener) :
            ReferenceInterfaceHolder(objListener)
    {
    }
    ~MockReferenceInterfaceHolder() override {}
    MOCK_METHOD(void, ReferenceDelivered, (IN IReference*), (override));
    MOCK_METHOD(void, ReferenceDeliveryFailed, (IN IReference*), (override));
    MOCK_METHOD(void, ReferenceNotify, (IN IReference*, IN IMessage*), (override));
    MOCK_METHOD(void, ReferenceTerminated, (IN IReference* piReference), (override));
    MOCK_METHOD(void, Timer_TimerExpired, (IN ITimer* piTimer), (override));
    MOCK_METHOD(IReference*, GetIReference,
            (IN ISession* piSession, IN const AString& strReferTo, IN const AString& strMethod),
            (override));
    MOCK_METHOD(void, ReleaseIReference, (IN IReference* piReference, IN IMS_BOOL bTerminated),
            (override));
    MOCK_METHOD(IMS_BOOL, IsReadyToDestroy, (IN IReference* piReference), ());
    MOCK_METHOD(void, ClearIReferences, (), ());
    MOCK_METHOD(IMS_RESULT, StartTimer, (IN IReference* piReference, IN IMS_SINT32 nDuration), ());
    MOCK_METHOD(void, StopTimer, (IN ITimer* piTimer), ());
    MOCK_METHOD(ITimer*, GetTimer, (IN IReference* piReference), ());
};

#endif
