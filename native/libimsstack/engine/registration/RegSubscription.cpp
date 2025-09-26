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
#include "ServiceUtil.h"

#include "ImsIdentity.h"

#include "Connector.h"
#include "IRegInfoManager.h"
#include "IRegSubscriptionListener.h"
#include "ISipDialog.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipServerConnection.h"
#include "ImsCoreContext.h"
#include "PAccessNetworkInfoHeader.h"
#include "RegContact.h"
#include "RegInfo.h"
#include "RegSubscription.h"
#include "SipConfigProxy.h"
#include "SipDebug.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "SipTimerValuesHelper.h"
#include "SubscriberRefreshHelper.h"
#include "SubscriberState.h"
#include "base/Ims.h"
#include "util/DialogMethodManager.h"
// NOTIFY_REQUEST_HANDLING_AFTER_DE_REG
#include "util/ISipConnectionNotifierManager.h"
#include "util/UserAgentHeader.h"

__IMS_TRACE_TAG_REG__;

#define REG_SUB_MAKE_LPARAM(HIWORD, LOWORD) (IMS_UINT32)(((HIWORD) << 16) | (LOWORD))

#define REG_SUB_HIWORD(LPARAM)              (IMS_UINT16)(((LPARAM) >> 16) & (0xFFFF))

#define REG_SUB_LOWORD(LPARAM)              (IMS_UINT16)((LPARAM)&0xFFFF)

PRIVATE GLOBAL const IMS_CHAR RegSubscription::EVENT[] = "reg";
PRIVATE GLOBAL const IMS_CHAR RegSubscription::MEDIA_TYPE[] = "application";
PRIVATE GLOBAL const IMS_CHAR RegSubscription::MEDIA_SUB_TYPE[] = "reginfo+xml";

PUBLIC
RegSubscription::RegSubscription(IN const RegKey& objRegKey, IN RegStateTracker* pRegStateTracker,
        IN IMS_UINT32 nExpiresValue /*= 0*/, IN const SipTimerValues* pTimerValues /*= IMS_NULL*/) :
        Method(),
        m_nFeatureSet(0),
        m_nState(STATE_INACTIVE),
        m_nExpiresValue(DEFAULT_EXPIRES),
        m_nPendingOperation(SubState::NO_OPERATION),
        m_piListener(IMS_NULL),
        m_objRegKey(objRegKey),
        m_pRegStateTracker(pRegStateTracker),
        m_pSubState(IMS_NULL),
        m_pRefreshHelper(IMS_NULL),
        m_piOngoingScc(IMS_NULL),
        m_piNextRequest(IMS_NULL),
        m_piPreviousRequest(IMS_NULL),
        m_piPreviousResponse(IMS_NULL),
        m_pSipTimerValues(IMS_NULL),
        m_piReferredScn(IMS_NULL)
{
    // Default expires value of 'reg' event package will be set to the pre-configured value.
    if (SipConfigProxy::IsRegSubExpiresConfigured(GetSlotId(), m_pRegStateTracker->GetSipProfile()))
    {
        m_nExpiresValue =
                SipConfigProxy::GetRegSubExpires(GetSlotId(), m_pRegStateTracker->GetSipProfile());
    }
    else if (nExpiresValue != 0)
    {
        m_nExpiresValue = nExpiresValue;
    }

    if (pTimerValues != IMS_NULL)
    {
        m_pSipTimerValues = new SipTimerValues(*pTimerValues);
    }

    // NOTIFY_REQUEST_HANDLING_AFTER_DE_REG
    if (!m_pRegStateTracker.IsNull())
    {
        m_piReferredScn = ImsCoreContext::GetInstance()
                                  ->GetSipConnectionNotifierManager()
                                  ->GetConnectionNotifier(m_pRegStateTracker->GetIpAddress(),
                                          m_pRegStateTracker->GetPortUs());
    }
}

PUBLIC VIRTUAL RegSubscription::~RegSubscription()
{
    ImsCoreContext::GetInstance()->GetRegInfoManager()->DestroyRegInfo(m_objRegKey);
    m_objRegKey.Invalidate();

    DialogMethodManager::GetInstance()->RemoveMethod(GetName());

    if (m_pRefreshHelper != IMS_NULL)
    {
        delete m_pRefreshHelper;
        m_pRefreshHelper = IMS_NULL;
    }

    if (m_pSubState != IMS_NULL)
    {
        delete m_pSubState;
        m_pSubState = IMS_NULL;
    }

    if (m_piOngoingScc != IMS_NULL)
    {
        m_piOngoingScc->Close();
        m_piOngoingScc = IMS_NULL;
    }

    ClearNextRequest();
    SetPreviousRequest(IMS_NULL);
    SetPreviousResponse(IMS_NULL);

    if (m_pSipTimerValues != IMS_NULL)
    {
        delete m_pSipTimerValues;
        m_pSipTimerValues = IMS_NULL;
    }

    // NOTIFY_REQUEST_HANDLING_AFTER_DE_REG
    ImsCoreContext::GetInstance()->GetSipConnectionNotifierManager()->ReleaseConnectionNotifier(
            m_piReferredScn);

    IMS_TRACE_D("Destructor :: RegSubscription", 0, 0, 0);
}

PUBLIC VIRTUAL ISipMessage* RegSubscription::GetNextRequest()
{
    if (m_piNextRequest == IMS_NULL)
    {
        m_piNextRequest = SipParsingHelper::CreateMessage(ISipMessage::TYPE_REQUEST);
    }

    return m_piNextRequest;
}

PUBLIC VIRTUAL void RegSubscription::SetSipMessageMediator(IN IMessageMediator* piMediator)
{
    SetMessageMediator(piMediator);

    if (m_pRefreshHelper != IMS_NULL)
    {
        m_pRefreshHelper->SetMessageMediator(piMediator);
    }
}

PUBLIC VIRTUAL void RegSubscription::DestroyEx()
{
    m_piListener = IMS_NULL;

    if (m_pRefreshHelper != IMS_NULL)
    {
        if (m_pRefreshHelper->IsRequestPending())
        {
            m_pRefreshHelper->AbortConnection();
        }
    }

    if (m_piOngoingScc != IMS_NULL)
    {
        m_piOngoingScc->Close();
        m_piOngoingScc = IMS_NULL;
    }

    ImsCoreContext::GetInstance()->GetRegInfoManager()->DestroyRegInfo(m_objRegKey);
    m_objRegKey.Invalidate();

    DialogMethodManager::GetInstance()->RemoveMethod(GetName());

    // NOTIFY_REQUEST_HANDLING_AFTER_DE_REG
    ImsCoreContext::GetInstance()->GetSipConnectionNotifierManager()->ReleaseConnectionNotifier(
            m_piReferredScn);

    // SIP_MESSAGE_MEDIATOR
    SetSipMessageMediator(IMS_NULL);

    Destroy();
}

PUBLIC VIRTUAL IMS_SINT32 RegSubscription::DisableFeatures(IN IMS_SINT32 nFeatures)
{
    m_nFeatureSet &= (~nFeatures);

    return m_nFeatureSet;
}

PUBLIC VIRTUAL IMS_SINT32 RegSubscription::EnableFeatures(IN IMS_SINT32 nFeatures)
{
    m_nFeatureSet |= nFeatures;

    return m_nFeatureSet;
}

PUBLIC VIRTUAL const IRegInfo* RegSubscription::GetRegInfo() const
{
    return ImsCoreContext::GetInstance()->GetRegInfoManager()->GetRegInfo(m_objRegKey);
}

