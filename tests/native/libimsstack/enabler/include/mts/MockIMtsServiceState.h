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

#ifndef MOCK_I_MTS_SERVICESTATE_H_
#define MOCK_I_MTS_SERVICESTATE_H_

#include <gmock/gmock.h>
#include "IMtsServiceState.h"

class MockIMtsServiceState : public IMtsServiceState
{
public:
    virtual ~MockIMtsServiceState() {}

    // MtsMessageController
    MOCK_METHOD(IMS_BOOL, IsMoServiceBlocked, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsMtServiceBlocked, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsTemporaryServiceBlocked, (), (const, override));

    // MtsService
    MOCK_METHOD(IMS_BOOL, IsImsTrafficAllowed, (IN IMS_UINT32 nTrafficType), (override));
    MOCK_METHOD(void, StartImsTraffic,
            (IN IMS_UINT32 nTrafficType, IN IMS_UINT32 nAccessNetworkType,
                    IN IImsRadioConnectionListener* piListener),
            (override));
    MOCK_METHOD(void, TriggerEpsFallback, (IN IMS_UINT32 nEpsfbReason), (override));
    MOCK_METHOD(void, AddListenerForTrafficPriority,
            (IN IImsRadioTrafficPriorityListener* piListener), (override));
    MOCK_METHOD(void, RemoveListenerForTrafficPriority,
            (IN IImsRadioTrafficPriorityListener* piListener), (override));
    MOCK_METHOD(void, StartRadioGuardTimer, (IN IMS_UINT32 nTrafficType), (override));
    MOCK_METHOD(IMS_BOOL, IsRadioGuardTimerActive, (IN IMS_UINT32 nTrafficType), (override));

    MOCK_METHOD(void, SetSmsOverIpState, (IN IMS_BOOL bState), (override));
    MOCK_METHOD(void, SetConnectedServices, (IN IMS_UINT32 nServices), (override));
    MOCK_METHOD(void, OnImsConnected, (), (override));
    MOCK_METHOD(void, OnImsDisconnected, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, OnImsDisconnecting, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, OnImsSuspended, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, OnImsResumed, (), (override));
    MOCK_METHOD(void, NotifySpecificMessage,
            (IN IMS_UINT32 nMsg, IN IMS_UINT32 nWparam, IN IMS_UINT32 nLparam), (override));

    MOCK_METHOD(IMS_SINT32, GetServiceState, (), (override));
    MOCK_METHOD(IMS_BOOL, IsServiceConnected, (IN IMS_UINT32 nService), (override));
    MOCK_METHOD(void, SetImsRegConnected, (IN IMS_BOOL bConnected), (override));
};

#endif
