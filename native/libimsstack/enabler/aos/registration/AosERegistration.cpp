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
#include "ServiceEvent.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"

#include "CarrierConfig.h"
#include "IRegistration.h"
#include "SipStatusCode.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosNetTracker.h"
#include "interface/IAosPcscf.h"
#include "provider/AosLog.h"
#include "provider/AosProvider.h"
#include "registration/AosIpsecHelper.h"

#include "registration/AosERegistration.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define REGID m_strTag.GetStr()

PUBLIC
AosERegistration::AosERegistration(IN IAosAppContext* piAppContext, IN AString& strRegId) :
        AosRegistration(piAppContext, strRegId),
        m_bReinitiationRequested(IMS_FALSE),
        pEModeInfo(IMS_NULL)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosERegistration = %" PFLS_u "/%" PFLS_x, REGID,
            sizeof(AosERegistration), this);
};

PUBLIC VIRTUAL AosERegistration::~AosERegistration()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosERegistration = %" PFLS_u "/%" PFLS_x, REGID,
            sizeof(AosERegistration), this);

    IAosCallTracker* piCt = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
    if (piCt != IMS_NULL)
    {
        piCt->RemoveListener(this);
    }

    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_ECM_STATE, this, m_nSlotId);

    if (pEModeInfo != IMS_NULL)
    {
        delete pEModeInfo;
    }
}

PUBLIC VIRTUAL void AosERegistration::Start()
{
    A_IMS_TRACE_I(
            REGID, "Start :: state(%s)", AosProvider::GetLog()->RegStateToString(m_nState), 0, 0);

    IMS_UINT32 nScheme = GET_N_CONFIG(m_nSlotId)->GetPreferredEmergencyRegistration();

    if (IsFakeModeCondition() ||
            (nScheme == CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_SKIP))
    {
        SetMode(MODE_FAKE);
        SetFakeReg(IMS_TRUE);
    }
    else
    {
        SetMode(MODE_NORMAL);
        SetFakeReg(IMS_FALSE);

        // TODO: need to start the timer from the pdn establishment
        IMS_SINT32 nERegTimer = GET_N_CONFIG(m_nSlotId)->GetEmergencyRegistrationTimerMillis();
        if (nERegTimer > 0)
        {
            StartTimer(TIMER_TRANSACTION, nERegTimer);
        }
    }

    AosRegistration::Start();
}

PUBLIC VIRTUAL void AosERegistration::Update(IN IMS_BOOL bIgnoreRetryTimer /* = IMS_FALSE */,
        IN IMS_BOOL bExplicitUpdate /* = IMS_TRUE */)
{
    (void)bIgnoreRetryTimer;
    (void)bExplicitUpdate;

    A_IMS_TRACE_I(
            REGID, "Update :: state(%s)", AosProvider::GetLog()->RegStateToString(m_nState), 0, 0);

    switch (m_nState)
    {
        case STATE_REGSTOP:  // FALL-THROUGH
        case STATE_REFRESHSTOP:
            ProcessRetryInRegStopped();
            break;

        case STATE_REGISTERED:
            ProcessReregister();
            break;

        default:
            break;
    }
}

PUBLIC VIRTUAL void AosERegistration::RequestCmd(
        IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason /* = 0 */)
{
    switch (nCmdType)
    {
        case CMD_FAKE_MODE:
            HandleFakeMode(nReason);
            break;

        case CMD_ECALL_INIT:  // FALL-THROUGH
        case CMD_ECALL_DONE:
            HandleECallState(nCmdType);
            break;

        case CMD_ESMS_INIT:  // FALL-THROUGH
        case CMD_ESMS_DONE:
            HandleESmsState(nCmdType);
            break;

            // TODO: SCBM handling

        default:
            AosRegistration::RequestCmd(nCmdType, nReason);
            break;
    }
}

