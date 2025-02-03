/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef RTT_AUTO_UPGRADER_H_
#define RTT_AUTO_UPGRADER_H_

#include "ImsTypeDef.h"
#include "IMtcCallStateListener.h"
#include "call/IMtcCall.h"
#include "helper/IPassiveTimerListener.h"

class IMtcContext;
class IMtcSession;

class RttAutoUpgrader final : public IMtcCallStateListener, public IPassiveTimerListener
{
public:
    explicit RttAutoUpgrader(IN IMtcContext& objContext);
    virtual ~RttAutoUpgrader();
    RttAutoUpgrader(IN const RttAutoUpgrader&) = delete;
    RttAutoUpgrader& operator=(IN const RttAutoUpgrader&) = delete;

    static IMS_BOOL IsRequired(IN const MtcConfigurationProxy& objConfigProxy,
            IN const CallInfo& objCallInfo, IN const IMtcSession* pMtcSession);

    // IMtcCallStateListener implementation
    void OnCallStateChanged(IN CallKey nCallKey, IN State eState, IN Type eType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason) override;
    inline void OnTotalCallStateChanged(IN State) override {};
    inline IMS_BOOL IsSynchronousCallRequired() override { return IMS_TRUE; }

    // IPassiveTimerListener implementation
    void OnPassiveTimerExpired(IN IPassiveTimerHolder::Type eType) override;

private:
    void StartRttGuardTimer();
    void DetermineIfRttUpgradeIsNeeded(IN CallKey nCallKey);
    void UpgradeToRttIfNeeded(IN CallKey nCallKey);

    IMtcContext& m_objContext;
    IMS_BOOL m_bRttEmergencyCallEstablished;
    CallKey m_nIncomingVoiceCallKey;
};

#endif
