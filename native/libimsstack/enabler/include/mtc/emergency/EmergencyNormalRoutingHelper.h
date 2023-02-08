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

#ifndef EMERGENCY_NORMAL_ROUTING_HELPER_H_
#define EMERGENCY_NORMAL_ROUTING_HELPER_H_

#include "IMtcCallStateListener.h"
#include "ImsTypeDef.h"
#include "IuMtcService.h"
#include "call/IMtcCall.h"

class IMtcContext;

using EmergencyCallRoutingPdn = IuMtcService::EmergencyCallRoutingPdn;
using EmergencyServiceState = IuMtcService::EmergencyServiceState;

class IEmergencyNormalRoutingHelperListener
{
public:
    virtual ~IEmergencyNormalRoutingHelperListener() = default;
    virtual void OnNormalRoutingClosed() = 0;
};

class EmergencyNormalRoutingHelper final : public IMtcCallStateListener
{
public:
    EmergencyNormalRoutingHelper(
            IN IMtcContext& objContext, IN IEmergencyNormalRoutingHelperListener& objListener);
    ~EmergencyNormalRoutingHelper();
    EmergencyNormalRoutingHelper(IN const EmergencyNormalRoutingHelper&) = delete;
    EmergencyNormalRoutingHelper& operator=(IN const EmergencyNormalRoutingHelper&) = delete;

    void OnCallStateChanged(IN CallKey nCallKey, IN State eState, IN Type eType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason) override;
    void OnTotalCallStateChanged(IN State) override;
    void HandleEmergencyCall();

private:
    void NotifyServiceChanged(IN EmergencyServiceState eState);
    IMtcContext& m_objContext;
    IEmergencyNormalRoutingHelperListener& m_objListener;
};

#endif
