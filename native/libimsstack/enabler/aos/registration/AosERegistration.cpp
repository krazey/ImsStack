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
#include "ServiceTrace.h"

#include "CarrierConfig.h"
#include "SIPStatusCode.h"

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
AosERegistration::AosERegistration(IN IAosAppContext* piAppContext, IN AString& strRegId)
    : AosRegistration(piAppContext, strRegId)
    , m_bReinitiationRequested(IMS_FALSE)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosERegistration = %" PFLS_u "/%" PFLS_x, REGID,
            sizeof(AosERegistration), this);
};

PUBLIC VIRTUAL
AosERegistration::~AosERegistration()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosERegistration = %" PFLS_u "/%" PFLS_x, REGID,
            sizeof(AosERegistration), this);

    IAosCallTracker* piCt = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
    if (piCt != IMS_NULL)
    {
        piCt->RemoveListener(this);
    }
}

PUBLIC VIRTUAL
void AosERegistration::Start()
{
    A_IMS_TRACE_I(REGID, "Start :: state(%s)",
            AosProvider::GetLog()->RegStateToString(m_nState), 0, 0);

    IMS_UINT32 nScheme = GET_N_CONFIG(m_nSlotId)->GetPreferredEmergencyRegistration();

    if (IsFakeModeCondition() || (nScheme ==
            CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_SKIP))
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

PUBLIC VIRTUAL
void AosERegistration::Update(IN IMS_BOOL bIgnoreRetryTimer /* = IMS_FALSE */,
        IN IMS_BOOL bExplicitUpdate /* = IMS_TRUE */)
{
    (void) bIgnoreRetryTimer;
    (void) bExplicitUpdate;

    A_IMS_TRACE_I(REGID, "Update :: state(%s)", AosProvider::GetLog()->RegStateToString(m_nState),
            0, 0);

    switch (m_nState)
    {
        case STATE_REGSTOP: // FALL-THROUGH
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

PUBLIC VIRTUAL
void AosERegistration::RequestCmd(IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason /* = 0 */)
{
    if (nCmdType == CMD_FAKE_MODE)
    {
        A_IMS_TRACE_I(REGID, "RequestCmd :: try the fake E-REG with nReason[%d]", nReason, 0, 0);

        if (nReason == IAosRegistration::REASON_FAKE_MODE_NEXT_PCSCF)
        {
            m_piContext->GetPcscf()->RemoveCurrentPcscf();

            if (SetFirstPcscf(IMS_FALSE) == IMS_FALSE )
            {
                Destroy();
                ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
                return;
            }
        }

        ProcessFakeModeWithRegState(IsRegistered());
        return;
    }

    AosRegistration::RequestCmd(nCmdType, nReason);
}

PROTECTED VIRTUAL
IMS_BOOL AosERegistration::OnMessage(IN IMSMSG& objMsg)
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
            ProcessReinitiateWithRegState(((static_cast<IMS_SINT32>(objMsg.nWparam)) > 0) ?
                    IMS_TRUE : IMS_FALSE);
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL
void AosERegistration::Init()
{
    A_IMS_TRACE_D(REGID, "Init", 0, 0, 0);

    if (GET_N_CONFIG(m_nSlotId)->GetPreferredEmergencyRegistration() ==
            CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_SKIP)
    {
        SetFakeReg(IMS_TRUE);
        SetMode(MODE_FAKE);
    }

    IAosCallTracker* piCt = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
    if (piCt != IMS_NULL)
    {
        piCt->SetListener(this);
    }

    InitFeatures();
    InitRetryIntervals();
}

PROTECTED VIRTUAL
IMS_BOOL AosERegistration::IsFakeModeCondition()
{
    return m_piContext->GetBlock()->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED) ||
            m_piContext->GetNetTracker()->IsEmergencyLteAttach();
}

PROTECTED VIRTUAL
IMS_BOOL AosERegistration::IsReinitiationRequested() const
{
    return m_bReinitiationRequested;
}

PROTECTED VIRTUAL
void AosERegistration::ProcessAuthenticationFailed()
{
    ProcessDefaultFlowRecovery_Start();
}

PROTECTED VIRTUAL
void AosERegistration::ProcessDefaultFlowRecovery_Start(IN IMS_SINT32 nStatusCode /* = 0 */)
{
    (void) nStatusCode;

    if (IsReinitiationRequested())
    {
        A_IMS_TRACE_I(REGID,
                "ProcessDefaultFlowRecovery_Start :: ignore because reg is re-initiated", 0, 0, 0);
        return;
    }

    if (IsFakeRegistration())
    {
        A_IMS_TRACE_I(REGID, "ProcessDefaultFlowRecovery_Start :: fake E-REG is failed",
                0, 0, 0);
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

PROTECTED VIRTUAL
void AosERegistration::ProcessDefaultFlowRecovery_Update(IN IMS_SINT32 nStatusCode /* = 0 */)
{
    (void) nStatusCode;

    SetState(STATE_REFRESHSTOP);
}

PROTECTED VIRTUAL
void AosERegistration::ProcessStartFailed_StatusCode(IN IMS_SINT32 nStatusCode)
{
    A_IMS_TRACE_I(REGID, "ProcessStartFailed_StatusCode :: Code(%d) ", nStatusCode, 0, 0);

    switch (nStatusCode)
    {
         // 423
        case SIPStatusCode::SC_423:
            ProcessStartFailed_423();
            break;

        default:
            // Other 4xx, 5xx, 6xx response
            ProcessDefaultFlowRecovery_Start(nStatusCode);
            break;
    }
}

PROTECTED VIRTUAL
void AosERegistration::ProcessStartFailed_TxnTimeout()
{
    ProcessDefaultFlowRecovery_Start();
}

PROTECTED VIRTUAL
void AosERegistration::ProcessStartFailed_Others(IN IMS_SINT32 nReason)
{
    (void) nReason;

    ProcessDefaultFlowRecovery_Start();
}

PROTECTED VIRTUAL
void AosERegistration::ProcessUpdateFailed_StatusCode(IN IMS_SINT32 nStatusCode)
{
    A_IMS_TRACE_I(REGID, "ProcessUpdateFailed_StatusCode :: Code(%d) ", nStatusCode, 0, 0);

    switch (nStatusCode)
    {
        //423
        case SIPStatusCode::SC_423:
            ProcessUpdateFailed_423();
            break;

        default:
            // other 4xx, 5xx, 6xx response
            ProcessDefaultFlowRecovery_Update(nStatusCode);
            break;
    }
}

PROTECTED VIRTUAL
void AosERegistration::ProcessUpdateFailed_TxnTimeout()
{
    ProcessDefaultFlowRecovery_Update();
}

PROTECTED VIRTUAL
void AosERegistration::ProcessUpdateFailed_Others(IN IMS_SINT32 nReason)
{
    (void) nReason;

    ProcessDefaultFlowRecovery_Update();
}

PROTECTED VIRTUAL
void AosERegistration::ProcessECallStarted()
{
    if (GetState() == STATE_REFRESHSTOP)
    {
        A_IMS_TRACE_I(REGID, "ProcessECallStarted :: re-reg is trying in refresh stop", 0, 0, 0);
        ProcessRetryInRegStopped();
    }
}

PROTECTED VIRTUAL
void AosERegistration::ProcessFakeMode()
{
    A_IMS_TRACE_I(REGID, "ProcessFakeMode", 0, 0, 0);

    SetFakeReg(IMS_TRUE);
    SetMode(MODE_FAKE);
    SetReinitiationRequested(IMS_TRUE);
    PostMessage(MSG_REG_REINITIATE, 0, 0);
}

PROTECTED VIRTUAL
void AosERegistration::ProcessFakeModeWithRegState(IN IMS_BOOL bIsRegistered)
{
    A_IMS_TRACE_I(REGID, "ProcessFakeModeWithRegState", 0, 0, 0);

    SetFakeReg(IMS_TRUE);
    SetMode(MODE_FAKE);
    SetReinitiationRequested(IMS_TRUE);
    PostMessage(MSG_REG_REINITIATE_WITH_REG_STATE, bIsRegistered ? 1 : 0, 0);
}

PROTECTED VIRTUAL
void AosERegistration::ProcessReinitiateWithRegState(IN IMS_BOOL bIsRegistered)
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

PROTECTED VIRTUAL
void AosERegistration::SetReinitiationRequested(IN IMS_BOOL bRequest)
{
    A_IMS_TRACE_I(REGID, "SetReinitiationRequested :: (%s)", (bRequest) ? "ON" : "OFF", 0, 0);

    m_bReinitiationRequested = bRequest;
}

PROTECTED VIRTUAL
void AosERegistration::ProcessTransactionTimerExpired()
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

PROTECTED VIRTUAL
void AosERegistration::Registration_RefreshTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh)
{
    A_IMS_TRACE_I(REGID, "Registration_RefreshTimerExpired", 0, 0, 0);

    bDoImplicitRefresh = IMS_FALSE;

    if (m_piRegistration == IMS_NULL)
    {
        return;
    }

    if (!IsTransactionStarted())
    {
        A_IMS_TRACE_I(REGID, "Registration_RefreshTimerExpired :: transaction is not started",
                0, 0, 0);
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

PROTECTED VIRTUAL
void AosERegistration::Registration_Started()
{
    StopTimer(TIMER_TRANSACTION);
    AosRegistration::Registration_Started();
}

PROTECTED VIRTUAL
void AosERegistration::Registration_StartFailed(IN IMS_SINT32 nReason)
{
    StopTimer(TIMER_TRANSACTION);
    AosRegistration::Registration_StartFailed(nReason);
}

PROTECTED VIRTUAL
void AosERegistration::Registration_Terminated(IN IMS_SINT32 nReason)
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

PROTECTED VIRTUAL
void AosERegistration::CallTracker_StateChanged(IN IMS_UINT32 nType, IN IMS_UINT32 m_nState)
{
    if (nType != IAosCallTracker::TYPE_EMERGENCY)
    {
        return;
    }

    IMS_BOOL bCurrState = (m_nState > IAosCallTracker::STATE_IDLE) ? IMS_TRUE : IMS_FALSE;
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