PUBLIC VIRTUAL IMS_RESULT RegSubscription::SetContactParameter(
        IN const AString& strParameter, IN IMS_SINT32 nOperation /*= CONTACT_PARAM_OP_ADD*/)
{
    ISipDialog* piDialog = GetDialog();

    if (piDialog != IMS_NULL)
    {
        IMS_SINT32 nDialogState = piDialog->GetState();

        if (nDialogState == ISipDialog::STATE_CONFIRMED)
        {
            return piDialog->SetContactParameter(strParameter, nOperation);
        }
    }

    return IMS_FAILURE;
}

PUBLIC VIRTUAL void RegSubscription::SetExpires(IN IMS_UINT32 nExpires)
{
    // Do not check the current state for refresh failure case
    // if (GetState() != STATE_INACTIVE)
    //    return;

    m_nExpiresValue = nExpires;

    m_pSubState->GetEventPackage()->SetDuration(m_nExpiresValue);
}

PUBLIC VIRTUAL void RegSubscription::SetRefreshPolicy(IN IMS_SINT32 nPolicy,
        IN IMS_SINT32 nCriteriaInterval, IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt)
{
    if (m_pRefreshHelper == IMS_NULL)
    {
        return;
    }

    switch (nPolicy)
    {
        case REFRESH_POLICY_NO_REFRESH:
            m_pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_NO_REFRESH, nCriteriaInterval, nValueEorLt, nValueGt);
            break;
        case REFRESH_POLICY_SPEC:
            m_pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_SPEC, nCriteriaInterval, nValueEorLt, nValueGt);
            break;
        case REFRESH_POLICY_REMAIN_TIME:
            m_pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_REMAIN_TIME, nCriteriaInterval, nValueEorLt, nValueGt);
            break;
        case REFRESH_POLICY_RATIO:
            m_pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_RATIO, nCriteriaInterval, nValueEorLt, nValueGt);
            break;
        default:
            IMS_TRACE_E(0, "Invalid refresh policy (%d)", nPolicy, 0, 0);
            break;
    }
}

PUBLIC VIRTUAL IMS_RESULT RegSubscription::Subscribe()
{
    if ((GetState() != STATE_INACTIVE) && (GetState() != STATE_ACTIVE))
    {
        IMS_TRACE_E(
                0, "To subscribe an event, the state MUST be an INACTIVE or ACTIVE state", 0, 0, 0);
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    if (m_pSubState->IsInstantSubscription())
    {
        IMS_TRACE_E(0, "INVALID OPERATION :: It is for an instant subscription.", 0, 0, 0);
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    // if the state is in ACTIVE and refresh is started by the subscription,
    // keep the request and after refresh is completed, try to send a SUBSCRIBE request...
    if (m_pSubState->GetOperation() == SubState::OPERATION_IMPLICIT_REFRESH)
    {
        m_nPendingOperation = SubState::OPERATION_REFRESH;

        SetState(STATE_PENDING);

        Ims::SetLastError(ImsError::NO_ERROR);
        return IMS_FAILURE;
    }

    ISipClientConnection* piScc = CreateConnection(this);

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    piScc->SetListener(this);
    piScc->SetErrorListener(this);

    ISipMessage* piSipMsg = piScc->GetMessage();

    if (!SetHeaders(piSipMsg))
    {
        piScc->Close();
        return IMS_FAILURE;
    }

    // Copy the headers from the application
    if (!CopyHeadersAndBodyParts(piSipMsg))
    {
        piScc->Close();
        return IMS_FAILURE;
    }

    // Add "gruu" option tag to Supported header

    // Set Expires header
    if (!piSipMsg->IsHeaderPresent(ISipHeader::EXPIRES_ANY))
    {
        // Expires header ?
        AString strExpires;

        if (m_pSubState->GetEventPackage()->GetDuration() != (-1))
        {
            strExpires.Sprintf("%d", m_pSubState->GetEventPackage()->GetDuration());
        }
        else
        {
            strExpires.Sprintf("%d", m_pSubState->GetEventPackage()->GetDefaultDuration());
        }

        piSipMsg->SetHeader(ISipHeader::EXPIRES_ANY, strExpires);
    }

    // SIP_MESSAGE_MEDIATOR
    (void)AdjustMessage(piScc->GetMessage(), MESSAGE_CLASS_NORMAL);

    // Try to send a SUBSCRIBE request to the network
    if (piScc->Send() != IMS_SUCCESS)
    {
        piScc->Close();

        IMS_TRACE_E(0, "Sending SUBSCRIBE request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Clear the next request
    ClearNextRequest();

    // Store the ongoing SIP client connection
    SetOngoingConnection(piScc);

    // Update the last request/response message
    SetPreviousRequest(piScc->GetMessage());
    SetPreviousResponse(IMS_NULL);

    if (GetState() == STATE_INACTIVE)
    {
        m_pSubState->SetOperation(SubState::OPERATION_CREATE);
    }
    else
    {
        m_pSubState->SetOperation(SubState::OPERATION_REFRESH);
    }

    // Update the subscription state
    if (!m_pSubState->UpdateState(piSipMsg))
    {
        m_pSubState->SetOperation(SubState::NO_OPERATION);
        return IMS_FAILURE;
    }

    SetState(STATE_PENDING);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT RegSubscription::Unsubscribe()
{
    if (GetState() != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "To unsubscribe an event, the state MUST be an ACTIVE state", 0, 0, 0);
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    // If the state is in ACTIVE and refresh is started by the subscription,
    // keep the request and after refresh is completed, try to send a SUBSCRIBE request...
    if (m_pSubState->GetOperation() == SubState::OPERATION_IMPLICIT_REFRESH)
    {
        m_nPendingOperation = SubState::OPERATION_REMOVE;

        SetState(STATE_PENDING);

        Ims::SetLastError(ImsError::NO_ERROR);
        return IMS_FAILURE;
    }

    ISipClientConnection* piScc = CreateConnection(this);

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection inside of a dialog failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    piScc->SetListener(this);
    piScc->SetErrorListener(this);

    ISipMessage* piSipMsg = piScc->GetMessage();

    if (!SetHeaders(piSipMsg))
    {
        piScc->Close();
        return IMS_FAILURE;
    }

    // Copy the headers from the application
    if (!CopyHeadersAndBodyParts(piSipMsg))
    {
        piScc->Close();
        return IMS_FAILURE;
    }

    // Expires header (overwrites the header field if present)
    piSipMsg->SetHeader(ISipHeader::EXPIRES_ANY, "0");

    // SIP_MESSAGE_MEDIATOR
    (void)AdjustMessage(piScc->GetMessage(), MESSAGE_CLASS_NORMAL);

    // Try to send a SUBSCRIBE request to the network
    if (piScc->Send() != IMS_SUCCESS)
    {
        piScc->Close();

        IMS_TRACE_E(0, "Sending SUBSCRIBE request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Clear the next request
    ClearNextRequest();

    // Store the ongoing SIP client connection
    SetOngoingConnection(piScc);

    // Update the last request/response message
    SetPreviousRequest(piScc->GetMessage());
    SetPreviousResponse(IMS_NULL);

    m_pSubState->SetOperation(SubState::OPERATION_REMOVE);

    // Update the subscription state
    if (!m_pSubState->UpdateState(piSipMsg))
    {
        m_pSubState->SetOperation(SubState::NO_OPERATION);
        return IMS_FAILURE;
    }

    SetState(STATE_PENDING);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_BOOL RegSubscription::DispatchMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->RegSubscription_NotifyReceived(LONG_TO_SINT(objMsg.nWparam),
                        REG_SUB_HIWORD(objMsg.nLparam),
                        (REG_SUB_LOWORD(objMsg.nLparam) == 0) ? IMS_FALSE : IMS_TRUE);
            }
            return IMS_TRUE;
        case AMSG_REG_SUBSCRIPTION_STARTED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->RegSubscription_Started();
            }
            return IMS_TRUE;
        case AMSG_REG_SUBSCRIPTION_START_FAILED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->RegSubscription_StartFailed(LONG_TO_SINT(objMsg.nWparam));
            }
            return IMS_TRUE;
        case AMSG_REG_SUBSCRIPTION_UPDATED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->RegSubscription_Updated();
            }
            return IMS_TRUE;
        case AMSG_REG_SUBSCRIPTION_UPDATE_FAILED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->RegSubscription_UpdateFailed(LONG_TO_SINT(objMsg.nWparam));
            }
            return IMS_TRUE;
        case AMSG_REG_SUBSCRIPTION_REMOVED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->RegSubscription_Removed();
            }
            return IMS_TRUE;
        case AMSG_REG_SUBSCRIPTION_TERMINATED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->RegSubscription_Terminated(LONG_TO_SINT(objMsg.nWparam));
            }
            return IMS_TRUE;
        default:
            break;
    }

    return EngineActivity::DispatchMessage(objMsg);
}

