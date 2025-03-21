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
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceTimer.h"
#include "ServiceSystemTime.h"
#include "ServiceUtil.h"
#include "ServiceEvent.h"
#include "ServicePhoneInfo.h"
#include "CarrierConfig.h"
#include "Engine.h"
#include "IConfiguration.h"
#include "IImsRadio.h"
#include "IRegInfoContact.h"
#include "IRegSubscription.h"
#include "IRegistration.h"
#include "SipConfigProxy.h"
#include "SipStatusCode.h"
#include "ISipHeader.h"
#include "IAosService.h"
#include "interface/IAosSubscriptionListener.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosNetTracker.h"
#include "interface/IAosConnection.h"
#include "interface/IAosSubscriber.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"
#include "provider/AosRetryRepository.h"
#include "registration/AosRegistration.h"
#include "provider/AosUtil.h"
#include "registration/AosSubscription.h"

__IMS_TRACE_TAG_AOS__;

#define AOSTAG m_strTag.GetStr()

PUBLIC
AosSubscription::AosSubscription(IN IAosAppContext* piContext,
        IN IRegSubscription* piRegSubscription, IN const AString& strAor,
        IN const SipAddress& objContactAddress) :
        m_piRegSubscription(piRegSubscription),
        m_piContext(piContext),
        m_piRetryTimer(IMS_NULL),
        m_nThrottlingCount(0),
        m_objContactAddress(objContactAddress),
        m_strAor(strAor),
        m_nAorState(IRegInfoContact::STATE_TERMINATED),
        m_nRetryCountSubTerminated(0),
        m_nRetryCountRegRequired(0),
        m_piListener(IMS_NULL),
        m_nState(STATE_OFFLINE),
        m_bIsTerminated(IMS_FALSE),
        m_bIsErrChecked(IMS_FALSE),
        m_bIsRadioWaiting(IMS_FALSE),
        m_bIsTrafficPriorityBlocked(IMS_FALSE)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : AosSubscription = %" PFLS_u "/%" PFLS_x,
            sizeof(AosSubscription), this, 0);

    m_strTag.Sprintf("%d", m_piContext->GetSlotId());
}

PUBLIC VIRTUAL AosSubscription::~AosSubscription()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : AosSubscription = %" PFLS_u "/%" PFLS_x,
            sizeof(AosSubscription), this, 0);

    if (m_piRegSubscription != IMS_NULL)
    {
        m_piRegSubscription->DestroyEx();
        m_piRegSubscription = IMS_NULL;
    }

    StopTimer();

    m_nState = STATE_OFFLINE;
}

PUBLIC VIRTUAL void AosSubscription::Initialize()
{
    A_IMS_TRACE_D(AOSTAG, "Initialize", 0, 0, 0);

    SetRefreshPolicy();
    m_piRegSubscription->SetListener(this);

    IAosTransaction* piTransaction =
            AosProvider::GetInstance()->GetTransaction(m_piContext->GetSlotId());
    if (piTransaction != IMS_NULL)
    {
        piTransaction->SetListener(IAosTransaction::TYPE_SUB, this);
    }
}

PUBLIC VIRTUAL IMS_BOOL AosSubscription::Start(IN IMS_BOOL bIsRadioCheckRequired)
{
    A_IMS_TRACE_I(AOSTAG, "Start :: state(%s)", StateToString(m_nState), 0, 0);

    IMS_SINT32 nNextState = STATE_SUBSCRIBING;

    StopTimer();

    if (!m_piListener->Subscription_CanBeTransmitted())
    {
        A_IMS_TRACE_D(AOSTAG, "sending trx is not possible", 0, 0, 0);
        return IMS_FALSE;
    }

    if (bIsRadioCheckRequired && !CheckRadioReadyAndSetRadioWaiting())
    {
        A_IMS_TRACE_I(AOSTAG, "Start :: txn is pending due to radio", 0, 0, 0);
        return IMS_FALSE;
    }

    switch (m_nState)
    {
        case STATE_OFFLINE:  // FALL-THROUGH
        case STATE_SUBSTOP:
            break;
        case STATE_SUBSCRIBED:  // FALL-THROUGH
        case STATE_SUBREFRESHSTOP:
            nNextState = STATE_SUBREFRESHING;
            break;

        default:
            A_IMS_TRACE_D(AOSTAG, "state is invalid", 0, 0, 0);
            return IMS_FALSE;
    }

    if (!SendSubscribe())
    {
        A_IMS_TRACE_I(AOSTAG, "subscribe is failed", 0, 0, 0);
        return IMS_FALSE;
    }

    SetState(nNextState);

    return IMS_TRUE;
}

PUBLIC VIRTUAL void AosSubscription::Stop()
{
    if (!GET_N_CONFIG(m_piContext->GetSlotId())->IsUnSubscription())
    {
        return;
    }

    A_IMS_TRACE_I(AOSTAG, "Stop :: state(%s)", StateToString(m_nState), 0, 0);

    m_piRegSubscription->Unsubscribe();
    SetState(STATE_UNSUBSCRIBING);
}

PUBLIC VIRTUAL void AosSubscription::Destroy()
{
    A_IMS_TRACE_I(AOSTAG, "Destroy :: state(%s)", StateToString(m_nState), 0, 0);

    IAosTransaction* piTransaction =
            AosProvider::GetInstance()->GetTransaction(m_piContext->GetSlotId());

    if (piTransaction != IMS_NULL)
    {
        piTransaction->RemoveListener(IAosTransaction::TYPE_SUB, this);
    }

    delete this;
}

PUBLIC VIRTUAL void AosSubscription::SetListener(IN IAosSubscriptionListener* piListener)
{
    this->m_piListener = piListener;
}

PUBLIC VIRTUAL void AosSubscription::SetRetryTimer(IN IMS_BOOL bCheckRetryAfter)
{
    if (bCheckRetryAfter == IMS_TRUE)
    {
        IMS_SINT32 nRetryAfter = GetRetryAfter();
        if (nRetryAfter > 0)
        {
            StartTimer(static_cast<IMS_UINT32>(nRetryAfter * 1000));
            return;
        }
    }

    IMS_BOOL bUseRegsitrationRetryIntervals =
            GET_N_CONFIG(m_piContext->GetSlotId())->IsRegRetryIntervalsUsedForSub();

    ImsVector<IMS_SINT32>& objRetryIntervals =
            GET_N_CONFIG(m_piContext->GetSlotId())->GetRegRetryIntervals();

    if (bUseRegsitrationRetryIntervals == IMS_TRUE && objRetryIntervals.GetSize() > 0)
    {
        StartTimer(static_cast<IMS_UINT32>(GetNextThrottlingTime(objRetryIntervals)));
    }
    else
    {
        StartTimer(static_cast<IMS_UINT32>(GetNextThrottlingTime(ImsVector<IMS_SINT32>())));
    }
}

