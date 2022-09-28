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

#ifndef MOCK_I_MTC_CALL_TRAFFIC_CHECKER_H_
#define MOCK_I_MTC_CALL_TRAFFIC_CHECKER_H_

#include <gmock/gmock.h>

#include "call/traffic/IMtcCallTrafficChecker.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

class MockIMtcCallTrafficChecker : public IMtcCallTrafficChecker
{
public:
    ~MockIMtcCallTrafficChecker() = default;

    MOCK_METHOD(void, SetTrafficCheckerListener, (IN IMtcCallTrafficCheckerListener * pListener),
            (override));
    MOCK_METHOD(IMS_BOOL, IsTrafficPrepared, (CallType eCallType, IMS_BOOL bEmergency),
            (const, override));
    MOCK_METHOD(IMS_BOOL, IsTrafficAllowed, (CallType eCallType, IMS_BOOL bEmergency),
            (const, override));
    MOCK_METHOD(void, StartTrafficChecking,
            (IN CallType eCallType, IN IMS_BOOL bEmergency, IN IMS_BOOL bWifi), (override));
    MOCK_METHOD(void, StopTrafficChecking, (TrafficType eTrafficType), (override));
    MOCK_METHOD(void, HandleIpcanChanged, (IN IMS_UINT32 eIpcan, IMS_BOOL bEmergency), (override));
};

class MockIMtcCallTrafficCheckerListener : public IMtcCallTrafficCheckerListener
{
public:
    ~MockIMtcCallTrafficCheckerListener() = default;

    MOCK_METHOD(void, OnConnectionFailed, (), (override));
    MOCK_METHOD(void, OnConnectionSetupPrepared, (), (override));
};

class MockIMtcRadioConnectionListener : public IMtcRadioConnectionListener
{
public:
    ~MockIMtcRadioConnectionListener() = default;

    MOCK_METHOD(void, OnConnectionFailed,
            (TrafficType eTrafficType, IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode,
                    IN IMS_UINT32 nWaitTimeMillis),
            (override));
    MOCK_METHOD(void, OnConnectionSetupPrepared, (TrafficType eTrafficType), (override));
};

#endif
