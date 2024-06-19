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
#include "ServicePhoneInfo.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"

#include "IImsRadio.h"

#include "CarrierConfig.h"
#include "IRegistration.h"
#include "SipStatusCode.h"
#include "IAosService.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosEmergencyListener.h"
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
        m_pEModeInfo(IMS_NULL)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosERegistration = %" PFLS_u "/%" PFLS_x, REGID,
            sizeof(AosERegistration), this);
};

PUBLIC VIRTUAL AosERegistration::~AosERegistration()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosERegistration = %" PFLS_u "/%" PFLS_x, REGID,
            sizeof(AosERegistration), this);
}

PUBLIC VIRTUAL void AosERegistration::Start()
{
    A_IMS_TRACE_I(
            REGID, "Start :: state(%s)", AosProvider::GetLog()->RegStateToString(m_nState), 0, 0);

    IMS_UINT32 nScheme = GetPreferredRegScheme();

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
        m_pEModeInfo = new EmergencyModeInfo();

        IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
        if (piService != IMS_NULL)
        {
            piService->AddListener(DYNAMIC_CAST(IAosEmergencyListener*, this));
        }
    }

    IAosCallTracker* piCt = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
    if (piCt != IMS_NULL)
    {
        piCt->SetListener(this);
    }

    InitFeatures();

    SetTrafficListener(IMS_TRUE);
}

PROTECTED VIRTUAL void AosERegistration::CleanUp()
{
    A_IMS_TRACE_D(REGID, "CleanUp", 0, 0, 0);

    DestroyEx();

    StopTimer(TIMER_OFFLINE_RECOVER);

    SetTrafficListener(IMS_FALSE);

    IAosCallTracker* piCt = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
    if (piCt != IMS_NULL)
    {
        piCt->RemoveListener(this);
    }

    IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (piService != IMS_NULL)
    {
        piService->RemoveListener(DYNAMIC_CAST(IAosEmergencyListener*, this));
    }

    if (m_pEModeInfo != IMS_NULL)
    {
        delete m_pEModeInfo;
        m_pEModeInfo = IMS_NULL;
    }
}

PROTECTED VIRTUAL IMS_BOOL AosERegistration::CreateRegistration()
{
    if (IsFakeRegistration())
    {
        return AosRegistration::CreateRegistration();
    }

    ProcessRearrangePcscf();

    IMS_BOOL bResult = AosRegistration::CreateRegistration();

    if (bResult)
    {
        StartRegRetryTimer();
    }

    return bResult;
}

PROTECTED VIRTUAL void AosERegistration::DestroyRegistration()
{
    ClearCbm();
    AosRegistration::DestroyRegistration();
}

PROTECTED VIRTUAL void AosERegistration::ProcessAuthenticationFailed()
{
    ProcessDefaultFlowRecovery_Start();
}

PROTECTED VIRTUAL void AosERegistration::ProcessDefaultFlowRecovery_Start(
        IN IMS_SINT32 /* nStatusCode */ /* = 0 */)
{
    if (m_pEModeInfo != IMS_NULL && !m_pEModeInfo->IsECall())
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
    SetTraffic(IMS_FALSE);
    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
}