PUBLIC VIRTUAL IMS_UINT32 AosSubscription::GetState()
{
    A_IMS_TRACE_I(AOSTAG, "GetState :: state(%s)", StateToString(m_nState), 0, 0);
    return m_nState;
}

PROTECTED
void AosSubscription::ClearThrottlingCount()
{
    m_nThrottlingCount = 0;
}

PROTECTED
IMS_BOOL AosSubscription::IsSubTrying() const
{
    return (m_nState == STATE_SUBSCRIBING || m_nState == STATE_SUBREFRESHING);
}

PROTECTED
IMS_BOOL AosSubscription::IsTerminated() const
{
    return m_bIsTerminated;
}

PROTECTED
void AosSubscription::ReportState(IN IMS_SINT32 nReason, IN IMS_SINT32 nCommand,
        IN IMS_BOOL bAwt /* = IMS_FALSE */, IN IMS_SINT32 nRetryAfter /* = 0 */)
{
    if (nReason == REASON_SUB_TERMINATED)
    {
        if (IsTerminated())
        {
            A_IMS_TRACE_I(AOSTAG, "ReportState :: already terminated", 0, 0, 0);
            return;
        }

        SetTerminated(IMS_TRUE);
    }

    m_piListener->Subscription_StateChanged(m_nState, nReason);

    if (nCommand != CMD_NONE)
    {
        m_piListener->Subscription_Request(nCommand, nRetryAfter, bAwt);
    }
}

PROTECTED
void AosSubscription::ReportNotifyEvent(IN IMS_SINT32 nEvent, IN IMS_SINT32 nRetryAfter /* = 0 */)
{
    m_piListener->Subscription_NotifyReceived(nEvent);

    IMS_UINT32 nFeature = 0;
    switch (nEvent)
    {
        case EVENT_EXPIRED:
            nFeature = IAosNConfiguration::NOTIFY_TERMINATED_EXPIRED;
            break;
        case EVENT_DEACTIVATED:
            nFeature = IAosNConfiguration::NOTIFY_TERMINATED_DEACTIVATED;
            break;
        case EVENT_PROBATION:
            nFeature = IAosNConfiguration::NOTIFY_TERMINATED_PROBATION;
            break;
        case EVENT_UNREGISTERED:
            nFeature = IAosNConfiguration::NOTIFY_TERMINATED_UNREGISTERED;
            break;
        case EVENT_REJECTED:
            nFeature = IAosNConfiguration::NOTIFY_TERMINATED_REJECTED;
            break;
        default:
            break;
    }

    if (nFeature == 0)
    {
        return;
    }

    IMS_BOOL bRegRequired = IMS_TRUE;

    if (IsRegAfterWaitRequiredByNotify(nFeature))
    {
        nRetryAfter = GET_N_CONFIG(m_piContext->GetSlotId())->GetNotifyWaitTime();
    }
    else
    {
        bRegRequired = IsRegRequiredByNotify(nFeature);
        nRetryAfter = (nRetryAfter > 0) ? nRetryAfter : 0;
    }

    if (!bRegRequired)
    {
        m_piListener->Subscription_Request(CMD_REG_TERMINATED);
        return;
    }

    if (IsWfcErrorMessageSupportedWithStateChecked(
                CarrierConfig::ImsWfc::WFC_ERROR_NOTIFY_TERMINATED))
    {
        m_piListener->Subscription_Request(CMD_REG_REQUIRED_WITH_NOTIFY_TERMINATED_MSG);
    }
    else
    {
        m_piListener->Subscription_Request(CMD_REG_REQUIRED, nRetryAfter);
    }
}

PROTECTED
void AosSubscription::SetState(IN IMS_UINT32 nState)
{
    if (m_nState == nState)
    {
        return;
    }

    A_IMS_TRACE_I(
            AOSTAG, "SetState :: (%s) to (%s)", StateToString(m_nState), StateToString(nState), 0);

    m_nState = nState;

    IAosTransaction* piTransaction =
            AosProvider::GetInstance()->GetTransaction(m_piContext->GetSlotId());

    if (piTransaction != IMS_NULL && !IsSubTrying())
    {
        piTransaction->StopTraffic(IAosTransaction::TYPE_SUB);
    }
}

PROTECTED
void AosSubscription::SetTerminated(IN IMS_BOOL bTerminated)
{
    A_IMS_TRACE_I(AOSTAG, "SetTerminated :: (%s)", _TRACE_B_(bTerminated), 0, 0);
    m_bIsTerminated = bTerminated;
}

PROTECTED
void AosSubscription::StartTimer(IN IMS_UINT32 nDuration)
{
    if (m_piRetryTimer != IMS_NULL)
    {
        A_IMS_TRACE_I(AOSTAG, "timer is exist", 0, 0, 0);
        StopTimer();
    }

    m_piRetryTimer = AosUtil::GetInstance()->StartTimer(nDuration, this, "SUB_RETRY_TIMER");
}

PROTECTED
void AosSubscription::StopTimer()
{
    if (m_piRetryTimer == IMS_NULL)
    {
        return;
    }

    AosUtil::GetInstance()->StopTimer(m_piRetryTimer, "SUB_RETRY_TIMER");
}

PROTECTED
IMS_BOOL AosSubscription::CheckRadioReadyAndSetRadioWaiting()
{
    IAosTransaction* m_piTransaction =
            AosProvider::GetInstance()->GetTransaction(m_piContext->GetSlotId());

    if (m_piTransaction == IMS_NULL)
    {
        return IMS_TRUE;
    }

    if (!m_piTransaction->IsTransactionAllowed(IAosTransaction::TYPE_SUB))
    {
        A_IMS_TRACE_I(AOSTAG, "CheckRadioReadyAndSetRadioWaiting :: trx is not allowed", 0, 0, 0);
        SetTrafficPriorityBlocked(IMS_TRUE);
        return IMS_FALSE;
    }
    else
    {
        if (m_piTransaction->StartTraffic(
                    IAosTransaction::TYPE_SUB, m_piContext->GetNetTracker()->GetNetworkType()))
        {
            return IMS_TRUE;
        }

        SetRadioWaiting(IMS_TRUE);
        return IMS_FALSE;
    }
}