PROTECTED VIRTUAL IMS_BOOL AosERegistration::OnMessage(IN IMSMSG& objMsg)
{
    A_IMS_TRACE_I(REGID, "OnMessage :: (%s)",
            AosProvider::GetLog()->RegMessageToString(objMsg.nMSG), 0, 0);

    switch (objMsg.nMSG)
    {
        case MSG_REG_REINITIATE:
            SetReinitiationRequested(IMS_FALSE);
            ProcessReinitiate();
            break;

        case MSG_REG_REINITIATE_WITH_REG_STATE:
            SetReinitiationRequested(IMS_FALSE);
            ProcessReinitiateWithRegState(
                    ((static_cast<IMS_SINT32>(objMsg.nWparam)) > 0) ? IMS_TRUE : IMS_FALSE);
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void AosERegistration::Init()
{
    A_IMS_TRACE_D(REGID, "Init", 0, 0, 0);

    if (GET_N_CONFIG(m_nSlotId)->IsEmergencyCallbackModeSupported())
    {
        pEModeInfo = new EmergencyModeInfo();
        IMS_EVENT_AddListenerForSlotId(IMS_EVENT_ECM_STATE, this, m_nSlotId);
    }

    IAosCallTracker* piCt = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
    if (piCt != IMS_NULL)
    {
        piCt->SetListener(this);
    }

    InitFeatures();
}

PROTECTED VIRTUAL IMS_BOOL AosERegistration::CreateRegistration()
{
    if (!IsFakeRegistration())
    {
        ProcessRearrangePcscf();
    }

    return AosRegistration::CreateRegistration();
}

PROTECTED VIRTUAL void AosERegistration::ProcessAuthenticationFailed()
{
    ProcessDefaultFlowRecovery_Start();
}

PROTECTED VIRTUAL void AosERegistration::ProcessDefaultFlowRecovery_Start(
        IN IMS_SINT32 /* nStatusCode */ /* = 0 */)
{
    if (pEModeInfo != IMS_NULL && !pEModeInfo->IsECall())
    {
        ProcessUnpredictableFailure();
        return;
    }

    if (IsReinitiationRequested())
    {
        A_IMS_TRACE_I(REGID,
                "ProcessDefaultFlowRecovery_Start :: ignore because reg is re-initiated", 0, 0, 0);
        return;
    }

    if (IsFakeRegistration())
    {
        A_IMS_TRACE_I(REGID, "ProcessDefaultFlowRecovery_Start :: fake E-REG is failed", 0, 0, 0);
        ProcessUnpredictableFailure();
        return;
    }

    if (GET_N_CONFIG(m_nSlotId)->GetPreferredEmergencyRegistration() ==
            CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK)
    {
        A_IMS_TRACE_I(REGID, "ProcessDefaultFlowRecovery_Start :: try the fake E-REG", 0, 0, 0);
        ProcessFakeMode();
        return;
    }

    SetState(STATE_REGSTOP);
    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
}

PROTECTED VIRTUAL void AosERegistration::ProcessDefaultFlowRecovery_Update(
        IN IMS_SINT32 /* nStatusCode */ /* = 0 */)
{
    if (!IsImsCall() && pEModeInfo != IMS_NULL && !pEModeInfo->IsEcbm())
    {
        ProcessUnpredictableFailure();
        return;
    }

    SetState(STATE_REFRESHSTOP);
}

PROTECTED VIRTUAL void AosERegistration::ProcessStartFailed_StatusCode(IN IMS_SINT32 nStatusCode)
{
    A_IMS_TRACE_I(REGID, "ProcessStartFailed_StatusCode :: Code(%d) ", nStatusCode, 0, 0);

    switch (nStatusCode)
    {
            // 423
        case SipStatusCode::SC_423:
            ProcessStartFailed_423();
            break;

        default:
            // Other 4xx, 5xx, 6xx response
            ProcessDefaultFlowRecovery_Start(nStatusCode);
            break;
    }
}

PROTECTED VIRTUAL void AosERegistration::ProcessStartFailed_TxnTimeout()
{
    ProcessDefaultFlowRecovery_Start();
}

PROTECTED VIRTUAL void AosERegistration::ProcessStartFailed_Others(IN IMS_SINT32 nReason)
{
    (void)nReason;

    ProcessDefaultFlowRecovery_Start();
}

PROTECTED VIRTUAL void AosERegistration::ProcessUpdateFailed_StatusCode(IN IMS_SINT32 nStatusCode)
{
    A_IMS_TRACE_I(REGID, "ProcessUpdateFailed_StatusCode :: Code(%d) ", nStatusCode, 0, 0);

    switch (nStatusCode)
    {
        // 423
        case SipStatusCode::SC_423:
            ProcessUpdateFailed_423();
            break;

        default:
            // other 4xx, 5xx, 6xx response
            ProcessDefaultFlowRecovery_Update(nStatusCode);
            break;
    }
}

PROTECTED VIRTUAL void AosERegistration::ProcessUpdateFailed_TxnTimeout()
{
    ProcessDefaultFlowRecovery_Update();
}

PROTECTED VIRTUAL void AosERegistration::ProcessUpdateFailed_Others(IN IMS_SINT32 nReason)
{
    (void)nReason;

    ProcessDefaultFlowRecovery_Update();
}

PROTECTED VIRTUAL void AosERegistration::ProcessStopRetryTimerExpired()
{
    StopTimer(TIMER_STOP_RETRY);

    m_nConsecutiveFailure++;
    ClearAuthChallengedCount();

    if (IsRetryAllowed() && SetNextPcscf() && SendRegister(IMS_TRUE))
    {
        SetState(STATE_REGISTERING);

        if (GetRetryTime() > 0)
        {
            StartTimer(TIMER_STOP_RETRY, GetRetryTime() * 1000);
        }

        return;
    }

    ProcessFakeMode();
}

PROTECTED VIRTUAL void AosERegistration::ProcessTransactionTimerExpired()
{
    StopTimer(TIMER_TRANSACTION);

    if (GetState() != STATE_REGISTERING)
    {
        return;
    }

    if (GET_N_CONFIG(m_nSlotId)->GetPreferredEmergencyRegistration() ==
            CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK)
    {
        A_IMS_TRACE_I(REGID, "ProcessTransactionTimerExpired :: try the fake E-REG", 0, 0, 0);
        ProcessFakeMode();
        return;
    }
    else
    {
        ProcessUnpredictableFailure();
    }
}

PROTECTED VIRTUAL void AosERegistration::SetRefreshPolicy()
{
    m_piRegistration->SetRefreshPolicy(IRegistration::REFRESH_POLICY_RATIO, 1200, 50, 50);
}

PROTECTED VIRTUAL void AosERegistration::UpdateTransactionStarted()
{
    if (pEModeInfo == IMS_NULL || !GET_N_CONFIG(m_nSlotId)->IsEmergencyCallbackModeSupported())
    {
        return;
    }

    if (pEModeInfo->IsEcbmCheckedForRefresh() && IsEcbmTimer())
    {
        m_bIsTransactionStarted = IsImsCall() || pEModeInfo->IsEcbm() || pEModeInfo->IsScbm();
    }
    else
    {
        m_bIsTransactionStarted = IsImsCall();
    }

    A_IMS_TRACE_D(REGID, "UpdateTransactionStarted :: (%s)",
            (m_bIsTransactionStarted) ? "READY" : "NOT READY", 0, 0);
}

PROTECTED VIRTUAL void AosERegistration::Registration_RefreshTimerExpired(
        OUT IMS_BOOL& bDoImplicitRefresh)
{
    A_IMS_TRACE_I(REGID, "Registration_RefreshTimerExpired", 0, 0, 0);

    bDoImplicitRefresh = IMS_FALSE;

    if (m_piRegistration == IMS_NULL)
    {
        return;
    }

    UpdateTransactionStarted();

    if (!IsTransactionStarted())
    {
        A_IMS_TRACE_I(
                REGID, "Registration_RefreshTimerExpired :: transaction is not started", 0, 0, 0);
        SetState(STATE_REFRESHSTOP);
        return;
    }

    if (IsIpsecSupported())
    {
        if (!m_pIpsecHelper->Create(IMS_FALSE))
        {
            ProcessRegTerminated();
            return;
        }
    }

    bDoImplicitRefresh = IMS_TRUE;

    SetState(STATE_REFRESHING);
    ReportTryingState();
}

PROTECTED VIRTUAL void AosERegistration::Registration_Started()
{
    StopTimer(TIMER_STOP_RETRY);
    StopTimer(TIMER_TRANSACTION);
    ProcessEMode();
    AosRegistration::Registration_Started();
}

PROTECTED VIRTUAL void AosERegistration::Registration_Updated()
{
    ProcessEMode();
    AosRegistration::Registration_Updated();
}

PROTECTED VIRTUAL void AosERegistration::Registration_StartFailed(IN IMS_SINT32 nReason)
{
    StopTimer(TIMER_TRANSACTION);
    AosRegistration::Registration_StartFailed(nReason);
}

PROTECTED VIRTUAL void AosERegistration::Registration_Terminated(IN IMS_SINT32 nReason)
{
    if (IsImsCall())
    {
        A_IMS_TRACE_I(REGID, "Registration_Terminated :: terminated is holding", 0, 0, 0);
        m_pUtil->AddFeature(PENDING_TERMINATED, m_nTxnPending);

        if (IsIpsecSupported())
        {
            m_pIpsecHelper->IgnoreCurrentPolicyExpired();
        }
    }
    else
    {
        AosRegistration::Registration_Terminated(nReason);
    }
}

PROTECTED VIRTUAL void AosERegistration::CallTracker_StateChanged(
        IN IMS_UINT32 nType, IN CallState eState)
{
    if (nType != IAosCallTracker::TYPE_EMERGENCY)
    {
        return;
    }

    IMS_BOOL bCurrState = (eState > CallState::IDLE) ? IMS_TRUE : IMS_FALSE;
    if (IsImsCall() != bCurrState)
    {
        SetImsCall(bCurrState);
        UpdateTransactionStarted();

        if (bCurrState)
        {
            ProcessECallStarted();
        }
    }
}

PROTECTED VIRTUAL void AosERegistration::NConfiguration_NotifyConfigChanged()
{
    if (GET_N_CONFIG(m_nSlotId) != IMS_NULL)
    {
        AosRegistration::NConfiguration_NotifyConfigChanged();

        if (pEModeInfo == IMS_NULL)
        {
            pEModeInfo = new EmergencyModeInfo();
            IMS_EVENT_AddListenerForSlotId(IMS_EVENT_ECM_STATE, this, m_nSlotId);
        }
    }
}

PROTECTED VIRTUAL void AosERegistration::Event_NotifyEvent(
        IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 /* nLParam */)
{
    if (pEModeInfo == IMS_NULL || nEvent != IMS_EVENT_ECM_STATE)
    {
        return;
    }

    A_IMS_TRACE_D(REGID, "ecm state :: (%d)", nWParam, 0, 0);

    if (nWParam == IMS_ECM_STATE_ON)
    {
        if (pEModeInfo->IsScbm())
        {
            pEModeInfo->SetScbm(IMS_FALSE);
            StopTimer(TIMER_MODE);
        }

        pEModeInfo->SetEcbm(IMS_TRUE);
        StartTimer(TIMER_MODE, (EmergencyModeInfo::EMERGENCY_CALLBACK_MODE_TIME / 2) * 1000);
    }
    else if (nWParam == IMS_ECM_STATE_OFF || nWParam == IMS_ECM_STATE_OFF_BY_NEW_ECALL)
    {
        pEModeInfo->SetEcbm(IMS_FALSE);
        StopTimer(TIMER_MODE);

        if (nWParam == IMS_ECM_STATE_OFF)
        {
            ProcessUnpredictableFailure();
        }
    }
}

PROTECTED IMS_UINT32 AosERegistration::GetRetryTime()
{
    IMSVector<IMS_SINT32>& objWaitTime = GET_N_CONFIG(m_nSlotId)->GetEmergencyPcscfRetryWaitTime();
    IMS_UINT32 nRetryMaxCount = objWaitTime.GetSize();

    if (nRetryMaxCount == 0)
    {
        return 0;
    }

    return (m_nConsecutiveFailure < nRetryMaxCount) ? objWaitTime.GetAt(m_nConsecutiveFailure)
                                                    : objWaitTime.GetAt(nRetryMaxCount - 1);
}

PROTECTED void AosERegistration::HandleECallState(IN IMS_UINT32 nState)
{
    if (pEModeInfo == IMS_NULL || !GET_N_CONFIG(m_nSlotId)->IsEmergencyCallbackModeSupported())
    {
        return;
    }

    pEModeInfo->SetECall(nState == CMD_ECALL_INIT);
}

PROTECTED void AosERegistration::HandleESmsState(IN IMS_UINT32 nState)
{
    if (pEModeInfo == IMS_NULL || !GET_N_CONFIG(m_nSlotId)->IsEmergencySmsOverImsSupported())
    {
        return;
    }

    pEModeInfo->SetESms(nState == CMD_ESMS_INIT);
}

PROTECTED void AosERegistration::HandleFakeMode(IN IMS_UINT32 nReason)
{
    A_IMS_TRACE_I(REGID, "HandleFakeMode :: try the fake E-REG with nReason[%d]", nReason, 0, 0);

    if (nReason == IAosRegistration::REASON_FAKE_MODE_NEXT_PCSCF)
    {
        m_piContext->GetPcscf()->RemoveCurrentPcscf();

        if (SetFirstPcscf(IMS_FALSE) == IMS_FALSE)
        {
            Destroy();
            ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
            return;
        }
    }

    ProcessFakeModeWithRegState(IsRegistered());
}

PROTECTED IMS_BOOL AosERegistration::IsEcbmTimer() const
{
    return (m_piModeTimer != IMS_NULL);
}

PROTECTED IMS_BOOL AosERegistration::IsFakeModeCondition()
{
    return m_piContext->GetBlock()->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED) ||
            m_piContext->GetNetTracker()->IsEmergencyLteAttach();
}

PROTECTED IMS_BOOL AosERegistration::IsReinitiationRequested() const
{
    return m_bReinitiationRequested;
}

PROTECTED IMS_BOOL AosERegistration::IsRetryAllowed() const
{
    IMSVector<IMS_SINT32>& objWaitTime = GET_N_CONFIG(m_nSlotId)->GetEmergencyPcscfRetryWaitTime();

    return m_nConsecutiveFailure < objWaitTime.GetSize();
}

PROTECTED void AosERegistration::ProcessECallStarted()
{
    if (GetState() == STATE_REFRESHSTOP)
    {
        A_IMS_TRACE_I(REGID, "ProcessECallStarted :: re-reg is trying in refresh stop", 0, 0, 0);
        ProcessRetryInRegStopped();
    }
}

PROTECTED void AosERegistration::ProcessEMode()
{
    if (!GET_N_CONFIG(m_nSlotId)->IsEmergencyCallbackModeSupported() || pEModeInfo == IMS_NULL)
    {
        return;
    }

    pEModeInfo->SetEcbmCheckedForRefresh(IMS_FALSE);

    if (GET_N_CONFIG(m_nSlotId)->IsEmergencySmsOverImsSupported())
    {
        pEModeInfo->SetEcbmCheckedForRefresh(IMS_FALSE);
    }

    if (IsFakeRegistration() || GetRegExpires() <= 0)
    {
        return;
    }

    if ((GetRegExpires() / 2) < (EmergencyModeInfo::EMERGENCY_CALLBACK_MODE_TIME * 2))
    {
        A_IMS_TRACE_D(REGID, "ProcessEMode :: check ecbm for refreshing", 0, 0, 0);
        pEModeInfo->SetEcbmCheckedForRefresh(IMS_TRUE);

        if (GET_N_CONFIG(m_nSlotId)->IsEmergencySmsOverImsSupported())
        {
            A_IMS_TRACE_D(REGID, "ProcessEMode :: check scbm for refreshing", 0, 0, 0);
            pEModeInfo->SetScbmCheckedForRefresh(IMS_TRUE);
        }
    }
}

PROTECTED void AosERegistration::ProcessFakeMode()
{
    A_IMS_TRACE_I(REGID, "ProcessFakeMode", 0, 0, 0);

    SetFakeReg(IMS_TRUE);
    SetMode(MODE_FAKE);
    SetReinitiationRequested(IMS_TRUE);
    PostMessage(MSG_REG_REINITIATE, 0, 0);
}

PROTECTED void AosERegistration::ProcessFakeModeWithRegState(IN IMS_BOOL bIsRegistered)
{
    A_IMS_TRACE_I(REGID, "ProcessFakeModeWithRegState", 0, 0, 0);

    SetFakeReg(IMS_TRUE);
    SetMode(MODE_FAKE);
    SetReinitiationRequested(IMS_TRUE);
    PostMessage(MSG_REG_REINITIATE_WITH_REG_STATE, bIsRegistered ? 1 : 0, 0);
}

PROTECTED void AosERegistration::ProcessRearrangePcscf()
{
    IMSVector<IMS_SINT32>& objWaitTime = GET_N_CONFIG(m_nSlotId)->GetEmergencyPcscfRetryWaitTime();

    IMS_UINT32 nRetryMaxCount = objWaitTime.GetSize();
    AStringArray objPcscfs = m_piContext->GetPcscf()->GetPcscfs();

    if (nRetryMaxCount == 0)
    {
        return;
    }

    if (GetRetryTime() > 0)
    {
        StartTimer(TIMER_STOP_RETRY, GetRetryTime() * 1000);
    }

    if (objPcscfs.GetCount() < static_cast<IMS_SINT32>(nRetryMaxCount))
    {
        return;
    }

    AStringArray objNewPcscfs;
    for (IMS_UINT32 nAt = 0; nAt < nRetryMaxCount; ++nAt)
    {
        IMS_UINT32 nRange = nRetryMaxCount - nAt;

        if (nRange > 1)
        {
            IMS_UINT32 nPosition = IMS_SYS_GetRandom(nRange);
            AString strPcscf = objPcscfs.GetElementAt(nPosition);

            objNewPcscfs.AddElement(strPcscf);
            objPcscfs.RemoveElementAt(nPosition);

            A_IMS_TRACE_D(REGID, "ProcessRearrangePcscf :: range (%d) , position (%d) , pcscf (%s)",
                    nRange, nPosition, strPcscf.GetStr());
        }
        else
        {
            objNewPcscfs.AddElement(objPcscfs.GetElementAt(0));
        }
    }

    m_piContext->GetPcscf()->UpdatePcscfs(objNewPcscfs);
}

PROTECTED void AosERegistration::ProcessReinitiateWithRegState(IN IMS_BOOL bIsRegistered)
{
    A_IMS_TRACE_I(REGID, "ProcessReinitiate - bIsRegistered [%s]",
            (bIsRegistered == IMS_TRUE) ? "TRUE" : "FALSE", 0, 0);

    Destroy();

    if (!CreateRegistration())
    {
        ProcessUnpredictableFailure();
        return;
    }

    if (bIsRegistered == IMS_TRUE)
    {
        SetState(STATE_REFRESHING);
    }
    else
    {
        SetState(STATE_REGISTERING);
    }

    ReportTryingState();
}

PROTECTED VIRTUAL void AosERegistration::SetReinitiationRequested(IN IMS_BOOL bRequest)
{
    A_IMS_TRACE_I(REGID, "SetReinitiationRequested :: (%s)", (bRequest) ? "ON" : "OFF", 0, 0);

    m_bReinitiationRequested = bRequest;
}