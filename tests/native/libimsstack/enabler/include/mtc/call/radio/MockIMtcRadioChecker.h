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

#ifndef MOCK_I_MTC_RADIO_CHECKER_H_
#define MOCK_I_MTC_RADIO_CHECKER_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/radio/IMtcRadioChecker.h"
#include "call/radio/MtcRadioChecker.h"
#include <gmock/gmock.h>

class MockIMtcRadioChecker : public IMtcRadioChecker
{
public:
    ~MockIMtcRadioChecker() = default;

    MOCK_METHOD(
            void, SetTrafficCheckerListener, (IN IMtcRadioCheckerListener * pListener), (override));
    MOCK_METHOD(CheckResult, Check,
            (IN CallType eCallType, IN IMS_BOOL bEmergency, IN PeerType ePeerType,
                    IN IMS_BOOL bWifi, IN CallKey nCallKey),
            (override));
};

class MockIMtcRadioCheckerListener : public IMtcRadioCheckerListener
{
public:
    ~MockIMtcRadioCheckerListener() = default;

    MOCK_METHOD(void, OnConnectionSetupPrepared, (), (override));
    MOCK_METHOD(void, OnConnectionFailed,
            (IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nWaitTimeMillis), (override));
};

class MockIMtcRadioConnectionListener : public IMtcRadioConnectionListener
{
public:
    ~MockIMtcRadioConnectionListener() = default;

    MOCK_METHOD(void, OnConnectionFailed,
            (IN TrafficType eTrafficType, IN CallDirection eCallDirection,
                    IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode,
                    IN IMS_UINT32 nWaitTimeMillis),
            (override));
    MOCK_METHOD(void, OnConnectionSetupPrepared,
            (IN TrafficType eTrafficType, IN CallDirection eCallDirection), (override));
};

#endif