PRIVATE VIRTUAL IMS_BOOL RegSubscription::InitInstance()
{
    if (m_pSubState == IMS_NULL)
    {
        m_pSubState = new SubscriberState();

        if (m_pSubState == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a subscriber state failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (!m_pSubState->CreateEventPackage(EVENT))
        {
            IMS_TRACE_E(0, "Creating an event package for a 'reg' event failed", 0, 0, 0);
            return IMS_FALSE;
        }

        m_pSubState->GetEventPackage()->SetDuration(m_nExpiresValue);
    }

    if (m_pRefreshHelper == IMS_NULL)
    {
        m_pRefreshHelper = new SubscriberRefreshHelper(this, m_pSubState);

        if (m_pRefreshHelper == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a subscription refresh helper failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    DialogMethodManager::GetInstance()->AddMethod(GetName(), this);

    IRegInfoManager* piRegInfoManager = ImsCoreContext::GetInstance()->GetRegInfoManager();
    piRegInfoManager->CreateRegInfo(m_objRegKey);

    RegInfo* pRegInfo = piRegInfoManager->GetRegInfo(m_objRegKey);

    if (pRegInfo != IMS_NULL)
    {
        pRegInfo->AddListener(this);
    }

    return IMS_TRUE;
}

// IMS_AUTH_SIP_DIGEST
PRIVATE VIRTUAL IMS_BOOL RegSubscription::SendRequestToChallenge(IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    if (piSipMsg == IMS_NULL)
    {
        IMS_TRACE_E(0, "SIP message is null", 0, 0, 0);
        return IMS_FALSE;
    }

    // Set P-Access-Network-Info header
    PAccessNetworkInfoHeader::SetHeader(
            GetSlotId(), m_pRegStateTracker->GetIpAddress(), piScc->GetSipProfile(), piSipMsg);

    if (!Method::SendRequestToChallenge(piScc))
    {
        return IMS_FALSE;
    }

    // Update the last request/response message
    SetPreviousRequest(piScc->GetMessage());
    SetPreviousResponse(IMS_NULL);

    // Update the subscription state
    if (!m_pSubState->UpdateState(piScc->GetMessage()))
    {
        m_pSubState->SetOperation(SubState::NO_OPERATION);
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL void RegSubscription::NotifySipResponse(IN ISipClientConnection* piScc)
{
    const ISipMessage* piSipMsg = piScc->GetMessage();
    const SipMethod& objMethod = piSipMsg->GetMethod();

    IMS_TRACE_I("___ Response (%s) received at the state, %d", objMethod.ToString().GetStr(),
            GetState(), 0);

    if (!objMethod.Equals(SipMethod::SUBSCRIBE))
    {
        piScc->Close();
        m_piOngoingScc = IMS_NULL;
        return;
    }

    // Handle the response according to the SIP method.
    IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

    // Handle the response to SUBSCRIBE request ...
    if (SipStatusCode::Is1XX(nStatusCode))
    {
        // Do nothing ...
        return;
    }

    SetPreviousResponse(piSipMsg);

    IMS_SINT32 nOperation = m_pSubState->GetOperation();

    // Update the subscription state
    if (!m_pSubState->UpdateState(piSipMsg))
    {
        if ((nOperation == SubState::OPERATION_CREATE) || (nOperation == SubState::OPERATION_FETCH))
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_START_FAILED, REASON_INTERNAL_ERROR, 0);
        }
        else if (nOperation == SubState::OPERATION_REMOVE)
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_REMOVED, REASON_NONE, 0);
        }
        else
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, REASON_INTERNAL_ERROR, 0);
        }

        m_pSubState->SetOperation(SubState::NO_OPERATION);
        piScc->Close();
        m_piOngoingScc = IMS_NULL;
        return;
    }

    // AUTH_SIP_DIGEST {
    if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        // In case of other method except for REGISTER,
        // the UE only supports the authentication algorithm, MD5
        if (RespondToChallenge(piScc))
        {
            return;
        }
    }
    // }

    // Start or re-start a subscription refresh timer
    m_pRefreshHelper->UpdateOnMessageReceived(piScc);

    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        // RACE_CONDITION: NOTIFY (w/ terminated) & 200 OK to SUBSCRIBE
        CheckNCreateDialog(piScc, IMS_FALSE, IMS_TRUE);

        SetState(STATE_ACTIVE);

        if ((nOperation == SubState::OPERATION_CREATE) || (nOperation == SubState::OPERATION_FETCH))
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_STARTED, REASON_NONE, 0);
        }
        else if (nOperation == SubState::OPERATION_REMOVE)
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_REMOVED, REASON_NONE, 0);
        }
        else
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_UPDATED, REASON_NONE, 0);
        }
    }
    else
    {
        SetState(STATE_INACTIVE);

        if ((nOperation == SubState::OPERATION_CREATE) || (nOperation == SubState::OPERATION_FETCH))
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_START_FAILED, REASON_STATUS_CODE, 0);
        }
        else if (nOperation == SubState::OPERATION_REMOVE)
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_REMOVED, REASON_NONE, 0);
        }
        else
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, REASON_STATUS_CODE, 0);
        }
    }

    m_pSubState->SetOperation(SubState::NO_OPERATION);
    piScc->Close();
    m_piOngoingScc = IMS_NULL;

    // Check if the dialog is terminated or not
    if (nOperation != SubState::OPERATION_CREATE)
    {
        CheckDialogNCallListener();
    }

    // Initialize all the subscriber's state at this moment,
    // because the application MAY use this subscription
    // to add/fetch/refresh/remove the event state.
    if (GetState() == STATE_INACTIVE)
    {
        m_pSubState->Clear();
        DestroyDialog();

        // RESTORE_INITIAL_EXPIRES_VALUE
        m_pSubState->GetEventPackage()->SetDuration(m_nExpiresValue);
    }

    // 3 check "nPendingOperation" member field
}

