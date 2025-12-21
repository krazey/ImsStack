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
#include "INetworkWatcher.h"
#include "ISystemProperty.h"
#include "ServiceEvent.h"
#include "ServicePhoneInfo.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"

#include "IImsRadio.h"
#include "IIpcan.h"

#include "CarrierConfig.h"
#include "IRegistration.h"
#include "SipStatusCode.h"
#include "IAosService.h"
#include "ImsAosParameter.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosConnection.h"
#include "interface/IAosEmergencyListener.h"
#include "interface/IAosHandle.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosNetTracker.h"
#include "interface/IAosPcscf.h"
#include "interface/IAosSubscriber.h"
#include "provider/AosLog.h"
#include "provider/AosProvider.h"
#include "provider/AosRetryRepository.h"
#include "provider/AosString.h"
#include "registration/AosIpsecHelper.h"

#include "registration/AosERegistration.h"

__IMS_TRACE_TAG_AOS__;

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

    if (!IsNetworkReady())
    {
        if (m_piWaitEmergencyNetworkTimer == IMS_NULL)
        {
            StartTimer(TIMER_WAIT_EMERGENCY_NETWORK, EMERGENCY_NETWORK_WAIT_TIME * 1000);
            return;
        }
    }

    StopTimer(TIMER_WAIT_EMERGENCY_NETWORK);

    if (IsFakeModeCondition())
    {
        if (IsERegRequestedByOnlySms())
        {
            ProcessUnpredictableFailure();
            return;
        }
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

PUBLIC VIRTUAL void AosERegistration::Update(
        IN IMS_BOOL /*bIgnoreRetryTimer = IMS_FALSE*/, IN IMS_BOOL /*bExplicitUpdate = IMS_TRUE*/)
{
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

PUBLIC VIRTUAL void AosERegistration::NotifyEmergencySmsState(
        IN IMS_BOOL bIsInitialized, IN EmergencyServicePdn ePdnType)
{
    if (m_pEModeInfo == IMS_NULL || !GET_N_CONFIG(m_nSlotId)->IsEmergencySmsOverImsSupported())
    {
        return;
    }

    m_pEModeInfo->SetESmsPdn(ePdnType);
    m_pEModeInfo->SetESms(bIsInitialized);
}

PUBLIC VIRTUAL IMS_BOOL AosERegistration::IsInCallbackMode()
{
    if (m_pEModeInfo == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_pEModeInfo->IsEcbm() ||
            (m_pEModeInfo->IsScbm() &&
                    m_pEModeInfo->GetESmsPdn() == EmergencyServicePdn::EMERGENCY);
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

        case MSG_REG_PROCESS_GIBA:
            SetReinitiationRequested(IMS_FALSE);
            ProcessGiba();
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
            piService->AddListener(DYNAMIC_CAST(IAosServicePhoneListener*, this));
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
        piService->RemoveListener(DYNAMIC_CAST(IAosServicePhoneListener*, this));
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
    AosRegistration::DestroyRegistration();
}

PROTECTED VIRTUAL void AosERegistration::ProcessAuthenticationFailed()
{
    StopTimer(TIMER_TRANSACTION);

    ProcessDefaultFlowRecovery_Start();
}

PROTECTED VIRTUAL void AosERegistration::ProcessDefaultFlowRecovery_Start(IN IMS_SINT32 nStatusCode)
{
    if (IsERegRequestedByOnlySms())
    {
        ProcessUnpredictableFailure();
        return;
    }

    if (IsReinitiationRequested())
    {
        A_IMS_TRACE_I(
                REGID, "ProcessDefaultFlowRecovery_Start :: Ignore due to re-initiation", 0, 0, 0);
        return;
    }

    IMS_BOOL bRequiredERegRetry = GET_N_CONFIG(m_nSlotId)->IsRegRetryRuleForERegUsed() &&
            !m_piContext->GetNetTracker()->IsRoaming();
    if (bRequiredERegRetry && ProcessNormalDefaultFlowRecovery_Start(nStatusCode))
    {
        A_IMS_TRACE_I(
                REGID, "ProcessDefaultFlowRecovery_Start :: Follow normal retry flow", 0, 0, 0);
        return;
    }

    if (IsFakeRegistration())
    {
        A_IMS_TRACE_I(REGID, "ProcessDefaultFlowRecovery_Start :: Fake E-REG is failed", 0, 0, 0);
        ProcessUnpredictableFailure();
        return;
    }

    if (GetPreferredRegScheme() ==
            CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK)
    {
        if (!GET_N_CONFIG(m_nSlotId)->IsAnonymousECallActionSupported() ||
                IsAnonymousECallActionPresent(nStatusCode))
        {
            A_IMS_TRACE_I(REGID, "ProcessDefaultFlowRecovery_Start :: Try fake E-REG", 0, 0, 0);
            ProcessFakeMode();
            return;
        }
    }

    SetState(STATE_REGSTOP);
    SetTraffic(IMS_FALSE);
    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
}

PROTECTED VIRTUAL void
AosERegistration::ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(
        IN IMS_UINT32 nRetryAfter)
{
    IMS_UINT32 nAwt = nRetryAfter;
    if (nAwt == 0)
    {
        const ImsVector<IMS_SINT32>& objInterval = GET_N_CONFIG(m_nSlotId)->GetRegRetryIntervals();
        nAwt = (objInterval.GetSize() > 0) ? objInterval.GetAt(0) : RETRY_DEFAULT_WAIT_TIME;
    }

    IMS_BOOL bIsRetryNeeded = IMS_FALSE;

    if (AosProvider::GetInstance()
                    ->GetRetryRepository(m_piContext->GetSlotId())
                    ->IncreaseRetryCount(AosRetryRepository::TYPE_EMERGENCY))
    {
        bIsRetryNeeded = IMS_TRUE;
    }
    else
    {
        m_piContext->GetPcscf()->SetCurrentPcscfInvalid();
        if (SetNextPcscf())
        {
            bIsRetryNeeded = IMS_TRUE;
        }
        else
        {
            A_IMS_TRACE_I(REGID, "All P-CSCFs attempted, will start retrying", 0, 0, 0);
            m_piContext->GetPcscf()->SetAllPcscfValid();
            if (SetFirstPcscf())
            {
                bIsRetryNeeded = IMS_TRUE;
            }
        }
    }

    if (bIsRetryNeeded)
    {
        StartTimer(TIMER_STOP_RETRY, nAwt * 1000);
        SetState(STATE_REGSTOP);
    }
    else
    {
        ProcessUnpredictableFailure();
    }
}

PROTECTED VIRTUAL void AosERegistration::ProcessDefaultFlowRecovery_Update(
        IN IMS_SINT32 /* nStatusCode */ /* = 0 */)
{
    if (!IsImsCall() && m_pEModeInfo != IMS_NULL && !IsInCallbackMode())
    {
        ProcessUnpredictableFailure();
        return;
    }

    SetState(STATE_REFRESHSTOP);
    SetTraffic(IMS_FALSE);
}

PROTECTED VIRTUAL IMS_BOOL AosERegistration::ProcessStartFailed_305()
{
    IMS_SINT32 nPolicy = GET_N_CONFIG(m_nSlotId)->GetRegRetrySip305CodePolicy();
    // It's only for CarrierConfig::Ims::SIP_305_CODE_POLICY_3GPP
    if (nPolicy == CarrierConfig::Ims::SIP_305_CODE_POLICY_3GPP)
    {
        m_piContext->GetPcscf()->SetCurrentPcscfInvalid();

        if (SetNextPcscf())
        {
            SetState(STATE_REGSTOP);
            if (GET_N_CONFIG(m_piContext->GetSlotId())
                            ->IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            {
                AosProvider::GetInstance()
                        ->GetRetryRepository(m_piContext->GetSlotId())
                        ->ResetRetryCount(AosRetryRepository::TYPE_EMERGENCY);
            }

            if (SendRegister(IMS_TRUE))
            {
                SetState(STATE_REGISTERING);
            }
            else
            {
                ProcessUnpredictableFailure();
            }
        }
        else
        {
            SetTraffic(IMS_FALSE);
            ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
        }
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosERegistration::ProcessStartFailed_420()
{
    if (GET_N_CONFIG(m_nSlotId)->IsGibaSupportedForERegInRoaming() &&
            m_piContext->GetNetTracker()->IsRoaming() &&
            m_pUtil->IsParameterIncluded(m_piRegistration->GetPreviousResponse(),
                    ISipHeader::UNSUPPORTED, AosString::STR_SEC_AGREE))
    {
        SetReinitiationRequested(IMS_TRUE);
        PostMessage(MSG_REG_PROCESS_GIBA, 0, 0);
        return IMS_TRUE;
    }
    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosERegistration::ProcessStartFailed_423()
{
    if (m_pUtil->IsErrorCodeExisted(
                GET_N_CONFIG(m_nSlotId)->GetERegErrCodeNotSupportedCommonPolicy(),
                SipStatusCode::SC_423))
    {
        return IMS_FALSE;
    }
    else
    {
        return AosRegistration::ProcessStartFailed_423();
    }
}

PROTECTED VIRTUAL void AosERegistration::ProcessStartFailed_StatusCode(IN IMS_SINT32 nStatusCode)
{
    A_IMS_TRACE_I(REGID, "ProcessStartFailed_StatusCode :: Code(%d) ", nStatusCode, 0, 0);

    StopTimer(TIMER_TRANSACTION);

    IMS_BOOL bIsStandardHandled = IMS_FALSE;
    switch (nStatusCode)
    {
        case SipStatusCode::SC_420:
            bIsStandardHandled = ProcessStartFailed_420();
            break;

        case SipStatusCode::SC_423:
            bIsStandardHandled = ProcessStartFailed_423();
            break;

        default:
            break;
    }

    if (bIsStandardHandled)
    {
        return;
    }

    // Other responses not processed above
    ProcessDefaultFlowRecovery_Start(nStatusCode);
}

PROTECTED VIRTUAL void AosERegistration::ProcessStartFailed_TxnTimeout()
{
    ProcessDefaultFlowRecovery_Start();
}

PROTECTED VIRTUAL void AosERegistration::ProcessStartFailed_Others(IN IMS_SINT32 nReason)
{
    if (m_piTransactionTimer != IMS_NULL)
    {
        A_IMS_TRACE_D(REGID, "ProcessStartFailed_Others :: Reason(%d), Wait transaction timeout",
                nReason, 0, 0);
        if (nReason == IRegistration::REASON_CLIENT_SOCKET_ERROR ||
                nReason == IRegistration::REASON_SERVER_SOCKET_ERROR)
        {
            m_eImsReasonCode = AosReasonCode::REG_RESP_NETWORK_TIMEOUT;
        }
        return;
    }

    ProcessDefaultFlowRecovery_Start();
}

PROTECTED VIRTUAL void AosERegistration::ProcessUpdateFailed_StatusCode(IN IMS_SINT32 nStatusCode)
{
    A_IMS_TRACE_I(REGID, "ProcessUpdateFailed_StatusCode :: Code(%d) ", nStatusCode, 0, 0);

    IMS_BOOL bIsStandardHandled = IMS_FALSE;
    switch (nStatusCode)
    {
        // 423
        case SipStatusCode::SC_423:
            bIsStandardHandled = ProcessUpdateFailed_423();
            break;

        default:
            // other 4xx, 5xx, 6xx response
            break;
    }

    if (bIsStandardHandled)
    {
        return;
    }

    // Other responses not processed above
    ProcessDefaultFlowRecovery_Update(nStatusCode);
}

PROTECTED VIRTUAL void AosERegistration::ProcessUpdateFailed_TxnTimeout()
{
    ProcessDefaultFlowRecovery_Update();
}

PROTECTED VIRTUAL void AosERegistration::ProcessUpdateFailed_Others(IN IMS_SINT32 /*nReason*/)
{
    ProcessDefaultFlowRecovery_Update();
}

PROTECTED VIRTUAL void AosERegistration::ProcessStopRetryTimerExpired()
{
    StopTimer(TIMER_STOP_RETRY);

    if (!IsRetryHeld())
    {
        return;
    }

    if (SendRegister(GetState() != STATE_REFRESHSTOP))
    {
        SetRetryState();
    }
    else
    {
        ProcessUnpredictableFailure();
        return;
    }
}

PROTECTED VIRTUAL void AosERegistration::ProcessModeTimerExpired()
{
    StopTimer(TIMER_MODE);

    if (IsInCallbackMode())
    {
        ProcessUnpredictableFailure();
        ClearCbm();
    }
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

    if (IsRetryAllowed())
    {
        if (GET_N_CONFIG(m_nSlotId)->IsEmcRegOnRandomPcscf())
        {
            IMS_SINT32 nNumOfPcscfs = m_piContext->GetPcscf()->GetPcscfs().GetCount();

            if (nNumOfPcscfs > 1 && (m_nConsecutiveFailure % nNumOfPcscfs == 0))
            {
                ProcessRearrangePcscf();
            }
        }

        if (SetNextPcscf() && SendRegister(IMS_TRUE))
        {
            StartRegRetryTimer();
            return;
        }
    }

    if (m_eImsReasonCode == AosReasonCode::REG_RESP_NETWORK_TIMEOUT)
    {
        SetCallFailureCauseToProperty(ECALL_FAILURE_CAUSE_EREG_TIMEOUT_DUE_TO_TCP_FAILURE);
    }
    else
    {
        if (!IsERegRequestedByOnlySms() &&
                GetPreferredRegScheme() ==
                        CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK)
        {
            A_IMS_TRACE_I(REGID, "ProcessTransactionTimerExpired :: try the fake E-REG", 0, 0, 0);

            ProcessFakeMode();
            return;
        }
    }

    ProcessUnpredictableFailure();
}

PROTECTED VIRTUAL void AosERegistration::ProcessWaitEmergencyNetworkTimerExpired()
{
    Start();
    StopTimer(TIMER_WAIT_EMERGENCY_NETWORK);
}

PROTECTED VIRTUAL void AosERegistration::ProcessScscfRestoration(
        IN IMS_UINT32 /* nUnavailableTimeForCurrentPcscf */)
{
    A_IMS_TRACE_I(REGID, "ProcessScscfRestoration", 0, 0, 0);

    IAosPcscf* piPcscf = m_piContext->GetPcscf();

    if (piPcscf == IMS_NULL)
    {
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_INTERNAL);
        return;
    }

    piPcscf->SetCurrentPcscfInvalid();
    m_nConsecutiveFailure++;

    if (piPcscf->HasNextPcscf())
    {
        DestroyEx();
        SetNextPcscf(IMS_FALSE);
        Start();
    }
    else
    {
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_INTERNAL);
    }
}

PROTECTED VIRTUAL void AosERegistration::SetRefreshPolicy()
{
    m_piRegistration->SetRefreshPolicy(IRegistration::REFRESH_POLICY_RATIO, 1200, 50, 50);
}

PROTECTED
void AosERegistration::SetReregFailureReportOnIpcanChangeRequired(IN IMS_BOOL bRequired)
{
    if (!GET_N_CONFIG(m_nSlotId)->IsEmergencyReregSupportedOnIpcanChange())
    {
        return;
    }

    AosRegistration::SetReregFailureReportOnIpcanChangeRequired(bRequired);
    UpdateTransactionStarted();
}

PROTECTED VIRTUAL void AosERegistration::UpdateTransactionStarted()
{
    m_bIsTransactionStarted = IsImsCall() ||
            (GET_N_CONFIG(m_nSlotId)->IsEmergencyCallbackModeSupported() &&
                    m_pEModeInfo != IMS_NULL && IsRefreshRequiredByCbm()) ||
            IsReregFailureReportOnIpcanChangeRequired();

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
        ClearCbm();
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
            ClearCbm();
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

PROTECTED void AosERegistration::ProcessGiba()
{
    A_IMS_TRACE_I(REGID, "ProcessGiba", 0, 0, 0);
    Destroy();
    m_piContext->GetSubscriber()->CreateTemporaryPublicUserIdForGiba();
    UpdateIpsecSupported(IMS_FALSE, IPSEC_BLOCK_GIBA);

    if (!CreateRegistration())
    {
        ProcessUnpredictableFailure();
        return;
    }

    SetState(STATE_REGISTERING);
}

PROTECTED VIRTUAL void AosERegistration::ClearCbm()
{
    if (m_pEModeInfo == IMS_NULL || !GET_N_CONFIG(m_nSlotId)->IsEmergencyCallbackModeSupported())
    {
        return;
    }

    StopTimer(TIMER_MODE);

    m_pEModeInfo->SetEcbm(IMS_FALSE);
    m_pEModeInfo->SetScbm(IMS_FALSE);
    m_pEModeInfo->SetCbmBeginTime(0);
    m_pEModeInfo->SetReRegTryTime(0);
}

// TODO: This is to inform the modem that the exact emergency call failure cause is the E-IMS
// registration failure with the TCP socket problems. It directly set the property since there is no
// AOSP HAL APIs that can be used for this purpose. This implementation may be removed when the AOSP
// HAL API is available or the call failure causes are not needed in APDS's logic later.
PROTECTED void AosERegistration::SetCallFailureCauseToProperty(IN IMS_UINT32 nFailureCause)
{
    AString strName;
    strName.Sprintf("%s%d", AosString::STR_EMERGENCY_CALL_FAIL_CAUSE, m_nSlotId);

    AString strCallFailureCause;
    strCallFailureCause.SetNumber(nFailureCause);

    UtilService::GetUtilService()->GetSystemProperty()->Set(strName, strCallFailureCause);
}

PROTECTED void AosERegistration::CallbackModeChanged(
        IN EmergencyCallbackModeType eType, IN EmergencyCallbackMode eState, IN IMS_ULONG nDuration)
{
    if (m_pEModeInfo == IMS_NULL || !GET_N_CONFIG(m_nSlotId)->IsEmergencyCallbackModeSupported())
    {
        return;
    }

    A_IMS_TRACE_I(REGID, "CallbackModeChanged :: eType (%d), eState(%d), nDuration(%d)", eType,
            eState, nDuration);

    if (eState == EmergencyCallbackMode::START)
    {
        SetCallbackMode(eType, IMS_TRUE);

        m_pEModeInfo->SetCbmDuration(nDuration);
        m_pEModeInfo->SetCbmBeginTime(IMS_SYS_GetTimeInSeconds());
        StartTimer(TIMER_MODE, nDuration * 1000);

        if (GetState() != STATE_OFFLINE)
        {
            UpdateTransactionStarted();
            ProcessReRegStart();
        }
    }
    else
    {
        SetCallbackMode(eType, IMS_FALSE);
        if (eState == EmergencyCallbackMode::STOP)
        {
            if (!IsInCallbackMode())
            {
                if (m_piContext->GetConnection()->IsActivationRequested())
                {
                    ProcessUnpredictableFailure();
                }
                ClearCbm();
            }
            A_IMS_TRACE_I(REGID, "Current callback mode status:: Ecbm(%d), Scbm(%d)",
                    m_pEModeInfo->IsEcbm(), m_pEModeInfo->IsScbm(), 0);
        }
    }
}

PROTECTED void AosERegistration::ServicePhone_EmergencyRegistrationStateChanged(
        IN IMS_BOOL /* bEmergencyAttached */)
{
    if (m_piWaitEmergencyNetworkTimer != IMS_NULL)
    {
        Start();
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

    if (IsERegRequestedByOnlySms())
    {
        ProcessUnpredictableFailure();
        return;
    }

    if (nReason == IAosRegistration::REASON_FAKE_MODE_NEXT_PCSCF)
    {
        m_piContext->GetPcscf()->RemoveCurrentPcscf();

        if (SetFirstPcscf(IMS_FALSE) == IMS_FALSE)
        {
            Destroy();
            ReportStateChanged(RESULT_FAILURE,
                    GET_N_CONFIG(m_nSlotId)->IsKeepEPdnUponPcscfUnavailable()
                            ? REASON_FAILURE_NO_PCSCF_AVAILABLE
                            : REASON_FAILURE_GENERAL);
            return;
        }
    }

    ProcessFakeModeWithRegState(IsRegistered());
}

PROTECTED IMS_BOOL AosERegistration::IsRefreshRequiredByCbm()
{
    if (!IsInCallbackMode())
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
    IMS_UINT32 nScheme = GetPreferredRegScheme();
    if (nScheme == CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_SKIP)
    {
        A_IMS_TRACE_I(REGID, "IsFakeModeCondition :: EREG preferred scheme is SKIP", 0, 0, 0);
        return IMS_TRUE;
    }

    if (m_piContext->GetBlock()->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED))
    {
        A_IMS_TRACE_I(REGID, "IsFakeModeCondition :: subscriber is incompleted", 0, 0, 0);
        return IMS_TRUE;
    }

    if (!m_piContext->GetConnection()->IsEpdgEnabled() &&
            m_piContext->GetNetTracker()->IsEmergencyAttach())
    {
        A_IMS_TRACE_I(REGID, "IsFakeModeCondition :: emergency attach", 0, 0, 0);
        // Emergency SMS is supported when inserting valid UICC.
        // When receiving PCO 5, the UE will recognize self activation UICC. It's invalid UICC.
        if (IsERegRequestedByOnlySms())
        {
            return IMS_FALSE;
        }

        if (GET_N_CONFIG(m_nSlotId)->IsSupportERegWhenEAttachWithValidSim())
        {
            IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
            if (piService == IMS_NULL || piService->IsNasSecurityAlgorithmNull())
            {
                return IMS_TRUE;
            }
        }
        else
        {
            return IMS_TRUE;
        }
    }

    if (GET_N_CONFIG(m_nSlotId)->IsSupportLimitedAdminSmsMode())
    {
        AStringArray objImpu = m_piContext->GetSubscriber()->GetConfiguredImpus();
        if (objImpu.GetCount() == 1 || m_piContext->GetConnection()->IsLimitedServicePcoValue())
        {
            return IMS_TRUE;
        }
    }

    A_IMS_TRACE_I(REGID, "IsFakeModeCondition :: it's not fake condition", 0, 0, 0);

    return IMS_FALSE;
}

PROTECTED IMS_BOOL AosERegistration::IsERegRequestedByOnlySms() const
{
    return !IsImsCall() && m_pEModeInfo != IMS_NULL && !m_pEModeInfo->IsECall() &&
            m_pEModeInfo->IsESms();
}

PROTECTED IMS_BOOL AosERegistration::IsReinitiationRequested() const
{
    return m_bReinitiationRequested;
}

PROTECTED IMS_BOOL AosERegistration::IsRetryAllowed() const
{
    if (GetRegIpcanCategory() == IIpcan::CATEGORY_WLAN &&
            GET_N_CONFIG(m_nSlotId)->IsKeepERegRetryOnWlanRequired())
    {
        return IMS_TRUE;
    }

    IMS_SINT32 nMaxRetryCnt = GET_N_CONFIG(m_nSlotId)->GetEmcRegRetryMaxCnt();
    switch (nMaxRetryCnt)
    {
        case CarrierConfig::ImsEmergency::EREG_RETRY_MAX_CNT_NO_RETRY:
            return IMS_FALSE;
        case CarrierConfig::ImsEmergency::EREG_RETRY_MAX_CNT_EVERY_PCSCF_RETRY:
        {
            AStringArray objPcscfs = m_piContext->GetPcscf()->GetPcscfs();
            return (m_nConsecutiveFailure < objPcscfs.GetCount());
        }
        default:
            return m_nConsecutiveFailure <= nMaxRetryCnt;
    }
}

PROTECTED IMS_BOOL AosERegistration::IsAnonymousECallActionPresent(IN IMS_SINT32 nStatusCode) const
{
    return (nStatusCode == SipStatusCode::SC_403) &&
            m_pUtil->IsAnonymousECallActionPresent(m_piRegistration->GetPreviousResponse());
}

PROTECTED IMS_BOOL AosERegistration::IsNetworkReady() const
{
    if (m_piContext->GetConnection()->IsEpdgEnabled())
    {
        return IMS_TRUE;
    }

    return (m_piContext->GetNetTracker()->GetMobileServiceState() !=
                    INetworkWatcher::STATE_OUT_OF_SERVICE ||
            m_piContext->GetNetTracker()->IsEmergencyAttach());
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
    m_nImsRegState = IMS_REG_STATE_DEREGISTERED;
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

PROTECTED IMS_BOOL AosERegistration::ProcessNormalDefaultFlowRecovery_Start(
        IN IMS_SINT32 nStatusCode)
{
    if (nStatusCode == SipStatusCode::SC_305)
    {
        // It's only for CarrierConfig::Ims::SIP_305_CODE_POLICY_3GPP in ProcessStartFailed_305()
        if (ProcessStartFailed_305())
        {
            return IMS_TRUE;
        }
    }

    IMS_UINT32 nRetryAfter = m_pUtil->GetRetryAfterValue(m_piRegistration);

    // It's only for awt policy is CarrierConfig::Ims::AWT_POLICY_SPECIFIED_INTERVAL and
    // KEY_EXTRA_REG_ERR_RETRY_CNT_SHARED_FOR_REG_AND_SUB_BOOL is true.
    // If you need to follow the normal reg retry rule, update it below.
    IMS_SINT32 nAwtPolicy = GET_N_CONFIG(m_nSlotId)->GetRegActualWaitTimePolicy();
    if (nAwtPolicy == CarrierConfig::Ims::AWT_POLICY_SPECIFIED_INTERVAL &&
            GET_N_CONFIG(m_nSlotId)->IsExtraRegErrRetryCntSharedForRegAndSubRequired())
    {
        ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(nRetryAfter);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED void AosERegistration::SetReinitiationRequested(IN IMS_BOOL bRequest)
{
    A_IMS_TRACE_I(REGID, "SetReinitiationRequested :: (%s)", (bRequest) ? "ON" : "OFF", 0, 0);

    m_bReinitiationRequested = bRequest;
}

PROTECTED void AosERegistration::SetCallbackMode(
        IN EmergencyCallbackModeType eType, IN IMS_BOOL bEnable)
{
    if (eType == EmergencyCallbackModeType::CALL)
    {
        m_pEModeInfo->SetEcbm(bEnable);
    }
    else
    {
        m_pEModeInfo->SetScbm(bEnable);
    }
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
