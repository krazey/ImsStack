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

#ifndef EMERGENCY_SERVICE_CONTROLLER_H_
#define EMERGENCY_SERVICE_CONTROLLER_H_

#include "IMtcCallStateListener.h"
#include "ImsTypeDef.h"
#include "IuMtcService.h"
#include "call/IMtcCall.h"
#include "emergency/IMtcEmergencyServiceManager.h"
#include "helper/IMtcAosStateListener.h"
#include "helper/IPassiveTimerHolder.h"
#include "helper/IPassiveTimerListener.h"

class IMtcContext;

using EmergencyServiceState = IuMtcService::EmergencyServiceState;

class EmergencyServiceController final :
        public IEmergencyServiceController,
        public IMtcAosStateListener,
        public IMtcCallStateListener,
        public IPassiveTimerListener
{
public:
    explicit EmergencyServiceController(
            IN IMtcEmergencyServiceManager& objServiceManager, IN IMtcContext& objContext);
    virtual ~EmergencyServiceController();
    EmergencyServiceController(IN const EmergencyServiceController&) = delete;
    EmergencyServiceController& operator=(IN const EmergencyServiceController&) = delete;

    void Start() override;
    void Close() override;
    inline ServiceType GetServiceType() const override { return ServiceType::EMERGENCY; }

    void OnAosStateChanged(IN IMtcService& objMtcService, IN MtcAosState eState,
            IN IMS_UINT32 eAosReason) override;

    void OnCallStateChanged(IN CallKey nCallKey, IN IMtcCall::State eState, IN Type eType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason) override;
    inline void OnTotalCallStateChanged(IN State) override {}
    void OnCallSessionReleased(
            IN CallKey nCallKey, IN IMS_BOOL bEmergency, IN IMS_BOOL bEstablished) override;
    void OnPassiveTimerExpired(IN IPassiveTimerHolder::Type eType) override;

private:
    enum class State
    {
        IDLE,
        OPENING,
        OPENED,
    };

    LOCAL const IMS_SINT32 REASON_UNSPECIFIED = -1;

    IMtcEmergencyServiceManager& m_objServiceManager;
    IMtcContext& m_objContext;
    State m_eState;

    IMtcCall::State m_eEmergencyCallState;
    CallKey m_nEmergencyCallKey;

    void HandleServiceOpened();
    void HandleServiceUnavailable(IN IMS_UINT32 eAosReason);

    void AddListeners();
    void RemoveListeners();

    void Notify(IN EmergencyServiceState eState, IN IMS_SINT32 eReason = REASON_UNSPECIFIED) const;
    void ControlAos(IN IMS_UINT32 nType) const;
    void Finish();
    void FinishAndRetryOverImsPdn();
    void SetState(IN State eState);
    void Start18xWaitingTimer();
    void Stop18xWaitingTimer();

    IMS_BOOL IsCurrentEmergencyCall(IN CallKey nCallKey) const;
    IMS_BOOL IsRetryOverImsPdnRequired(IN IMS_SINT32 eAosReason) const;
};

#endif
