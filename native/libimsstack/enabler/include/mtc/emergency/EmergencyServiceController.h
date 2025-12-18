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
#include "IMtcService.h"
#include "ImsTypeDef.h"
#include "IuMtcService.h"
#include "call/IMtcCall.h"
#include "emergency/IMtcEmergencyServiceManager.h"
#include "helper/IMtcAosStateListener.h"
#include "helper/IMtcNetworkWatcherListener.h"
#include "helper/IPassiveTimerHolder.h"
#include "helper/IPassiveTimerListener.h"

class IMtcContext;

using EmergencyServiceUnavailableReason = IuMtcService::EmergencyServiceUnavailableReason;

class EmergencyServiceController final :
        public IEmergencyServiceController,
        public IMtcAosStateListener,
        public IMtcCallStateListener,
        public IMtcNetworkWatcherListener,
        public IPassiveTimerListener
{
public:
    explicit EmergencyServiceController(
            IN IMtcEmergencyServiceManager& objServiceManager, IN IMtcContext& objContext);
    virtual ~EmergencyServiceController() override;
    EmergencyServiceController(IN const EmergencyServiceController&) = delete;
    EmergencyServiceController& operator=(IN const EmergencyServiceController&) = delete;

    void Start() override;
    void Close() override;
    inline ServiceType GetServiceType() const override { return ServiceType::EMERGENCY; }
    inline IEmergencyServiceController::State GetState() const override { return m_eState; }

    void OnAosStateChanged(IN IMtcService& objMtcService, IN MtcAosState eState,
            IN IMS_UINT32 eAosReason, IN IMS_SINT32 nDataFailureReason) override;

    void OnCallStateChanged(IN CallKey nCallKey, IN IMtcCall::State eState, IN Type eType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason) override;
    inline void OnTotalCallStateChanged(IN IMtcCall::State) override {}
    void OnCallSessionReleased(
            IN CallKey nCallKey, IN IMS_BOOL bEmergency, IN IMS_BOOL bEstablished) override;
    void OnPassiveTimerExpired(IN IPassiveTimerHolder::Type eType) override;
    void OnRatChanged(IN ServiceType eServiceType, IN IMS_SINT32 eOldRatType,
            IN IMS_SINT32 eRatType) override;

private:
    LOCAL const IMS_SINT32 REASON_UNSPECIFIED = -1;

    enum class RetryOverImsPdnAction
    {
        NO_RETRY,
        RETRY_IMMEDIATELY,
        RETRY_AFTER_HANDOVER
    };

    IMtcEmergencyServiceManager& m_objServiceManager;
    IMtcContext& m_objContext;
    IEmergencyServiceController::State m_eState;

    IMtcCall::State m_eEmergencyCallState;
    CallKey m_nEmergencyCallKey;
    IMS_UINT32 m_ePendingUnavailableAosReasonForHandover;

    void HandleServiceOpened();
    void HandleServiceUnavailable(IN IMS_UINT32 eAosReason);

    void AddListeners();
    void RemoveListeners();

    void Notify(IN IuMtcService::EmergencyServiceState eState,
            IN EmergencyServiceUnavailableReason eReason =
                    EmergencyServiceUnavailableReason::UNKNOWN) const;
    IMS_BOOL ControlAos(IN IMS_UINT32 nType) const;
    void Finish();
    void FinishAndRetryOverImsPdn();
    void SetState(IN IEmergencyServiceController::State eState);
    void Start18xWaitingTimer();
    void Stop18xWaitingTimer();
    void StartWaitForHandoverToRetryOverImsPdnTimer();
    void StopWaitForHandoverToRetryOverImsPdnTimer();

    IMS_BOOL IsCurrentEmergencyCall(IN CallKey nCallKey) const;
    RetryOverImsPdnAction GetRetryOverImsPdnAction(IN IMS_SINT32 eAosReason) const;
    IMS_BOOL IsNetworkRoaming() const;

    EmergencyServiceUnavailableReason ConvertToUnavailableReason(IN IMS_UINT32 eAosReason);
};

#endif