PRIVATE VIRTUAL void RegSubscription::NotifySipError(
        IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    const SipMethod& objMethod = piSc->GetMethod();

    (void)nCode;
    (void)strMessage;

    if (!objMethod.Equals(SipMethod::SUBSCRIBE))
    {
        piSc->Close();
        m_piOngoingScc = IMS_NULL;
        return;
    }

    IMS_SINT32 nOperation = m_pSubState->GetOperation();

    if ((nOperation == SubState::OPERATION_CREATE) || (nOperation == SubState::OPERATION_FETCH))
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_REG_SUBSCRIPTION_START_FAILED, REASON_TRANSACTION_TIMEOUT, 0);
    }
    else if (nOperation == SubState::OPERATION_REMOVE)
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_REG_SUBSCRIPTION_REMOVED, REASON_NONE, 0);
        PostMessage(AMSG_REG_SUBSCRIPTION_TERMINATED, REASON_NONE, 0);
    }
    else if (nOperation == SubState::OPERATION_REFRESH)
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, REASON_TRANSACTION_TIMEOUT, 0);
        PostMessage(AMSG_REG_SUBSCRIPTION_TERMINATED, REASON_NONE, 0);
    }

    piSc->Close();
    m_piOngoingScc = IMS_NULL;

    // Initialize all the subscriber's state at this moment,
    // because the application MAY use this subscription
    // to add/fetch/refresh/remove the event state.
    if (GetState() == STATE_INACTIVE)
    {
        m_pSubState->Clear();
        DestroyDialog();

        // RESTORE_INITIAL_EXPIRES_VALUE
        m_pSubState->GetEventPackage()->SetDuration(m_nExpiresValue);
    }
}

PRIVATE VIRTUAL IMS_BOOL RegSubscription::Dialog_Compare(IN ISipServerConnection* piSsc) const
{
    // Filters some method which does not handle in the subscription
    if (!piSsc->GetMethod().Equals(SipMethod::NOTIFY))
    {
        return IMS_FALSE;
    }

    ISipDialog* piDialog = GetDialog();

    if (piDialog == IMS_NULL)
    {
        // In case of an early NOTIFY received ...
        if (GetState() == STATE_PENDING)
        {
            IMS_SINT32 nOperation = m_pSubState->GetOperation();

            IMS_TRACE_I("Checks if the early NOTIFY is received or not ...", 0, 0, 0);

            if ((nOperation == SubState::OPERATION_CREATE) ||
                    (nOperation == SubState::OPERATION_FETCH))
            {
                if (m_piOngoingScc != IMS_NULL)
                {
                    piDialog = m_piOngoingScc->GetDialog();
                }
            }
        }
    }

    if (piDialog == IMS_NULL)
    {
        IMS_TRACE_D("No dialog exists", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!piDialog->IsSameDialog(piSsc))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL IMS_BOOL RegSubscription::Dialog_NotifyRequest(IN ISipServerConnection* piSsc)
{
    if (!piSsc->GetMethod().Equals(SipMethod::NOTIFY))
    {
        SendResponse(piSsc, SipStatusCode::SC_405);
        piSsc->Close();
        return IMS_FALSE;
    }

    if (SipConfigProxy::IsRequestUriValidationRequiredInMidDialog(
                GetSlotId(), m_pRegStateTracker->GetSipProfile()))
    {
        // Checks if Request-URI is matched or not
        const AString& strRequestUri = piSsc->GetRequestUri();
        SipAddress objRequestUri(strRequestUri);

        if (!ValidateRequestUri(objRequestUri, piSsc->GetDialog()))
        {
            IMS_TRACE_D("Request-URI (%s) in mid-dialog request is not matched",
                    SipDebug::GetUri1(strRequestUri).GetStr(), 0, 0);

            SendResponse(piSsc, SipStatusCode::SC_404);
            piSsc->Close();
            return IMS_FALSE;
        }
    }

    // Update the dialog info.
    CheckNCreateDialog(piSsc);

    // Update the subscription state for NOTIFY request
    if (!m_pSubState->UpdateState(piSsc->GetMessage()))
    {
        piSsc->Close();
        return IMS_FALSE;
    }

    const ISipMessage* piSipMsgNotify = piSsc->GetMessage();

    if (piSipMsgNotify == IMS_NULL)
    {
        if (SendResponse(piSsc, SipStatusCode::SC_500))
        {
            m_pSubState->UpdateState(piSsc->GetMessage());
            m_pRefreshHelper->UpdateOnMessageSent(piSsc);
        }

        piSsc->Close();
        return IMS_FALSE;
    }

    IMS_UINT16 nReasonParameter = GetReasonParameter(piSipMsgNotify);

    IMS_TRACE_D("START - reginfo parsing ...", 0, 0, 0);

    // Parsing 'application/reginfo+xml' using XML parser & update the reginfo.
    ImsList<ISipMessageBodyPart*> objBodyParts = piSipMsgNotify->GetBodyParts();
    IMS_UINT16 nBodyParts = 0;

    if (!objBodyParts.IsEmpty())
    {
        AString strContentType;
        AString strType;
        AString strSubType;

        for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); ++i)
        {
            const ISipMessageBodyPart* piBodyPart = objBodyParts.GetAt(i);

            if (piBodyPart == IMS_NULL)
            {
                continue;
            }

            strContentType = piBodyPart->GetHeader(ISipMessageBodyPart::CONTENT_TYPE);

            if (!TextParser::ParseMediaType(strContentType, strType, strSubType))
            {
                continue;
            }

            if (!strType.EqualsIgnoreCase(MEDIA_TYPE) ||
                    !strSubType.EqualsIgnoreCase(MEDIA_SUB_TYPE))
            {
                continue;
            }

            AString strRegInfo = piBodyPart->GetContent().ToString();

            strContentType.Prepend("REG_INFO ( ");
            strContentType.Append(" )");

            if (!IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG())
            {
                IMS_TRACE_XML(strContentType.GetStr(), strRegInfo.GetStr(), strRegInfo.GetLength());
            }

            ImsCoreContext::GetInstance()->GetRegInfoManager()->Update(m_objRegKey, strRegInfo);

            nBodyParts = 1;
        }
    }
    else
    {
        IMS_TRACE_D("RegSubscription -> No body parts", 0, 0, 0);
    }

    // Send a 2xx response to NOTIFY request immediately
    if (SendResponse(piSsc, SipStatusCode::SC_200) == IMS_FALSE)
    {
        piSsc->Close();

        IMS_TRACE_E(0, "Creating & sending the response to NOTIFY request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Update the subscription state for NOTIFY response
    if (!m_pSubState->UpdateState(piSsc->GetMessage()))
    {
        piSsc->Close();
        return IMS_FALSE;
    }

    // Update the subscription refresh timer
    m_pRefreshHelper->UpdateOnMessageSent(piSsc);

    piSsc->Close();

    PostMessage(AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED, m_pSubState->GetSubState(),
            REG_SUB_MAKE_LPARAM(nReasonParameter, nBodyParts));

    if (m_pSubState->GetState() == SubState::STATE_TERMINATED)
    {
        PostMessage(AMSG_REG_SUBSCRIPTION_TERMINATED, REASON_NOTIFY_TERMINATED, 0);
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL void RegSubscription::Refreshable_RefreshCompleted(
        IN ISipClientConnection* piScc, IN IMS_SINT32 nCode /*= 0*/)
{
    (void)piScc;

    IMS_TRACE_I("___ REGISTRATION SUBSCRIBER REFRESH COMPLETED ... Code (%d)", nCode, 0, 0);

    // In case, the subscription refresh request is successfully done.
    if (nCode == 0)
    {
        IMS_SINT32 nStatusCode = piScc->GetStatusCode();

        SetPreviousResponse(piScc->GetMessage());

        if (!m_pSubState->UpdateState(piScc->GetMessage()))
        {
            // Set the operation
            m_pSubState->SetOperation(SubState::NO_OPERATION);

            // Check if the dialog is terminated or not
            CheckDialogNCallListener();

            return;
        }

        // AUTH_SIP_DIGEST {
        if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
        {
            // In case of other method except for REGISTER,
            // the UE only supports the authentication algorithm, MD5
            if (RespondToChallenge(piScc))
            {
                return;
            }
        }

        ResetChallengeCount(piScc);
        // }

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_UPDATED, REASON_STATUS_CODE, 0);
        }
        else
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, REASON_STATUS_CODE, 0);
        }

        // Check if the dialog is terminated or not
        CheckDialogNCallListener();
    }
    // The subscription refresh request is timed out.
    else if (nCode == RefreshHelper::TRANSACTION_TIMEOUT)
    {
        // 4 what to do ... ? In this moment, do nothing ...
        PostMessage(AMSG_REG_SUBSCRIPTION_UPDATE_FAILED, REASON_TRANSACTION_TIMEOUT, 0);

        if (!IsFeatureEnabled(FEATURE_KEEP_ACTIVE_ON_REFRESH_TRANSACTION_TIMEOUT))
        {
            Refreshable_RefreshTerminated();
        }
    }

    // Set the operation
    m_pSubState->SetOperation(SubState::NO_OPERATION);
}

