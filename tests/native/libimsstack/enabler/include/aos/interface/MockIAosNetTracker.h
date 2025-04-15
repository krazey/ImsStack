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

#ifndef MOCK_I_AOS_NET_TRACKER_H_
#define MOCK_I_AOS_NET_TRACKER_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "interface/IAosNetTracker.h"

class MockIAosNetTracker : public IAosNetTracker
{
public:
    MOCK_METHOD(IMS_BOOL, IsServiceIn, (IN IMS_UINT32 nType), (override));
    MOCK_METHOD(IMS_BOOL, IsDataIn, (), (override));
    MOCK_METHOD(IMS_BOOL, IsNetworkIn, (), (override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyAttach, (), (override));
    MOCK_METHOD(IMS_BOOL, IsSuspended, (), (override));
    MOCK_METHOD(IMS_BOOL, IsSessionContinuitySupported, (), (override));
    MOCK_METHOD(IMS_BOOL, IsServiceTimerRunning, (), (override));
    MOCK_METHOD(IMS_BOOL, IsImsVoiceCallSupported, (), (override));
    MOCK_METHOD(IMS_BOOL, IsRoaming, (), (override));
    MOCK_METHOD(IMS_UINT32, GetMobileChangingNetworkType, (), (override));
    MOCK_METHOD(IMS_UINT32, GetMobileNetworkType, (), (override));
    MOCK_METHOD(IMS_UINT32, GetMobileNetworkRegistrationRejectCause, (), (override));
    MOCK_METHOD(IMS_SINT32, GetMobileVoiceServiceState, (), (override));
    MOCK_METHOD(IMS_UINT32, GetMobileVoiceNetworkType, (), (override));
    MOCK_METHOD(IMS_UINT32, GetNetworkType, (), (override));
    MOCK_METHOD(void, SetRatGuardTime, (IN IMS_UINT32 nGuardTime), (override));
    MOCK_METHOD(void, SetSrvOutGuardTime, (IN IMS_UINT32 nGuardTime), (override));
    MOCK_METHOD(void, SetSrvInGuardTime, (IN IMS_UINT32 nGuardTime), (override));
    MOCK_METHOD(void, SetListener, (IN IAosNetTrackerListener * piListener), (override));
    MOCK_METHOD(void, RemoveListener, (IN IAosNetTrackerListener * piListener), (override));
    MOCK_METHOD(void, Init, (), (override));
};

#endif  // MOCK_I_AOS_NET_TRACKER_H_
