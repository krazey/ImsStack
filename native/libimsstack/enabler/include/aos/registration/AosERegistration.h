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
#ifndef AOS_E_REGISTRATION_H_
#define AOS_E_REGISTRATION_H_

#include "ServiceSystemTime.h"
#include "interface/IAosEmergencyListener.h"
#include "registration/AosRegistration.h"

class EmergencyModeInfo
{
public:
    EmergencyModeInfo() :
            m_bECall(IMS_FALSE),
            m_bEcbm(IMS_FALSE),
            m_bScbm(IMS_FALSE),
            m_bESms(IMS_FALSE),
            m_nCbmDurationSec(0),
            m_nCbmBeginTimeSec(0),
            m_nReregTryTimeSec(0)
    {
    }
    virtual ~EmergencyModeInfo() {}

    inline IMS_BOOL IsECall() { return m_bECall; }
    inline IMS_BOOL IsEcbm() { return m_bEcbm; }
    inline IMS_BOOL IsScbm() { return m_bScbm; }
    inline IMS_BOOL IsESms() { return m_bESms; }
    inline IMS_ULONG GetCbmDuration() { return m_nCbmDurationSec; }
    inline IMS_UINT32 GetCbmBeginTime() { return m_nCbmBeginTimeSec; }
    inline IMS_UINT32 GetReRegTryTime() { return m_nReregTryTimeSec; }

    inline void SetECall(IN IMS_BOOL bECall) { m_bECall = bECall; }
    inline void SetEcbm(IN IMS_BOOL bEcbm) { m_bEcbm = bEcbm; }
    inline void SetScbm(IN IMS_BOOL bScbm) { m_bScbm = bScbm; }
    inline void SetESms(IN IMS_BOOL bESms) { m_bESms = bESms; }
    inline void SetCbmDuration(IN IMS_ULONG nCbmDurationSec)
    {
        m_nCbmDurationSec = nCbmDurationSec;
    }
    inline void SetCbmBeginTime(IN IMS_UINT32 nCbmBeginTimeSec)
    {
        m_nCbmBeginTimeSec = nCbmBeginTimeSec;
    }
    inline void SetReRegTryTime(IN IMS_UINT32 nReregTryTimeSec)
    {
        m_nReregTryTimeSec = nReregTryTimeSec;
    }

private:
    IMS_BOOL m_bECall;
    IMS_BOOL m_bEcbm;
    IMS_BOOL m_bScbm;
    IMS_BOOL m_bESms;
    IMS_ULONG m_nCbmDurationSec;
    IMS_UINT32 m_nCbmBeginTimeSec;
    IMS_UINT32 m_nReregTryTimeSec;
};

class AosERegistration : public AosRegistration, public IAosEmergencyListener
{
public:
    AosERegistration(IN IAosAppContext* piAppContext, IN AString& strRegId);
    ~AosERegistration() override;

    void Start() final;
    void Update(IN IMS_BOOL bIgnoreRetryTimer = IMS_FALSE,
            IN IMS_BOOL bExplicitUpdate = IMS_TRUE) final;
    void RequestCmd(IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason = 0) final;

    IMS_BOOL IsInCallbackMode() final;

protected:
    IMS_BOOL OnMessage(IN IMSMSG& objMsg) final;

    void Init() final;
    void CleanUp() final;
    IMS_BOOL CreateRegistration() final;
    void DestroyRegistration() final;

    void ProcessAuthenticationFailed() final;

    void ProcessDefaultFlowRecovery_Start(IN IMS_SINT32 nStatusCode = 0) final;
    void ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(IN IMS_UINT32 nRetryAfter);
    void ProcessDefaultFlowRecovery_Update(IN IMS_SINT32 nStatusCode = 0) final;

    IMS_BOOL ProcessStartFailed_305() final;

    void ProcessStartFailed_StatusCode(IN IMS_SINT32 nStatusCode) final;
    void ProcessStartFailed_TxnTimeout() final;
    void ProcessStartFailed_Others(IN IMS_SINT32 nReason) final;

    void ProcessUpdateFailed_StatusCode(IN IMS_SINT32 nStatusCode) final;
    void ProcessUpdateFailed_TxnTimeout() final;
    void ProcessUpdateFailed_Others(IN IMS_SINT32 nReason) final;

    void ProcessStopRetryTimerExpired() final;
    void ProcessTransactionTimerExpired() final;

    void SetRefreshPolicy() final;
    void SetReregFailureReportOnIpcanChangeRequired(IN IMS_BOOL bRequired) final;

    void UpdateTransactionStarted() final;

    void Registration_RefreshTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh) final;
    void Registration_Started() final;
    void Registration_Updated() final;
    void Registration_StartFailed(IN IMS_SINT32 nReason) final;
    void Registration_Terminated(IN IMS_SINT32 nReason) final;

    void CallTracker_StateChanged(IN IMS_UINT32 nType, IN CallState eState) final;

    void NConfiguration_NotifyConfigChanged() final;

    void Transaction_OnConnectionFailed(IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode,
            IN IMS_UINT32 nWaitTimeMillis) final;
    void Transaction_OnConnectionSetupPrepared() final;
    void Transaction_OnTrafficPriorityChanged() final;

    void ClearCbm();

    /// IAosEmergencyListener
    void CallbackModeChanged(IN EmergencyCallbackModeType eType, IN EmergencyCallbackMode eState,
            IN IMS_ULONG nDuration) override;

    void HandleECallState(IN IMS_UINT32 nState);
    void HandleESmsState(IN IMS_UINT32 nState);
    void HandleFakeMode(IN IMS_UINT32 nReason);

    IMS_BOOL IsRefreshRequiredByCbm();
    IMS_BOOL IsFakeModeCondition();
    IMS_BOOL IsReinitiationRequested() const;
    IMS_BOOL IsRetryAllowed() const;

    void ProcessReRegStart();
    void ProcessFakeMode();
    void ProcessFakeModeWithRegState(IN IMS_BOOL bIsRegistered);
    void ProcessRearrangePcscf();
    void ProcessReinitiateWithRegState(IN IMS_BOOL bIsRegistered);
    IMS_BOOL ProcessNormalDefaultFlowRecovery_Start(IN IMS_SINT32 nStatusCode);

    void SetReinitiationRequested(IN IMS_BOOL bRequest);
    void SetCallbackMode(IN EmergencyCallbackModeType eType, IN IMS_BOOL bEnable);
    void StartRegRetryTimer();

    IMS_UINT32 GetPreferredRegScheme();

private:
    IMS_BOOL m_bReinitiationRequested;

protected:
    EmergencyModeInfo* m_pEModeInfo;
};
#endif  // AOS_E_REGISTRATION_H_