PRIVATE VIRTUAL IMS_BOOL RegSubscription::Refreshable_RefreshStarted()
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;

    IMS_TRACE_I("___ REGISTRATION SUBSCRIPTION REFRESH STARTED ... State(%d)", GetState(), 0, 0);

    if (GetState() == STATE_ACTIVE)
    {
        if (m_piListener != IMS_NULL)
        {
            m_piListener->RegSubscription_RefreshTimerExpired(bDoImplicitRefresh);
        }

        if (bDoImplicitRefresh)
        {
            return SubscribeOnImplicitRefresh();
        }

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE VIRTUAL void RegSubscription::Refreshable_RefreshTerminated()
{
    IMS_TRACE_D("_____ REGISTRATION SUBSCRIBER REFRESH TERMINATED ...", 0, 0, 0);

    if (GetState() == STATE_ACTIVE)
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_REG_SUBSCRIPTION_TERMINATED, REASON_REFRESH_TIMEOUT, 0);
    }

    if (GetState() == STATE_INACTIVE)
    {
        m_pSubState->Clear();
        DestroyDialog();

        // RESTORE_INITIAL_EXPIRES_VALUE
        m_pSubState->GetEventPackage()->SetDuration(m_nExpiresValue);
    }
}

PRIVATE
void RegSubscription::CheckDialogNCallListener()
{
    // Check if the dialog is terminated or not
    const ISipDialog* piDialog = GetDialog();

    if (piDialog != IMS_NULL)
    {
        if (piDialog->GetState() == ISipDialog::STATE_TERMINATED)
        {
            PostMessage(AMSG_REG_SUBSCRIPTION_TERMINATED, REASON_NONE, 0);
        }
    }
    else
    {
        PostMessage(AMSG_REG_SUBSCRIPTION_TERMINATED, REASON_NONE, 0);
    }
}

PRIVATE
void RegSubscription::ClearNextRequest()
{
    if (m_piNextRequest != IMS_NULL)
    {
        m_piNextRequest->Destroy();
        m_piNextRequest = IMS_NULL;
    }
}