PROTECTED
IMS_BOOL AosSubscription::IsRadioWaiting() const
{
    return m_bIsRadioWaiting;
}

PROTECTED
IMS_BOOL AosSubscription::IsTrafficPriorityBlocked() const
{
    return m_bIsTrafficPriorityBlocked;
}

PROTECTED
void AosSubscription::SetRadioWaiting(IN IMS_BOOL bWaiting)
{
    m_bIsRadioWaiting = bWaiting;
}

PROTECTED
void AosSubscription::SetTrafficPriorityBlocked(IN IMS_BOOL bBlocked)
{
    m_bIsTrafficPriorityBlocked = bBlocked;
}

PROTECTED
void AosSubscription::PrintRegInfo(IN ImsList<IRegInfoContact*>& objRegInfo)
{
    AString strLog;
    for (IMS_UINT32 i = 0; i < objRegInfo.GetSize(); i++)
    {
        const IRegInfoContact* piCurr = objRegInfo.GetAt(i);

        strLog.Append("[state/");
        strLog.Append(RegInfoStateToString(piCurr->GetState()));
        strLog.Append("]");
        strLog.Append("[event/");
        strLog.Append(RegInfoEventToString(piCurr->GetEvent()));
        strLog.Append("]");
        strLog.Append("[uri/");
        strLog.Append(piCurr->GetUri().GetUri().GetStr());
        strLog.Append("]");

        if (piCurr->GetQValue().GetLength() > 0)
        {
            strLog.Append("[qvalue/");
            strLog.Append(piCurr->GetQValue().GetStr());
            strLog.Append("]");
        }

        AString strNumber;
        strLog.Append("[retry-after/");
        strLog.Append(strNumber.SetNumber(piCurr->GetRetryAfterValue()));
        strLog.Append("]");
        strLog.Append("[expires/");
        strLog.Append(strNumber.SetNumber(piCurr->GetExpiresValue()));
        strLog.Append("]\n");
    }

    if (strLog.GetLength() > 0)
    {
        // Remove line-feed (LF)
        strLog.Chop(1);
    }

    A_IMS_TRACE_D(AOSTAG, "PrintRegInfo :: %s", strLog.GetStr(), 0, 0);
}

