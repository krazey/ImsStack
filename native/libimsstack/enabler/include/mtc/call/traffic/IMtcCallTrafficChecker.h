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

#ifndef INTERFACE_MTC_CALL_TRAFFIC_CHECKER_H_
#define INTERFACE_MTC_CALL_TRAFFIC_CHECKER_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

using TrafficType = IMS_UINT32;

class IMtcCallTrafficCheckerListener;

class IMtcCallTrafficChecker
{
public:
    ~IMtcCallTrafficChecker() = default;

    virtual void SetTrafficCheckerListener(IN IMtcCallTrafficCheckerListener* pListener) = 0;
    virtual IMS_BOOL IsTrafficPrepared(IN CallType eCallType, IN IMS_BOOL bEmergency) const = 0;
    virtual IMS_BOOL IsTrafficAllowed(IN CallType eCallType, IN IMS_BOOL bEmergency) const = 0;
    virtual void StartTrafficChecking(
            IN CallType eCallType, IN IMS_BOOL bEmergency, IN IMS_BOOL bWifi) = 0;
    virtual void StopTrafficChecking(IN TrafficType eTrafficType) = 0;
    virtual void HandleIpcanChanged(IN IMS_UINT32 eIpcan, IN IMS_BOOL bEmergency) = 0;
};

class IMtcCallTrafficCheckerListener
{
public:
    ~IMtcCallTrafficCheckerListener() = default;

    virtual void OnConnectionFailed() = 0;
    virtual void OnConnectionSetupPrepared() = 0;
};

class IMtcRadioConnectionListener
{
public:
    ~IMtcRadioConnectionListener() = default;

    virtual void OnConnectionFailed(IN TrafficType eTrafficType, IN IMS_UINT32 nFailureReason,
            IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis) = 0;
    virtual void OnConnectionSetupPrepared(IN TrafficType eTrafficType) = 0;
};

#endif