PRIVATE
IMS_BOOL RegSubscription::CopyHeadersAndBodyParts(IN_OUT ISipMessage*& piSipMsg)
{
    // Copy the headers from the application
    if (m_piNextRequest != IMS_NULL)
    {
        const AString strGruu(Sip::STR_GRUU);
        IMS_BOOL bGruuRequired = piSipMsg->IsOptionSupported(strGruu);

        // Set the headers and body parts if application already sets
        if (piSipMsg->CopyHeadersAndBodyParts(m_piNextRequest) != IMS_SUCCESS)
        {
            Ims::SetLastError(ImsError::GENERAL_ERROR);
            IMS_TRACE_E(0, "Copying SIP headers and message body parts failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (bGruuRequired && !piSipMsg->IsOptionSupported(strGruu))
        {
            piSipMsg->AddHeader(ISipHeader::SUPPORTED, strGruu);
        }
    }

    return IMS_TRUE;
}

PRIVATE
void RegSubscription::SetOngoingConnection(IN ISipClientConnection* piScc)
{
    if (m_piOngoingScc != IMS_NULL)
    {
        m_piOngoingScc->Close();
        m_piOngoingScc = IMS_NULL;
    }

    if (piScc != IMS_NULL)
    {
        m_piOngoingScc = piScc;
    }
}

PRIVATE
void RegSubscription::SetPreviousRequest(IN const ISipMessage* piSipMsg)
{
    if (m_piPreviousRequest != IMS_NULL)
    {
        m_piPreviousRequest->Destroy();
        m_piPreviousRequest = IMS_NULL;
    }

    if (piSipMsg != IMS_NULL)
    {
        m_piPreviousRequest = piSipMsg->Clone();
    }
}

PRIVATE
void RegSubscription::SetPreviousResponse(IN const ISipMessage* piSipMsg)
{
    if (m_piPreviousResponse != IMS_NULL)
    {
        m_piPreviousResponse->Destroy();
        m_piPreviousResponse = IMS_NULL;
    }

    if (piSipMsg != IMS_NULL)
    {
        m_piPreviousResponse = piSipMsg->Clone();
    }
}

PRIVATE
IMS_BOOL RegSubscription::SendResponse(IN ISipServerConnection* piSsc, IN IMS_SINT32 nStatusCode)
{
    if (piSsc->InitResponse(nStatusCode) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Initializing a SIP response failed - SipError(%d)",
                SipError::GetLastError(), 0, 0);

        Ims::SetLastError(ImsError::GENERAL_ERROR);
        return IMS_FALSE;
    }

    ISipMessage* piSipMsg = piSsc->GetMessage();

    // Sets Allow header fields
    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        IMS_BOOL bIsContactGruu = IMS_FALSE;

        // Set Contact headers
        if (!SetContactHeader(piSipMsg, bIsContactGruu))
        {
            Ims::SetLastError(ImsError::GENERAL_ERROR);
            return IMS_FALSE;
        }

        if (bIsContactGruu)
        {
            piSipMsg->AddHeader(ISipHeader::SUPPORTED, Sip::STR_GRUU);
        }

        // Do not P-Preferred-Identity header in case of NOTIFY

        const AStringArray& objMethods = SipConfigProxy::GetRegAllowMethods(
                GetSlotId(), m_pRegStateTracker->GetSipProfile());

        for (IMS_SINT32 j = 0; j < objMethods.GetCount(); ++j)
        {
            if (piSipMsg->AddHeader(ISipHeader::ALLOW, objMethods.GetElementAt(j)) != IMS_SUCCESS)
            {
                Ims::SetLastError(ImsError::GENERAL_ERROR);
                return IMS_FALSE;
            }
        }
    }

    // Sets P-Access-Network-Info header field
    PAccessNetworkInfoHeader::SetHeader(
            GetSlotId(), m_pRegStateTracker->GetIpAddress(), piSsc->GetSipProfile(), piSipMsg);

    // Sets Server header field - User-Agent ?
    if (SipConfigProxy::IsUserAgentConfigured(GetSlotId(), m_pRegStateTracker->GetSipProfile()))
    {
        if (SipConfigProxy::IsUserAgentSetByContext(
                    GetSlotId(), m_pRegStateTracker->GetSipProfile()))
        {
            UserAgentHeader::SetHeader(ISipHeader::SERVER, m_pRegStateTracker->GetSipProfile(),
                    AString::ConstNull(), m_pRegStateTracker->GetIpAddress(), GetSlotId(),
                    piSipMsg);
        }
        else
        {
            UserAgentHeader::SetHeader(ISipHeader::USER_AGENT, m_pRegStateTracker->GetSipProfile(),
                    AString::ConstNull(), m_pRegStateTracker->GetIpAddress(), GetSlotId(),
                    piSipMsg);
        }
    }

    // Sets the SIP transaction timers
    piSsc->SetTransactionTimerValues(
            SipTimerValuesHelper::GetValues(GetSlotId(), m_pRegStateTracker->GetSipProfile()));

    // SIP_MESSAGE_MEDIATOR
    (void)AdjustMessage(piSsc->GetMessage(), MESSAGE_CLASS_AUTOMATIC);

    if (piSsc->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending SIP response failed", 0, 0, 0);
        return IMS_FALSE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RegSubscription::SetContactHeader(
        IN_OUT ISipMessage*& piSipMsg, OUT IMS_BOOL& bIsContactGruu)
{
    const RegContact* pRegContact = m_pRegStateTracker->GetPreferredContact();
    AString strContact;

    bIsContactGruu = IMS_FALSE;

    if (SipConfigProxy::IsGruuConfigured(GetSlotId(), m_pRegStateTracker->GetSipProfile()))
    {
        if (SipConfigProxy::IsMultipleRegConfigured(GetSlotId()))
        {
            SipAddress objContact;

            if (pRegContact != IMS_NULL)
            {
                // 4 Consider the Privacy information (temp-gruu)
                const SipAddress* pPubGruu = pRegContact->GetPublicGruu();

                if (pPubGruu != IMS_NULL)
                {
                    bIsContactGruu = IMS_TRUE;
                    objContact = *pPubGruu;
                }
                else
                {
                    const SipAddress* pContact =
                            m_pRegStateTracker->GetContactAddressForOutgoingMessage();

                    objContact = (pContact != IMS_NULL) ? *pContact
                                                        : m_pRegStateTracker->GetContactAddress();
                }
            }
            else
            {
                const SipAddress* pContact =
                        m_pRegStateTracker->GetContactAddressForOutgoingMessage();

                objContact = (pContact != IMS_NULL) ? *pContact
                                                    : m_pRegStateTracker->GetContactAddress();
            }

            objContact.AddParameter(Sip::STR_OB, AString::ConstNull());
            strContact = objContact.ToString();
        }
        else
        {
            if (pRegContact != IMS_NULL)
            {
                // 4 Consider the Privacy information (temp-gruu)
                const SipAddress* pPubGruu = pRegContact->GetPublicGruu();

                if (pPubGruu != IMS_NULL)
                {
                    bIsContactGruu = IMS_TRUE;
                    strContact = pPubGruu->ToString();
                }
                else
                {
                    const SipAddress* pContact =
                            m_pRegStateTracker->GetContactAddressForOutgoingMessage();

                    strContact = (pContact != IMS_NULL)
                            ? pContact->ToString()
                            : m_pRegStateTracker->GetContactAddress().ToString();
                }
            }
            else
            {
                const SipAddress* pContact =
                        m_pRegStateTracker->GetContactAddressForOutgoingMessage();

                strContact = (pContact != IMS_NULL)
                        ? pContact->ToString()
                        : m_pRegStateTracker->GetContactAddress().ToString();
            }
        }
    }
    else
    {
        if (SipConfigProxy::IsMultipleRegConfigured(GetSlotId()))
        {
            const SipAddress* pContact = m_pRegStateTracker->GetContactAddressForOutgoingMessage();
            SipAddress objContact =
                    (pContact != IMS_NULL) ? *pContact : m_pRegStateTracker->GetContactAddress();
            objContact.AddParameter(Sip::STR_OB, AString::ConstNull());
            strContact = objContact.ToString();
        }
        else
        {
            const SipAddress* pContact = m_pRegStateTracker->GetContactAddressForOutgoingMessage();

            strContact = (pContact != IMS_NULL)
                    ? pContact->ToString()
                    : m_pRegStateTracker->GetContactAddress().ToString();
        }
    }

    IMS_BOOL bInstanceParameterIncluded = IMS_FALSE;

    if (pRegContact != IMS_NULL && m_pRegStateTracker->IsWithinTrustDomain(GetSlotId()) &&
            SipConfigProxy::IsSipInstanceParamRequiredInContactForNonRegisterRequest(
                    GetSlotId(), m_pRegStateTracker->GetSipProfile()))
    {
        // Add the '+sip.instance' parameter
        const SipParameter* pParameter = pRegContact->GetInstanceParameter();

        if (pParameter != IMS_NULL)
        {
            bInstanceParameterIncluded = IMS_TRUE;

            strContact.Append(TextParser::CHAR_SEMICOLON);
            strContact.Append(pParameter->ToString());
        }
    }

    if (piSipMsg->GetMethod().Equals(SipMethod::NOTIFY))
    {
        const ISipDialog* piDialog = GetDialog();
        const ISipHeader* piHeader =
                (piDialog != IMS_NULL) ? piDialog->GetContactHeader() : IMS_NULL;

        if (piHeader != IMS_NULL)
        {
            const ImsList<SipParameter*>& objParams = piHeader->GetParameters();

            for (IMS_UINT32 i = 0; i < objParams.GetSize(); ++i)
            {
                const SipParameter* pParameter = objParams.GetAt(i);

                if (pParameter == IMS_NULL)
                {
                    continue;
                }

                if (bInstanceParameterIncluded)
                {
                    if (pParameter->GetName().Equals(Sip::STR_SIP_INSTANCE))
                    {
                        continue;
                    }
                }

                strContact.Append(TextParser::CHAR_SEMICOLON);
                strContact.Append(pParameter->ToString());
            }
        }
    }

    if (piSipMsg->SetHeader(ISipHeader::CONTACT_NORMAL, strContact) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting Contact header failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RegSubscription::SetHeaders(IN_OUT ISipMessage*& piSipMsg)
{
    if (piSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (piSipMsg->GetMethod().Equals(SipMethod::SUBSCRIBE))
    {
        // Event header
        piSipMsg->SetHeader(ISipHeader::EVENT, EVENT);

        // Add a User-Agent if configurable
        if (SipConfigProxy::IsUserAgentConfigured(GetSlotId(), m_pRegStateTracker->GetSipProfile()))
        {
            UserAgentHeader::SetHeader(ISipHeader::USER_AGENT, m_pRegStateTracker->GetSipProfile(),
                    AString::ConstNull(), m_pRegStateTracker->GetIpAddress(), GetSlotId(),
                    piSipMsg);
        }

        IMS_BOOL bIsContactGruu = IMS_FALSE;

        if (GetState() == STATE_INACTIVE)
        {
            // Sets Contact headers
            if (!SetContactHeader(piSipMsg, bIsContactGruu))
            {
                return IMS_FALSE;
            }

            // Sets Route headers
            const AStringArray& objServiceRoutes = m_pRegStateTracker->GetServiceRoutes();

            for (IMS_SINT32 i = 0; i < objServiceRoutes.GetCount(); ++i)
            {
                if (piSipMsg->AddHeader(ISipHeader::ROUTE, objServiceRoutes.GetElementAt(i)) !=
                        IMS_SUCCESS)
                {
                    IMS_TRACE_E(0, "Setting Route header failed", 0, 0, 0);
                    return IMS_FALSE;
                }
            }
        }
        else
        {
            const ISipDialog* piDialog = GetDialog();
            const ISipHeader* piContactHeader =
                    (piDialog != IMS_NULL) ? piDialog->GetContactHeader() : IMS_NULL;
            const SipAddress* pContact =
                    (piContactHeader != IMS_NULL) ? piContactHeader->GetSipAddress() : IMS_NULL;

            if (pContact != IMS_NULL)
            {
                bIsContactGruu = (pContact->GetParameter("gr") != IMS_NULL);
            }
        }

        const AString strGruu(Sip::STR_GRUU);

        if (bIsContactGruu && !piSipMsg->IsOptionSupported(strGruu))
        {
            piSipMsg->AddHeader(ISipHeader::SUPPORTED, strGruu);
        }
    }

    return IMS_TRUE;
}

PRIVATE
void RegSubscription::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("RegSubscription :: %s to %s", StateToString(m_nState), StateToString(nState), 0);

    m_nState = nState;
}

PRIVATE
IMS_BOOL RegSubscription::SubscribeOnImplicitRefresh()
{
    // Set the operation
    m_pSubState->SetOperation(SubState::OPERATION_IMPLICIT_REFRESH);

    // Send a refresh request : SUBSCRIBE
    ISipClientConnection* piScc = CreateConnection(this);

    if (piScc == IMS_NULL)
    {
        m_pSubState->SetOperation(SubState::NO_OPERATION);

        IMS_TRACE_E(0, "Creating a new SIP connection for a 'reg' subscription refresh failed", 0,
                0, 0);
        return IMS_FALSE;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();
    const ISipMessage* piInitialSipMsg = m_pSubState->GetInitialMessage();

    if (piInitialSipMsg != IMS_NULL)
    {
        if (piSipMsg->CopyHeadersAndBodyParts(piInitialSipMsg) != IMS_SUCCESS)
        {
            m_pSubState->SetOperation(SubState::NO_OPERATION);
            piScc->Close();

            IMS_TRACE_E(0, "Setting SIP headers to refresh SUBSCRIBE request failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    if (m_pRefreshHelper->SendRefreshRequest(piScc) != IMS_SUCCESS)
    {
        m_pSubState->SetOperation(SubState::NO_OPERATION);
        piScc->Close();

        IMS_TRACE_E(0, "Sending a subscription refresh request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Clear the next request
    ClearNextRequest();

    // Update the last request/response message
    SetPreviousRequest(piScc->GetMessage());
    SetPreviousResponse(IMS_NULL);

    // Update the subscription state
    if (!m_pSubState->UpdateState(piScc->GetMessage()))
    {
        m_pSubState->SetOperation(SubState::NO_OPERATION);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

// IMS_REQUEST_URI_VALIDATION_IN_MID_DIALOG
PRIVATE
IMS_BOOL RegSubscription::ValidateRequestUri(
        IN const SipAddress& objRequestUri, IN const ISipDialog* piDialog) const
{
    // Checks GRUU identities if it is supported
    if (SipConfigProxy::IsGruuConfigured(GetSlotId(), m_pRegStateTracker->GetSipProfile()))
    {
        const AString strGr("gr");
        const SipParameter* pParamGr = objRequestUri.GetParameter(strGr);

        if (pParamGr != IMS_NULL)
        {
            if (pParamGr->GetValue().GetLength() > 0)
            {
                // If the pub-gruu is available, then compare it with the Request-URI
                const SipAddress* pPubGruu =
                        m_pRegStateTracker->GetPreferredContact()->GetPublicGruu();

                if (pPubGruu != IMS_NULL)
                {
                    if (pPubGruu->Equals(objRequestUri))
                    {
                        IMS_TRACE_D("pub-gruu is matched", 0, 0, 0);
                        return IMS_TRUE;
                    }
                }
            }
            else
            {
                // 4 List ?
                //  If the temp-gruu is available, then compare it with the Request-URI
                const SipAddress* pTempGruu =
                        m_pRegStateTracker->GetPreferredContact()->GetTemporaryGruu();

                if (pTempGruu != IMS_NULL)
                {
                    if (pTempGruu->Equals(objRequestUri))
                    {
                        IMS_TRACE_D("temp-gruu is matched", 0, 0, 0);
                        return IMS_TRUE;
                    }
                }
            }

            IMS_TRACE_D(
                    "Request-URI contains a 'gr' parameter & GRUU ids are not matched", 0, 0, 0);

            return IMS_FALSE;
        }
    }

    // Checks if the contact address matches or not.
    const SipAddress* pContact = IMS_NULL;

    if (piDialog != IMS_NULL)
    {
        const ISipHeader* piContactHeader = piDialog->GetContactHeader();

        if (piContactHeader != IMS_NULL)
        {
            pContact = piContactHeader->GetSipAddress();
        }
    }

    const SipAddress& objContact = m_pRegStateTracker->GetContactAddress();

    if (pContact == IMS_NULL)
    {
        IMS_TRACE_D("No contacts in the dialog; use the default contact address...", 0, 0, 0);

        pContact = m_pRegStateTracker->GetContactAddressForOutgoingMessage();

        if (pContact == IMS_NULL)
        {
            pContact = &objContact;
        }
    }

    if (pContact->Equals(objRequestUri))
    {
        return IMS_TRUE;
    }

    if (objContact.Equals(objRequestUri))
    {
        IMS_TRACE_D("Request-URI matches the contact address for REGISTER", 0, 0, 0);
        return IMS_TRUE;
    }

    // Checks if the contact address for outgoing message matches or not.
    pContact = m_pRegStateTracker->GetContactAddressForOutgoingMessage();

    if ((pContact != IMS_NULL) && pContact->Equals(objRequestUri))
    {
        IMS_TRACE_D("Request-URI matches the contact address for outgoing message", 0, 0, 0);
        return IMS_TRUE;
    }

    IMS_TRACE_D("Request-URI in mid-dialog request is not matched", 0, 0, 0);

    return IMS_FALSE;
}

PRIVATE GLOBAL ISipClientConnection* RegSubscription::CreateConnection(IN RegSubscription* pRegSub)
{
    AString strAor = pRegSub->GetUserAor()->ToString();
    const RegStateTracker* pStateTracker = pRegSub->m_pRegStateTracker.Get();
    ISipClientConnection* piScc = IMS_NULL;
    IMS_BOOL bOverwriteTarget = IMS_FALSE;
    IMS_SINT32 nTransportExt = pStateTracker->GetTransportExt();

    IMS_TRACE_D("CreateConnection - To (%s), Method (SUBSCRIBE)",
            SipDebug::GetUri1(strAor).GetStr(), 0, 0);

    if (pRegSub->GetState() == STATE_ACTIVE)
    {
        ISipDialog* piDialog = pRegSub->GetDialog();

        piScc = piDialog->GetNewClientConnection(SipMethod::ToName(SipMethod::SUBSCRIBE));

        // AUTH_SIP_DIGEST {
        // Sets an authentication challenge & credentials
        pRegSub->SetChallengeNCredentials(piScc);
        // }
    }
    else
    {
        if (!pRegSub->GetUserAor()->IsSchemeSip() && !pRegSub->GetUserAor()->IsSchemeSips())
        {
            bOverwriteTarget = IMS_TRUE;
            strAor = ImsIdentity::GetAnonymousUserId();
        }

        piScc = DYNAMIC_CAST(ISipClientConnection*, Connector::Open(strAor));

        if (pRegSub->IsFeatureEnabled(
                    IRegSubscription::FEATURE_USE_TCP_TRANSPORT_ON_INITIAL_SUBSCRIPTION))
        {
            nTransportExt |= Sip::TRANSPORT_EXT_TCP;
            IMS_TRACE_D("RegSubscription :: TCP transport protocol will be used", 0, 0, 0);
        }
    }

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a SIP client connection failed", 0, 0, 0);
        return IMS_NULL;
    }

    // In case of a multiple PDConnections & SIPConnectionNotifiers,
    // we needs to have a policy of which network will have a preference between the connections.

    // MULTI_REG_SIP_PROFILE
    RcPtr<SipProfile> pSipProfile = SipProfile::Create(
            pStateTracker->GetSipProfile(), pStateTracker->IsEmergencyRegistration());
    piScc->SetSipProfile(pSipProfile.Get());

    // Sets the transport tuples
    // RFC5626_FLOW_CONTROL
    // MULTI_REG_TRANSPORT
    piScc->SetTransportTuple(pStateTracker->GetIpAddress(), pStateTracker->GetPortUs(),
            pStateTracker->GetPortUc(), pStateTracker->GetPortFlowControl(), nTransportExt);

    // Sets the SIP transaction timers
    if (pRegSub->m_pSipTimerValues != IMS_NULL)
    {
        piScc->SetTransactionTimerValues(*(pRegSub->m_pSipTimerValues));
    }
    else
    {
        piScc->SetTransactionTimerValues(SipTimerValuesHelper::GetValues(
                pRegSub->GetSlotId(), pStateTracker->GetSipProfile()));
    }

    if (pRegSub->GetState() == STATE_ACTIVE)
    {
        // IMPLICIT_ROUTING_FOR_MID_DIALOG
        const AStringArray& objServiceRoutes = pStateTracker->GetServiceRoutes();

        if (!objServiceRoutes.IsEmpty())
        {
            piScc->SetImplicitRouteHeader(objServiceRoutes.GetElementAt(0));
        }
    }
    else
    {
        if (piScc->InitRequest(SipMethod::ToName(SipMethod::SUBSCRIBE), IMS_NULL) != IMS_SUCCESS)
        {
            piScc->Close();
            return IMS_NULL;
        }

        // Re-write the Request-URI & To header if the URI is not "sip" or "sips" URI
        if (bOverwriteTarget)
        {
            strAor = pRegSub->GetUserAor()->ToString();

            if (piScc->SetRequestUri(strAor) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting Request-URI failed", 0, 0, 0);

                piScc->Close();
                return IMS_NULL;
            }

            // Sets To header field
            if (piScc->GetMessage()->SetHeader(ISipHeader::TO, strAor) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting To header failed", 0, 0, 0);

                piScc->Close();
                return IMS_NULL;
            }
        }

        // From
        if (piScc->GetMessage()->SetHeader(ISipHeader::FROM, strAor) != IMS_SUCCESS)
        {
            piScc->Close();
            return IMS_NULL;
        }
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Sets Accept header field
    AString strAccept;

    strAccept.Append(MEDIA_TYPE);
    strAccept.Append('/');
    strAccept.Append(MEDIA_SUB_TYPE);

    if (piSipMsg->SetHeader(ISipHeader::ACCEPT, strAccept) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Adding Accept header failed", 0, 0, 0);

        piScc->Close();
        return IMS_NULL;
    }

    // Sets P-Preferred-Identity header fields
    // hwangoo.park, 130514, do not add P-Preferred-Identity header
    if (SipConfigProxy::IsPPreferredIdInRegSubRequired(
                pRegSub->GetSlotId(), pStateTracker->GetSipProfile()))
    {
        if (piSipMsg->SetHeader(ISipHeader::P_PREFERRED_IDENTITY, strAor) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Adding P-Preferred-Identity header failed", 0, 0, 0);

            piScc->Close();
            return IMS_NULL;
        }
    }

    // Add an allowed/supported methods for this UA
    const AStringArray& objMethods = SipConfigProxy::GetRegAllowMethods(
            pRegSub->GetSlotId(), pStateTracker->GetSipProfile());

    for (IMS_SINT32 i = 0; i < objMethods.GetCount(); ++i)
    {
        if (piSipMsg->AddHeader(ISipHeader::ALLOW, objMethods.GetElementAt(i)) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting Allow header failed", 0, 0, 0);

            piScc->Close();
            return IMS_NULL;
        }
    }

    // IPSEC {
    // RFC 3329 - SIP Security Agreement :: Sets Security-Client / Security-Verify headers
    // Do not add Security-Client headers
    const AStringArray& objSecurityVerifys = pStateTracker->GetSecurityVerifys();

    for (IMS_SINT32 i = 0; i < objSecurityVerifys.GetCount(); ++i)
    {
        const AString& strHeader = objSecurityVerifys.GetElementAt(i);

        if (piSipMsg->AddHeader(ISipHeader::SECURITY_VERIFY, strHeader) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Adding Security-Verify header failed", 0, 0, 0);

            piScc->Close();
            return IMS_NULL;
        }
    }

    if (!objSecurityVerifys.IsEmpty())
    {
        piSipMsg->AddHeader(ISipHeader::REQUIRE, Sip::STR_SEC_AGREE);
        piSipMsg->AddHeader(ISipHeader::PROXY_REQUIRE, Sip::STR_SEC_AGREE);
    }
    // }

    // Set P-Access-Network-Info header
    PAccessNetworkInfoHeader::SetHeader(
            pRegSub->GetSlotId(), pStateTracker->GetIpAddress(), piScc->GetSipProfile(), piSipMsg);

    return piScc;
}

PRIVATE GLOBAL IMS_UINT16 RegSubscription::GetReasonParameter(IN const ISipMessage* piSipMsg)
{
    AString strSubState = piSipMsg->GetHeader(ISipHeader::SUBSCRIPTION_STATE);
    ISipHeader* piHeader =
            SipParsingHelper::CreateHeader(ISipHeader::SUBSCRIPTION_STATE, strSubState);
    IMS_UINT16 nReason = IRegSubscription::PARAM_REASON_NONE;

    if (piHeader != IMS_NULL)
    {
        nReason = SubState::ExtractReasonParameter(piHeader);
        nReason = GetReasonFromSubStateReason(nReason);
        piHeader->Destroy();
    }

    return nReason;
}

PRIVATE GLOBAL IMS_UINT16 RegSubscription::GetReasonFromSubStateReason(IN IMS_UINT16 nReason)
{
    switch (nReason)
    {
        case SubState::REASON_NORESOURCE:
            return IRegSubscription::PARAM_REASON_NORESOURCE;
        case SubState::REASON_DEACTIVATED:
            return IRegSubscription::PARAM_REASON_DEACTIVATED;
        case SubState::REASON_PROBATION:
            return IRegSubscription::PARAM_REASON_PROBATION;
        case SubState::REASON_REJECTED:
            return IRegSubscription::PARAM_REASON_REJECTED;
        case SubState::REASON_TIMEOUT:
            return IRegSubscription::PARAM_REASON_TIMEOUT;
        case SubState::REASON_GIVEUP:
            return IRegSubscription::PARAM_REASON_GIVEUP;
        default:
            return nReason;
    }
}

PRIVATE GLOBAL const IMS_CHAR* RegSubscription::StateToString(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case STATE_INACTIVE:
            return "STATE_INACTIVE";
        case STATE_PENDING:
            return "STATE_PENDING";
        case STATE_ACTIVE:
            return "STATE_ACTIVE";
        default:
            return "__INVALID__";
    }
}