PROTECTED VIRTUAL IMS_BOOL AosSubscription::SendSubscribe()
{
    if (m_piRegSubscription == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_piRegSubscription->Subscribe() == IMS_FAILURE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosSubscription::ProcessFailureResponse_423(IN IMS_BOOL bIsRefreshed)
{
    IMS_SINT32 nMinTime =
            AosUtil::GetInstance()->GetMinExpiresValue(m_piRegSubscription->GetPreviousResponse());

    if (nMinTime <= 0)
    {
        return IMS_FALSE;
    }

    m_piRegSubscription->SetExpires(nMinTime);

    if (!SendSubscribe())
    {
        return IMS_FALSE;
    }

    SetState((bIsRefreshed) ? STATE_SUBREFRESHING : STATE_SUBSCRIBING);

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosSubscription::ProcessFailureResponse_503(IN IMS_BOOL bIsRefreshed)
{
    if (GET_N_CONFIG(m_piContext->GetSlotId())->GetSubRetrySip503CodePolicy() !=
            CarrierConfig::Ims::SIP_503_CODE_POLICY_3GPP)
    {
        return IMS_FALSE;
    }

    // This is in accordance with the 3GPP TS24.229 5.1.2A.1.6 specification.
    IMS_SINT32 nRetryAfter =
            AosUtil::GetInstance()->GetRetryAfterValue(m_piRegSubscription->GetPreviousResponse());

    IMS_SINT32 nTimerF = SipConfigProxy::GetTimerValueF(m_piContext->GetSlotId(), IMS_NULL,
            Engine::GetConfiguration()->GetSipConfig(m_piContext->GetSlotId())->GetSipConfigV(),
            IMS_TRUE);

    A_IMS_TRACE_I(
            AOSTAG, "ProcessFailureResponse_503 :: TF (%d), RA (%d)", nTimerF, nRetryAfter, 0);

    if (nRetryAfter > 0 && (nRetryAfter * 1000) <= nTimerF)
    {
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(AOSTAG, "request initial registration with scscf restoration", 0, 0, 0);
    SetRequestCommand(bIsRefreshed, CMD_REG_REQUIRED_WITH_SCSCF_RESTORATION,
            (nRetryAfter < 0) ? 0 : nRetryAfter);
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosSubscription::ProcessFailureResponse_504(IN IMS_BOOL bIsRefreshed)
{
    A_IMS_TRACE_I(AOSTAG, "ProcessFailureResponse_504", 0, 0, 0);

    const ISipMessage* piMsg = m_piRegSubscription->GetPreviousResponse();

    if (piMsg == IMS_NULL)
        return IMS_FALSE;

    if (AosUtil::GetInstance()->IsInitialRegistrationRequired(piMsg))
    {
        A_IMS_TRACE_I(AOSTAG, "Request initial registration", 0, 0, 0);
        SetRequestCommand(bIsRefreshed, CMD_REG_REQUIRED);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosSubscription::IsRetryActionDueToRetryCounter(IN IMS_BOOL bIsRefreshed)
{
    IMS_BOOL bSupported = GET_N_CONFIG(m_piContext->GetSlotId())
                                  ->IsExtraRegErrRetryCntSharedForRegAndSubRequired();
    IMS_SINT32 nMaxCount = GET_N_CONFIG(m_piContext->GetSlotId())->GetExtraRegErrMaxCount();
    if ((bSupported == IMS_TRUE) && (nMaxCount > 0))
    {
        IMS_BOOL bIncreseRetryCount = AosProvider::GetInstance()
                                              ->GetRetryRepository(m_piContext->GetSlotId())
                                              ->IncreaseRetryCount(AosRetryRepository::TYPE_NORMAL);
        if (bIncreseRetryCount == IMS_FALSE)
        {
            m_bIsErrChecked = IMS_TRUE;
            A_IMS_TRACE_I(AOSTAG, "request initial registration with next pcscf", 0, 0, 0);
            SetRequestCommand(bIsRefreshed, CMD_REG_REQUIRED_WITH_NEXT_PCSCF);
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosSubscription::IsSubscriptionTerminated(IN IMS_SINT32 nStatusCode)
{
    IMS_SINT32 nRetryInfoSubTerminated =
            GET_N_CONFIG(m_piContext->GetSlotId())->GetRetryCountSubErrorSubTerminated();

    if (nRetryInfoSubTerminated > 0)
    {
        ImsVector<IMS_SINT32>& objErrSubTerminated =
                GET_N_CONFIG(m_piContext->GetSlotId())->GetSubErrorSubTerminated();

        IMS_UINT32 nSize = objErrSubTerminated.GetSize();
        for (IMS_UINT32 i = 0; i < nSize; i++)
        {
            if ((nStatusCode == objErrSubTerminated.GetAt(i)) ||
                    ((nStatusCode / 100) == objErrSubTerminated.GetAt(i)))
            {
                m_bIsErrChecked = IMS_TRUE;
                m_nRetryCountSubTerminated++;
                A_IMS_TRACE_I(AOSTAG, "Subscription terminated count: %d",
                        m_nRetryCountSubTerminated, 0, 0);
                if (m_nRetryCountSubTerminated < nRetryInfoSubTerminated)
                {
                    return IMS_FALSE;
                }
                else
                {
                    m_nRetryCountSubTerminated = 0;
                    A_IMS_TRACE_I(AOSTAG, "Request terminating its subscription", 0, 0, 0);
                    RequestCommand(REASON_SUB_TERMINATED, CMD_SUB_TERMINATED);
                    return IMS_TRUE;
                }
            }
        }
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosSubscription::IsInitialRegistrationRequired(
        IN IMS_SINT32 nStatusCode, IN IMS_BOOL bIsRefreshed)
{
    IMS_SINT32 nRetryInfoRegRequired =
            GET_N_CONFIG(m_piContext->GetSlotId())->GetRetryCountSubErrorRegRequired();
    if (nRetryInfoRegRequired > 0)
    {
        ImsVector<IMS_SINT32>& objErrRegRequired =
                GET_N_CONFIG(m_piContext->GetSlotId())->GetSubErrorRegRequired();
        IMS_UINT32 nSize = objErrRegRequired.GetSize();
        for (IMS_UINT32 i = 0; i < nSize; i++)
        {
            if ((nStatusCode == objErrRegRequired.GetAt(i)) ||
                    ((nStatusCode / 100) == objErrRegRequired.GetAt(i)))
            {
                m_bIsErrChecked = IMS_TRUE;
                m_nRetryCountRegRequired++;
                A_IMS_TRACE_I(
                        AOSTAG, "Registration required count: %d", m_nRetryCountRegRequired, 0, 0);
                if (m_nRetryCountRegRequired < nRetryInfoRegRequired)
                {
                    return IMS_FALSE;
                }
                else
                {
                    m_nRetryCountRegRequired = 0;
                    A_IMS_TRACE_I(AOSTAG, "Request initial registration", 0, 0, 0);
                    SetRequestCommand(bIsRefreshed, CMD_REG_REQUIRED);
                    return IMS_TRUE;
                }
            }
        }
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosSubscription::IsInitialRegistrationWithNextPcscfRequired(
        IN IMS_SINT32 nStatusCode, IN IMS_BOOL bIsRefreshed)
{
    ImsVector<IMS_SINT32>& objErrRegRequiredWithNextPcscf =
            GET_N_CONFIG(m_piContext->GetSlotId())->GetSubErrorRegRequiredWithNextPcscf();
    IMS_UINT32 nSize = objErrRegRequiredWithNextPcscf.GetSize();
    if (nSize > 0)
    {
        for (IMS_UINT32 i = 0; i < nSize; i++)
        {
            if ((nStatusCode == objErrRegRequiredWithNextPcscf.GetAt(i)) ||
                    ((nStatusCode / 100) == objErrRegRequiredWithNextPcscf.GetAt(i)))
            {
                m_bIsErrChecked = IMS_TRUE;
                A_IMS_TRACE_I(AOSTAG, "request initial registration with next pcscf", 0, 0, 0);
                SetRequestCommand(bIsRefreshed, CMD_REG_REQUIRED_WITH_NEXT_PCSCF);
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosSubscription::IsInitialRegistrationRequiredInWifi(
        IN IMS_SINT32 nStatusCode, IN IMS_BOOL bIsRefreshed)
{
    if (m_piContext->GetConnection()->IsEpdgEnabled() == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    ImsVector<IMS_SINT32>& objErrRegRequiredInWifi =
            GET_N_CONFIG(m_piContext->GetSlotId())->GetVowifiSubErrorRegRequired();
    IMS_UINT32 nSize = objErrRegRequiredInWifi.GetSize();
    if (nSize > 0)
    {
        for (IMS_UINT32 i = 0; i < nSize; i++)
        {
            if ((nStatusCode == objErrRegRequiredInWifi.GetAt(i)) ||
                    ((nStatusCode / 100) == objErrRegRequiredInWifi.GetAt(i)))
            {
                m_bIsErrChecked = IMS_TRUE;
                A_IMS_TRACE_I(AOSTAG, "Request initial registration", 0, 0, 0);

                if (IsWfcErrorMessageSupportedWithStateChecked(
                            CarrierConfig::ImsWfc::WFC_ERROR_SUB_403))
                {
                    SetRequestCommand(bIsRefreshed, CMD_REG_REQUIRED_WITH_SUB_403_MSG);
                }
                else
                {
                    SetRequestCommand(bIsRefreshed, CMD_REG_REQUIRED);
                }

                return IMS_TRUE;
            }
        }
    }
    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosSubscription::IsResubscriptionStopped(IN IMS_SINT32 nStatusCode)
{
    ImsVector<IMS_SINT32>& objErrResubStopped =
            GET_N_CONFIG(m_piContext->GetSlotId())->GetSubErrorStoppingResub();
    IMS_UINT32 nSize = objErrResubStopped.GetSize();
    if (nSize > 0)
    {
        for (IMS_UINT32 i = 0; i < nSize; i++)
        {
            if ((nStatusCode == objErrResubStopped.GetAt(i)) ||
                    ((nStatusCode / 100) == objErrResubStopped.GetAt(i)))
            {
                m_bIsErrChecked = IMS_TRUE;
                A_IMS_TRACE_I(AOSTAG, "nothing to do until expiration time", 0, 0, 0);
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosSubscription::IsRegRequiredByNotify(IN IMS_UINT32 nFeature)
{
    return AosUtil::GetInstance()->IsFeatureOn(nFeature,
            GET_N_CONFIG(m_piContext->GetSlotId())->GetNotifyEventForInitialRegistration());
}

PROTECTED VIRTUAL IMS_BOOL AosSubscription::IsRegAfterWaitRequiredByNotify(IN IMS_UINT32 nFeature)
{
    return AosUtil::GetInstance()->IsFeatureOn(nFeature,
            GET_N_CONFIG(m_piContext->GetSlotId())->GetNotifyEventForInitialRegWithWaitTime());
}

PROTECTED VIRTUAL IMS_BOOL AosSubscription::IsWfcErrorMessageSupportedWithStateChecked(
        IN IMS_SINT32 nError)
{
    if (m_piContext->GetConnection()->IsEpdgEnabled() == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    if (GetState() == STATE_UNSUBSCRIBING || GetState() == STATE_OFFLINE)
    {
        return IMS_FALSE;
    }

    return GET_N_CONFIG(m_piContext->GetSlotId())->IsWfcErrorMessageSupported(nError);
}

PROTECTED VIRTUAL IMS_BOOL AosSubscription::ProcessFailed_StatusCode(
        IN IMS_SINT32 nStatusCode, IN IMS_BOOL bIsRefreshed)
{
    m_bIsErrChecked = IMS_FALSE;

    if ((IsRetryActionDueToRetryCounter(bIsRefreshed) == IMS_TRUE) ||
            ((!m_bIsErrChecked) && (IsSubscriptionTerminated(nStatusCode) == IMS_TRUE)) ||
            ((!m_bIsErrChecked) &&
                    (IsInitialRegistrationRequired(nStatusCode, bIsRefreshed) == IMS_TRUE)) ||
            ((!m_bIsErrChecked) &&
                    (IsInitialRegistrationWithNextPcscfRequired(nStatusCode, bIsRefreshed) ==
                            IMS_TRUE)) ||
            ((!m_bIsErrChecked) &&
                    (IsInitialRegistrationRequiredInWifi(nStatusCode, bIsRefreshed) == IMS_TRUE)))

    {
        return IMS_TRUE;
    }
    else if ((!m_bIsErrChecked) && (nStatusCode == SipStatusCode::SC_423))
    {
        if (ProcessFailureResponse_423(bIsRefreshed))
        {
            return IMS_TRUE;
        }
    }
    else if ((!m_bIsErrChecked) && (nStatusCode == SipStatusCode::SC_503))
    {
        if (ProcessFailureResponse_503(bIsRefreshed))
        {
            return IMS_TRUE;
        }
    }
    else if ((!m_bIsErrChecked) && (nStatusCode == SipStatusCode::SC_504))
    {
        if (ProcessFailureResponse_504(bIsRefreshed))
        {
            return IMS_TRUE;
        }
    }
    else if (bIsRefreshed == IMS_TRUE)
    {
        if ((!m_bIsErrChecked) && (nStatusCode == SipStatusCode::SC_481))
        {
            RequestCommand(REASON_SUB_TERMINATED, CMD_SUB_REQUIRED);
            return IMS_TRUE;
        }
        else if ((!m_bIsErrChecked) && (IsResubscriptionStopped(nStatusCode) == IMS_TRUE))
        {
            return IMS_TRUE;
        }
    }
    m_bIsErrChecked = IMS_FALSE;
    return IMS_FALSE;
}

PROTECTED VIRTUAL void AosSubscription::SetRequestCommand(
        IN IMS_BOOL bIsRefreshed, IN IMS_SINT32 nCommand, IN IMS_SINT32 nRetryAfter /* = 0 */)
{
    if (bIsRefreshed)
    {
        RequestCommand(REASON_SUB_TERMINATED, nCommand, nRetryAfter);
    }
    else
    {
        RequestCommand(REASON_SUB_FAILED, nCommand, nRetryAfter);
    }
}

PROTECTED VIRTUAL void AosSubscription::RequestCommand(
        IN IMS_SINT32 nReason, IN IMS_SINT32 nCommand, IN IMS_SINT32 nRetryAfter /* = 0 */)
{
    A_IMS_TRACE_I(AOSTAG, "RequestCommand:: reason(%d), command(%d) ", nReason, nCommand, 0);
    SetState(STATE_OFFLINE);

    IMS_BOOL bIsRegRequired =
            (nCommand == CMD_REG_REQUIRED || nCommand == CMD_REG_REQUIRED_WITH_SUB_403_MSG ||
                    nCommand == CMD_REG_REQUIRED_WITH_NOTIFY_TERMINATED_MSG);

    ReportState(nReason, nCommand,
            bIsRegRequired &&
                    GET_N_CONFIG(m_piContext->GetSlotId())->GetRegRetryCountResetPolicy() ==
                            CarrierConfig::Ims::REG_RETRY_CNT_RESET_POLICY_SUBSCRIPTION,
            nRetryAfter);
}

PROTECTED VIRTUAL void AosSubscription::ProcessStartFailed_StatusCode(IN IMS_SINT32 nStatusCode)
{
    A_IMS_TRACE_I(AOSTAG, "ProcessStartFailed_StatusCode :: status code (%d)", nStatusCode, 0, 0);

    IMS_BOOL bDone = ProcessFailed_StatusCode(nStatusCode, IMS_FALSE);

    if (bDone == IMS_TRUE)
    {
        return;
    }

    SetState(STATE_SUBSTOP);
    SetRetryTimer((nStatusCode != 0) ? IMS_TRUE : IMS_FALSE);
}

PROTECTED VIRTUAL void AosSubscription::ProcessStartFailed_Others(IN IMS_SINT32 nReason)
{
    A_IMS_TRACE_I(AOSTAG,
            "ProcessStartFailed_Others :: reason (%d) , "
            "process after terminated event",
            nReason, 0, 0);

    SetState(STATE_SUBSTOP);
    SetRetryTimer(IMS_FALSE);
}

PROTECTED VIRTUAL void AosSubscription::ProcessUpdateFailed_StatusCode(IN IMS_SINT32 nStatusCode)
{
    A_IMS_TRACE_I(AOSTAG, "ProcessUpdateFailed_StatusCode :: status code (%d)", nStatusCode, 0, 0);

    IMS_BOOL bDone = ProcessFailed_StatusCode(nStatusCode, IMS_TRUE);

    if (bDone == IMS_TRUE)
    {
        return;
    }

    SetState(STATE_SUBREFRESHSTOP);
    SetRetryTimer((nStatusCode != 0) ? IMS_TRUE : IMS_FALSE);
}

PROTECTED VIRTUAL void AosSubscription::ProcessUpdateFailed_Others(IN IMS_SINT32 nReason)
{
    A_IMS_TRACE_I(AOSTAG,
            "ProcessUpdateFailed_Others :: reason (%d) , "
            "process after terminated event",
            nReason, 0, 0);

    SetState(STATE_SUBREFRESHSTOP);
    SetRetryTimer(IMS_FALSE);
}

PROTECTED VIRTUAL IMS_SINT32 AosSubscription::GetRetryAfter()
{
    IMS_SINT32 nRetryAfter =
            AosUtil::GetInstance()->GetRetryAfterValue(m_piRegSubscription->GetPreviousResponse());

    if (nRetryAfter <= 0)
    {
        return 0;
    }

    m_nThrottlingCount++;

    return nRetryAfter;
}

PROTECTED VIRTUAL IMS_SINT32 AosSubscription::GetNextThrottlingTime(
        IN const ImsVector<IMS_SINT32>& objInterval)
{
    IMS_UINT32 nMaxCount = objInterval.GetSize();

    m_nThrottlingCount++;
    if (nMaxCount == 0)
    {
        IMS_UINT32 nRetryBaseTime =
                GET_N_CONFIG(m_piContext->GetSlotId())->GetRegistrationRetryBaseTime() / 1000;
        IMS_UINT32 nRetryMaxTime =
                GET_N_CONFIG(m_piContext->GetSlotId())->GetRegistrationRetryMaxTime() / 1000;
        return AosUtil::GetInstance()->WaitTimeForFlowRecovery(
                       nRetryBaseTime, nRetryMaxTime, m_nThrottlingCount) *
                1000;
    }

    IMS_UINT32 nTimeIndex = 0;

    if (m_nThrottlingCount > nMaxCount)
    {
        nTimeIndex = nMaxCount;
    }
    else
    {
        nTimeIndex = m_nThrottlingCount;
    }

    ImsVector<IMS_SINT32>& objRetryRandomIntervals =
            GET_N_CONFIG(m_piContext->GetSlotId())->GetRegRandomRetryIntervals();

    IMS_SINT32 nThrotllingTime = objInterval.GetAt(nTimeIndex - 1) * 1000;

    if (objRetryRandomIntervals.GetSize() == nMaxCount &&
            objRetryRandomIntervals.GetAt(nTimeIndex - 1) > 0)
    {
        nThrotllingTime +=
                (IMS_SYS_GetRandom(objRetryRandomIntervals.GetAt(nTimeIndex - 1) + 1) * 1000);
    }

    A_IMS_TRACE_I(AOSTAG, "GetNextThrottlingTime :: throttling count (%d) , time (%d)",
            m_nThrottlingCount, nThrotllingTime, 0);

    return nThrotllingTime;
}

PROTECTED VIRTUAL void AosSubscription::ProcessTimerExpired()
{
    A_IMS_TRACE_I(AOSTAG, "ProcessTimerExpired :: state(%s)", StateToString(m_nState), 0, 0);

    if ((m_nState != STATE_OFFLINE) && (m_nState != STATE_SUBSTOP) &&
            (m_nState != STATE_SUBREFRESHSTOP))
    {
        A_IMS_TRACE_I(AOSTAG, "sub state is invalid", 0, 0, 0);
        return;
    }

    Start(IMS_TRUE);
}

PROTECTED VIRTUAL void AosSubscription::SetRefreshPolicy()
{
    m_piRegSubscription->SetRefreshPolicy(IRegistration::REFRESH_POLICY_SPEC,
            REFRESH_POLICY_CRITERIA_INTERVAL_FOR_RETRY,
            REFRESH_POLICY_RATIO_VALUE_BELOW_THE_CRITERIA,
            REFRESH_POLICY_INTERVAL_VALUE_ABOVE_THE_CRITERIA);
}

PROTECTED VIRTUAL IRegInfoContact* AosSubscription::GetRegInfoContact(
        IN const ImsList<IRegInfoContact*>& objContact)
{
    IRegInfoContact* piRegInfoContact = IMS_NULL;

    for (IMS_UINT32 i = 0; i < objContact.GetSize(); i++)
    {
        piRegInfoContact = objContact.GetAt(i);

        if (piRegInfoContact == IMS_NULL)
        {
            continue;
        }

        if (CompareUriAssociatedWithContact(piRegInfoContact->GetUri()))
        {
            break;
        }

        piRegInfoContact = IMS_NULL;
    }

    return piRegInfoContact;
}

PROTECTED VIRTUAL IMS_BOOL AosSubscription::CompareUriAssociatedWithContact(
        IN const SipAddress& objUri)
{
    if (objUri.GetHost().Equals(m_objContactAddress.GetHost()) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    if (objUri.GetPort() != m_objContactAddress.GetPort())
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_SINT32 AosSubscription::ConvertRegInfoEvent(IN IMS_SINT32 nEvent)
{
    switch (nEvent)
    {
        case IRegInfoContact::EVENT_EXPIRED:
            return EVENT_EXPIRED;
        case IRegInfoContact::EVENT_DEACTIVATED:
            return EVENT_DEACTIVATED;
        case IRegInfoContact::EVENT_PROBATION:
            return EVENT_PROBATION;
        case IRegInfoContact::EVENT_UNREGISTERED:
            return EVENT_UNREGISTERED;
        case IRegInfoContact::EVENT_REJECTED:
            return EVENT_REJECTED;
        default:
            return EVENT_UNKNOWN;
    }
}

PROTECTED VIRTUAL void AosSubscription::ProcessNotifyState_Terminated(IN IMS_SINT32 nEvent)
{
    (void)nEvent;
}

PROTECTED VIRTUAL void AosSubscription::ProcessNotifyState_Active(IN IMS_SINT32 nState)
{
    if (nState == IRegInfoContact::STATE_ACTIVE)
    {
        ProcessRegEventChange(SipStatusCode::SC_200);
    }

    IMS_BOOL bSupported = GET_N_CONFIG(m_piContext->GetSlotId())
                                  ->IsExtraRegErrRetryCntSharedForRegAndSubRequired();
    IMS_SINT32 nMaxCount = GET_N_CONFIG(m_piContext->GetSlotId())->GetExtraRegErrMaxCount();
    if ((bSupported == IMS_TRUE) && (nMaxCount > 0))
    {
        AosProvider::GetInstance()
                ->GetRetryRepository(m_piContext->GetSlotId())
                ->ResetRetryCount(AosRetryRepository::TYPE_NORMAL);
    }
}

PROTECTED VIRTUAL void AosSubscription::ProcessNotifyState_InvalidBody()
{
    ProcessRegEventChange(SipStatusCode::SC_INVALID);
}

PROTECTED VIRTUAL void AosSubscription::ProcessRegEventChange(IN IMS_UINT32 nStatusCode)
{
    IMS_SINT32 nPolicy = GET_N_CONFIG(m_piContext->GetSlotId())->GetUsatRegEventDownloadPolicy();
    if (nPolicy == CarrierConfig::Ims::USAT_REG_EVENT_NOT_DOWNLOAD)
    {
        return;
    }

    IAosService* piService = AosProvider::GetInstance()->GetService(m_piContext->GetSlotId());
    if (piService == IMS_NULL)
    {
        return;
    }

    ImsList<IRegInfoRegistration*> objRegInfoRegistrations =
            m_piRegSubscription->GetRegInfo()->GetRegistrations();
    ImsList<AString> objImpus;

    for (IMS_UINT32 i = 0; i < objRegInfoRegistrations.GetSize(); ++i)
    {
        const IRegInfoRegistration* piRegInfo = objRegInfoRegistrations.GetAt(i);
        if (piRegInfo != IMS_NULL)
        {
            objImpus.Append(piRegInfo->GetAor().ToString());
        }
    }

    if (nPolicy == CarrierConfig::Ims::USAT_REG_EVENT_UNCONDITIONAL_DOWNLOAD)
    {
        piService->NotifyRegEventState(nStatusCode, objImpus);
    }

    // Notice : Handling for USAT_REG_EVENT_CONDITIONAL_DOWNLOAD has not yet been considered.
}

PROTECTED VIRTUAL void AosSubscription::RegSubscription_NotifyReceived(
        IN IMS_SINT32 nSubState, IN IMS_SINT32 nReasonParam, IN IMS_BOOL bHasBody)
{
    A_IMS_TRACE_I(AOSTAG,
            "RegSubscription_NotifyReceived :: "
            "nSubState (%d), nReasonParam (%d), bHasBody (%d)",
            nSubState, nReasonParam, bHasBody);

    if (bHasBody == IMS_TRUE)
    {
        m_nAorState = IRegInfoContact::STATE_TERMINATED;

        const IRegInfoRegistration* piRegInfo =
                m_piRegSubscription->GetRegInfo()->GetRegistration(m_strAor);
        if (piRegInfo == IMS_NULL)
        {
            A_IMS_TRACE_I(AOSTAG, "RegInfo (%s) is not found", m_strAor.GetStr(), 0, 0);
            ProcessNotifyState_InvalidBody();
            return;
        }

        ImsList<IRegInfoContact*> objContact = piRegInfo->GetContacts();
        if (objContact.GetSize() == 0)
        {
            A_IMS_TRACE_I(AOSTAG, "no contact of reg info", 0, 0, 0);
            return;
        }

        PrintRegInfo(objContact);

        const IRegInfoContact* piRegInfoContact = GetRegInfoContact(objContact);
        if (piRegInfoContact == IMS_NULL)
        {
            A_IMS_TRACE_I(
                    AOSTAG, "contact of reg info is not matched from contact address", 0, 0, 0);
            return;
        }

        IMS_SINT32 nGetState = piRegInfoContact->GetState();
        IMS_SINT32 nEvent = piRegInfoContact->GetEvent();
        IMS_UINT32 nRetryAfter = static_cast<IMS_UINT32>(piRegInfoContact->GetRetryAfterValue());

        A_IMS_TRACE_I(AOSTAG, "State (%s), Event (%s)", RegInfoStateToString(nGetState),
                RegInfoEventToString(nEvent), 0);

        if (nGetState == IRegInfoContact::STATE_TERMINATED)
        {
            ProcessNotifyState_Terminated(nEvent);
            ReportNotifyEvent(ConvertRegInfoEvent(nEvent), nRetryAfter);
        }
        else
        {
            m_nAorState = IRegInfoContact::STATE_ACTIVE;
            ProcessNotifyState_Active(nGetState);
            ReportNotifyEvent(EVENT_REGISTERED);
        }
    }
    else
    {
        A_IMS_TRACE_D(AOSTAG, "NOTIFY without Body", 0, 0, 0);
    }
}

PROTECTED VIRTUAL void AosSubscription::RegSubscription_RefreshTimerExpired(
        OUT IMS_BOOL& bDoImplicitRefresh)
{
    A_IMS_TRACE_I(AOSTAG, "RegSubscription_RefreshTimerExpired", 0, 0, 0);

    bDoImplicitRefresh = IMS_FALSE;

    if (m_piListener->Subscription_CanBeTransmitted())
    {
        if (CheckRadioReadyAndSetRadioWaiting())
        {
            // refresh subscription from engine
            bDoImplicitRefresh = IMS_TRUE;
            SetState(STATE_SUBREFRESHING);
        }
        else
        {
            SetState(STATE_SUBREFRESHSTOP);
        }
    }
    else
    {
        // after expiring subscription, do initial subscription
        SetState(STATE_OFFLINE);
    }
}

PROTECTED VIRTUAL void AosSubscription::RegSubscription_Started()
{
    A_IMS_TRACE_I(AOSTAG, "RegSubscription_Started", 0, 0, 0);

    StopTimer();
    ClearThrottlingCount();

    SetState(STATE_SUBSCRIBED);
    ReportState(REASON_SUB_ESTABLISHED, CMD_NONE);
}

PROTECTED VIRTUAL void AosSubscription::RegSubscription_StartFailed(IN IMS_SINT32 nReason)
{
    A_IMS_TRACE_I(AOSTAG, "RegSubscription_StartFailed :: reason(%s)",
            RegSubReasonToString(nReason), 0, 0);

    switch (nReason)
    {
        case IRegSubscription::REASON_STATUS_CODE:
            ProcessStartFailed_StatusCode(AosUtil::GetInstance()->GetResponseCode(
                    m_piRegSubscription->GetPreviousResponse()));
            break;

        case IRegSubscription::REASON_TRANSACTION_TIMEOUT:
            // "Timer F" should be replaced to 0
            ProcessStartFailed_StatusCode(0);
            break;

        default:
            ProcessStartFailed_Others(nReason);
            break;
    }
}

PROTECTED VIRTUAL void AosSubscription::RegSubscription_Updated()
{
    A_IMS_TRACE_I(AOSTAG, "RegSubscription_Updated", 0, 0, 0);

    StopTimer();

    ClearThrottlingCount();
    SetState(STATE_SUBSCRIBED);
    ReportState(REASON_SUB_ESTABLISHED, CMD_NONE);
}

PROTECTED VIRTUAL void AosSubscription::RegSubscription_UpdateFailed(IN IMS_SINT32 nReason)
{
    A_IMS_TRACE_I(AOSTAG, "RegSubscription_UpdateFailed :: reason(%s)",
            RegSubReasonToString(nReason), 0, 0);

    switch (nReason)
    {
        case IRegSubscription::REASON_STATUS_CODE:
            ProcessUpdateFailed_StatusCode(AosUtil::GetInstance()->GetResponseCode(
                    m_piRegSubscription->GetPreviousResponse()));
            break;

        case IRegSubscription::REASON_TRANSACTION_TIMEOUT:
            // "Timer F" should be replaced to 0
            ProcessUpdateFailed_StatusCode(0);
            break;

        default:
            ProcessUpdateFailed_Others(nReason);
            break;
    }
}

PROTECTED VIRTUAL void AosSubscription::RegSubscription_Removed()
{
    A_IMS_TRACE_I(AOSTAG, "RegSubscription_Removed", 0, 0, 0);

    SetState(STATE_OFFLINE);
    ReportState(REASON_SUB_REMOVED, CMD_NONE);
}

PROTECTED VIRTUAL void AosSubscription::RegSubscription_Terminated(IN IMS_SINT32 nReason)
{
    A_IMS_TRACE_I(AOSTAG, "RegSubscription_Terminated :: reason(%d)", nReason, 0, 0);

    SetState(STATE_OFFLINE);
    ReportState(REASON_SUB_TERMINATED,
            (GET_N_CONFIG(m_piContext->GetSlotId())->IsInitSubUponSubTerminated() == IMS_TRUE)
                    ? CMD_SUB_REQUIRED
                    : CMD_NONE);
}

PROTECTED VIRTUAL void AosSubscription::Transaction_OnConnectionFailed(
        IN IMS_UINT32 nFailureReason, IN IMS_UINT32 /* nCauseCode */, IN IMS_UINT32 nWaitTimeMillis)
{
    A_IMS_TRACE_I(AOSTAG, "Transaction_OnConnectionFailed :: reason(%d), nWaitTimeMillis(%d)",
            nFailureReason, nWaitTimeMillis, 0);

    if (!IsRadioWaiting())
    {
        return;
    }

    if (nFailureReason == IImsRadio::REASON_ACCESS_DENIED)
    {
        StartTimer(RETRY_DEFAULT_WAIT_TIME * 1000);
    }
    else
    {
        if (nWaitTimeMillis > 0)
        {
            StartTimer(nWaitTimeMillis * 1000);
        }
        else
        {
            Transaction_OnConnectionSetupPrepared();
            return;
        }
    }

    SetRadioWaiting(IMS_FALSE);
}

PROTECTED VIRTUAL void AosSubscription::Transaction_OnConnectionSetupPrepared()
{
    if (IsRadioWaiting())
    {
        A_IMS_TRACE_D(AOSTAG, "Transaction_OnConnectionSetupPrepared", 0, 0, 0);
        SetRadioWaiting(IMS_FALSE);
        Start(IMS_FALSE);
    }
}

PROTECTED VIRTUAL void AosSubscription::Transaction_OnTrafficPriorityChanged()
{
    if (IsTrafficPriorityBlocked())
    {
        A_IMS_TRACE_D(AOSTAG, "Transaction_OnTrafficPriorityChanged", 0, 0, 0);
        SetTrafficPriorityBlocked(IMS_FALSE);
        Start(IMS_TRUE);
    }
}

PUBLIC VIRTUAL void AosSubscription::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer != m_piRetryTimer)
    {
        return;
    }

    StopTimer();

    ProcessTimerExpired();
}

PUBLIC GLOBAL const IMS_CHAR* AosSubscription::StateToString(IN IMS_UINT32 nState)
{
    switch (nState)
    {
        case STATE_OFFLINE:
            return "STATE_OFFLINE";
        case STATE_SUBSCRIBING:
            return "STATE_SUBSCRIBING";
        case STATE_SUBSTOP:
            return "STATE_SUBSTOP";
        case STATE_SUBSCRIBED:
            return "STATE_SUBSCRIBED";
        case STATE_SUBREFRESHING:
            return "STATE_SUBREFRESHING";
        case STATE_SUBREFRESHSTOP:
            return "STATE_SUBREFRESHSTOP";
        case STATE_UNSUBSCRIBING:
            return "STATE_UNSUBSCRIBING";
        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosSubscription::RegSubReasonToString(IN IMS_SINT32 nReason)
{
    switch (nReason)
    {
        case IRegSubscription::REASON_NONE:
            return "REASON_NONE";
        case IRegSubscription::REASON_STATUS_CODE:
            return "REASON_STATUS_CODE";
        case IRegSubscription::REASON_NO_EXPIRES:
            return "REASON_NO_EXPIRES";
        case IRegSubscription::REASON_INTERNAL_ERROR:
            return "REASON_INTERNAL_ERROR";
        case IRegSubscription::REASON_TRANSACTION_TIMEOUT:
            return "REASON_TRANSACTION_TIMEOUT";
        case IRegSubscription::REASON_REFRESH_TIMEOUT:
            return "REASON_REFRESH_TIMEOUT";
        case IRegSubscription::REASON_REFRESH_INTERNAL_ERROR:
            return "REASON_REFRESH_INTERNAL_ERROR";
        case IRegSubscription::REASON_NOTIFY_TERMINATED:
            return "REASON_NOTIFY_TERMINATED";
        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosSubscription::RegInfoStateToString(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case IRegInfoContact::STATE_CREATED:
            return "STATE_CREATED";
        case IRegInfoContact::STATE_ACTIVE:
            return "STATE_ACTIVE";
        case IRegInfoContact::STATE_TERMINATED:
            return "STATE_TERMINATED";
        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosSubscription::RegInfoEventToString(IN IMS_SINT32 nEvent)
{
    switch (nEvent)
    {
        case IRegInfoContact::EVENT_REGISTERED:
            return "EVENT_REGISTERED";
        case IRegInfoContact::EVENT_CREATED:
            return "EVENT_CREATED";
        case IRegInfoContact::EVENT_REFRESHED:
            return "EVENT_REFRESHED";
        case IRegInfoContact::EVENT_SHORTENED:
            return "EVENT_SHORTENED";
        case IRegInfoContact::EVENT_EXPIRED:
            return "EVENT_EXPIRED";
        case IRegInfoContact::EVENT_DEACTIVATED:
            return "EVENT_DEACTIVATED";
        case IRegInfoContact::EVENT_PROBATION:
            return "EVENT_PROBATION";
        case IRegInfoContact::EVENT_UNREGISTERED:
            return "EVENT_UNREGISTERED";
        case IRegInfoContact::EVENT_REJECTED:
            return "EVENT_REJECTED";
        default:
            return "__INVALID__";
    }
}