PROTECTED VIRTUAL void AosERegistration::ProcessDefaultFlowRecovery_Update(
        IN IMS_SINT32 /* nStatusCode */ /* = 0 */)
{
    if (!IsImsCall() && m_pEModeInfo != IMS_NULL && !m_pEModeInfo->IsEcbm() &&
            !m_pEModeInfo->IsScbm())
    {
        ProcessUnpredictableFailure();
        return;
    }

    SetState(STATE_REFRESHSTOP);
    SetTraffic(IMS_FALSE);
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

PROTECTED VIRTUAL void AosERegistration::ProcessTransactionTimerExpired()
{
    StopTimer(TIMER_TRANSACTION);

    if (GetState() != STATE_REGISTERING)
    {
        return;
    }

    m_nConsecutiveFailure++;
    ClearAuthChallengedCount();

    if (IsRetryAllowed() && SetNextPcscf() && SendRegister(IMS_TRUE))
    {
        StartRegRetryTimer();
        return;
    }

    if (GET_N_CONFIG(m_nSlotId)->GetPreferredEmergencyRegistration() ==
            CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK)
    {
        A_IMS_TRACE_I(REGID, "ProcessTransactionTimerExpired :: try the fake E-REG", 0, 0, 0);

        ProcessFakeMode();
        return;
    }

    ProcessUnpredictableFailure();
}

PROTECTED VIRTUAL void AosERegistration::SetRefreshPolicy()
{
    m_piRegistration->SetRefreshPolicy(IRegistration::REFRESH_POLICY_RATIO, 1200, 50, 50);
}

PROTECTED VIRTUAL void AosERegistration::UpdateTransactionStarted()
{
    if (m_pEModeInfo == IMS_NULL || !GET_N_CONFIG(m_nSlotId)->IsEmergencyCallbackModeSupported())
    {
        m_bIsTransactionStarted = IsImsCall();
    }
    else
    {
        m_bIsTransactionStarted = IsImsCall() || IsRefreshRequiredByCbm();
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

    if (m_pEModeInfo != IMS_NULL)
    {
        m_pEModeInfo->SetReRegTryTime(IMS_SYS_GetTimeInSeconds());
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

    SetTraffic(IMS_TRUE);
    SetState(STATE_REFRESHING);
    ReportTryingState();
}

PROTECTED VIRTUAL void AosERegistration::Registration_Started()
{
    StopTimer(TIMER_TRANSACTION);
    SetTraffic(IMS_FALSE);
    AosRegistration::Registration_Started();
}

PROTECTED VIRTUAL void AosERegistration::Registration_Updated()
{
    SetTraffic(IMS_FALSE);
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
            ProcessReRegStart();
        }
    }
}

PROTECTED VIRTUAL void AosERegistration::NConfiguration_NotifyConfigChanged()
{
    if (GET_N_CONFIG(m_nSlotId) != IMS_NULL)
    {
        AosRegistration::NConfiguration_NotifyConfigChanged();

        if (GET_N_CONFIG(m_nSlotId)->IsEmergencyCallbackModeSupported())
        {
            if (m_pEModeInfo == IMS_NULL)
            {
                m_pEModeInfo = new EmergencyModeInfo();
                IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
                if (piService != IMS_NULL)
                {
                    piService->AddListener(DYNAMIC_CAST(IAosEmergencyListener*, this));
                }
            }
        }
        else
        {
            if (m_pEModeInfo != IMS_NULL)
            {
                IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
                if (piService != IMS_NULL)
                {
                    piService->RemoveListener(DYNAMIC_CAST(IAosEmergencyListener*, this));
                }
                delete m_pEModeInfo;
                m_pEModeInfo = IMS_NULL;
            }
        }
    }
}

PROTECTED VIRTUAL void AosERegistration::Transaction_OnConnectionFailed(
        IN IMS_UINT32 nFailureReason, IN IMS_UINT32 /* nCauseCode */,
        IN IMS_UINT32 /* nWaitTimeMillis */)
{
    if (IsRegistered())
    {
        return;
    }

    if (nFailureReason == IImsRadio::REASON_ACCESS_DENIED)
    {
        ProcessUnpredictableFailure();
    }
}

PROTECTED VIRTUAL void AosERegistration::Transaction_OnConnectionSetupPrepared() {}

PROTECTED VIRTUAL void AosERegistration::Transaction_OnTrafficPriorityChanged() {}

PROTECTED VIRTUAL void AosERegistration::ClearCbm()
{
    if (m_pEModeInfo == IMS_NULL || !GET_N_CONFIG(m_nSlotId)->IsEmergencyCallbackModeSupported())
    {
        return;
    }

    m_pEModeInfo->SetEcbm(IMS_FALSE);
    m_pEModeInfo->SetScbm(IMS_FALSE);
    m_pEModeInfo->SetCbmBeginTime(0);
    m_pEModeInfo->SetReRegTryTime(0);
}

PROTECTED void AosERegistration::CallbackModeChanged(
        IN EmcCallbackModeType eType, IN EmcCallbackMode eState, IN IMS_ULONG nDuration)
{
    if (m_pEModeInfo == IMS_NULL || !GET_N_CONFIG(m_nSlotId)->IsEmergencyCallbackModeSupported())
    {
        return;
    }

    A_IMS_TRACE_I(REGID, "CallbackModeChanged eType (%d), eState(%d), nDuration(%d)", eType, eState,
            nDuration);

    if (eState == EmcCallbackMode::START)
    {
        if (eType == EmcCallbackModeType::CALL)
        {
            m_pEModeInfo->SetEcbm(IMS_TRUE);
        }
        else
        {
            m_pEModeInfo->SetScbm(IMS_TRUE);
        }

        m_pEModeInfo->SetCbmDuration(nDuration);
        m_pEModeInfo->SetCbmBeginTime(IMS_SYS_GetTimeInSeconds());

        if (GetState() != STATE_OFFLINE)
        {
            UpdateTransactionStarted();
            ProcessReRegStart();
        }
    }
    else
    {
        if (eType == EmcCallbackModeType::CALL)
        {
            m_pEModeInfo->SetEcbm(IMS_FALSE);
        }
        else
        {
            m_pEModeInfo->SetScbm(IMS_FALSE);
        }

        if (eState == EmcCallbackMode::STOP)
        {
            ProcessUnpredictableFailure();
        }
    }
}

PROTECTED void AosERegistration::HandleECallState(IN IMS_UINT32 nState)
{
    if (m_pEModeInfo == IMS_NULL || !GET_N_CONFIG(m_nSlotId)->IsEmergencyCallbackModeSupported())
    {
        return;
    }

    m_pEModeInfo->SetECall(nState == CMD_ECALL_INIT);
}

PROTECTED void AosERegistration::HandleESmsState(IN IMS_UINT32 nState)
{
    if (m_pEModeInfo == IMS_NULL || !GET_N_CONFIG(m_nSlotId)->IsEmergencySmsOverImsSupported())
    {
        return;
    }

    m_pEModeInfo->SetESms(nState == CMD_ESMS_INIT);
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

PROTECTED IMS_BOOL AosERegistration::IsRefreshRequiredByCbm()
{
    if (!m_pEModeInfo->IsEcbm() && !m_pEModeInfo->IsScbm())
    {
        return IMS_FALSE;
    }

    if (m_pEModeInfo->IsESms())
    {
        return IMS_TRUE;
    }

    IMS_UINT32 nEndTimeSec = IMS_SYS_GetTimeInSeconds();
    IMS_UINT32 nDiffTimeOffSec = nEndTimeSec - m_pEModeInfo->GetCbmBeginTime();
    IMS_TRACE_D(
            "IsRefreshRequiredByCbm()(seconds):: nDiffTime(%d), CBM Duration(%d) GetRegExpires(%d)",
            nDiffTimeOffSec, m_pEModeInfo->GetCbmDuration(), GetRegExpires());

    if (nDiffTimeOffSec == 0 && m_pEModeInfo->GetReRegTryTime() > 0)
    {
        return (m_pEModeInfo->GetCbmDuration() >
                       (static_cast<IMS_UINT32>(GetRegExpires()) / 2 -
                               (nEndTimeSec - m_pEModeInfo->GetReRegTryTime())))
                ? IMS_TRUE
                : IMS_FALSE;
    }

    return ((static_cast<IMS_SINT32>(m_pEModeInfo->GetCbmDuration()) -
                    static_cast<IMS_SINT32>(nDiffTimeOffSec)) > (GetRegExpires()) / 2)
            ? IMS_TRUE
            : IMS_FALSE;
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
    IMS_SINT32 nMaxRetryCnt = GET_N_CONFIG(m_nSlotId)->GetEmcRegRetryMaxCnt();
    if (nMaxRetryCnt == 0)
    {
        AStringArray objPcscfs = m_piContext->GetPcscf()->GetPcscfs();
        return (m_nConsecutiveFailure < objPcscfs.GetCount());
    }

    return m_nConsecutiveFailure <= nMaxRetryCnt;
}

PROTECTED void AosERegistration::ProcessReRegStart()
{
    if (GetState() == STATE_REFRESHSTOP)
    {
        A_IMS_TRACE_I(REGID, "ProcessReRegStart :: re-reg is trying in refresh stop", 0, 0, 0);
        if (IsTransactionStarted())
        {
            ProcessRetryInRegStopped();
        }
    }
}

PROTECTED void AosERegistration::ProcessFakeMode()
{
    A_IMS_TRACE_I(REGID, "ProcessFakeMode", 0, 0, 0);

    NotifyDeregistered();
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
    if (!GET_N_CONFIG(m_nSlotId)->IsEmcRegOnRandomPcscf())
    {
        return;
    }

    AStringArray objPcscfs = m_piContext->GetPcscf()->GetPcscfs();
    IMS_UINT32 nNumOfPcscfs = objPcscfs.GetCount();

    if (nNumOfPcscfs < 2)
    {
        A_IMS_TRACE_D(
                REGID, "ProcessRearrangePcscf :: don't need to rearrange (%d)", nNumOfPcscfs, 0, 0);
        return;
    }

    AStringArray objNewPcscfs;
    for (IMS_UINT32 nAt = 0; nAt < nNumOfPcscfs; ++nAt)
    {
        IMS_UINT32 nRange = nNumOfPcscfs - nAt;

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

            A_IMS_TRACE_D(REGID, "ProcessRearrangePcscf :: range (1) , position (0) , pcscf (%s)",
                    objPcscfs.GetElementAt(0).GetStr(), 0, 0);
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

PROTECTED void AosERegistration::StartRegRetryTimer()
{
    IMS_SINT32 nERegRetryTimer = GET_N_CONFIG(m_nSlotId)->GetEmcRegRetryTimerMillis();

    if (nERegRetryTimer > 0)
    {
        StartTimer(TIMER_TRANSACTION, nERegRetryTimer);
    }
}

PROTECTED VIRTUAL IMS_UINT32 AosERegistration::GetPreferredRegScheme()
{
    IMS_SINT32 nRoamingScheme = GET_N_CONFIG(m_nSlotId)->GetRoamingPreferredEmcReg();

    if (nRoamingScheme != CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NOT_DEFINED)
    {
        if (PhoneInfoService::GetPhoneInfoService()
                        ->GetNetworkWatcher(m_nSlotId)
                        ->GetRoamingState())
        {
            return nRoamingScheme;
        }
    }

    return GET_N_CONFIG(m_nSlotId)->GetPreferredEmergencyRegistration();
}
