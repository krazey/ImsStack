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

#include "Capabilities.h"
#include "CoreContext.h"
#include "IOnSubscriptionListener.h"
#include "ISipDialog.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipServerConnection.h"
#include "Message.h"
#include "Service.h"
#include "SipConfigProxy.h"
#include "SipDebug.h"
#include "SipStatusCode.h"
#include "SubscriberRefreshHelper.h"
#include "SubscriberState.h"
#include "Subscription.h"
#include "base/IRefreshListener.h"
#include "base/Ims.h"
#include "util/CallerPreferenceManager.h"
#include "util/DialogMethodManager.h"
#include "util/ForkedDialogMethodManager.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
Subscription::Subscription(IN Service* pService, IN const AString& strEvent,
        IN IMS_BOOL bImplicitRoutingRequired /*= IMS_FALSE*/) :
        ServiceMethod(pService),
        m_nState(STATE_INACTIVE),
        m_strEvent(strEvent),
        m_nPendingOperation(SubState::NO_OPERATION),
        m_piListener(IMS_NULL),
        m_pSubState(IMS_NULL),
        m_piRefreshListener(IMS_NULL),
        m_pRefreshHelper(IMS_NULL),
        m_bSubscriptionInOtherDialog(IMS_FALSE),
        m_bImplicitRoutingRequired(bImplicitRoutingRequired)
{
}

PUBLIC VIRTUAL Subscription::~Subscription()
{
    CleanupOnDestroy();

    while (!m_objNotifyMessages.IsEmpty())
    {
        Message* pMessage = m_objNotifyMessages.GetAt(0);

        if (pMessage != IMS_NULL)
        {
            delete pMessage;
        }

        m_objNotifyMessages.RemoveAt(0);
    }

    while (!m_objForkedSubscriptions.IsEmpty())
    {
        Subscription* pSubscription = m_objForkedSubscriptions.GetAt(0);

        if (pSubscription != IMS_NULL)
        {
            pSubscription->Destroy();
        }

        m_objForkedSubscriptions.RemoveAt(0);
    }

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
}

PUBLIC VIRTUAL void Subscription::Destroy()
{
    CleanupOnDestroy();
    ServiceMethod::Destroy();
}

PUBLIC VIRTUAL void Subscription::SetMessageMediator(IN IMessageMediator* piMediator)
{
    Method::SetMessageMediator(piMediator);

    if (m_pRefreshHelper != IMS_NULL)
    {
        m_pRefreshHelper->SetMessageMediator(piMediator);
    }
}

PUBLIC
IMS_RESULT Subscription::Poll()
{
    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
#if 0
    if (!IsServiceOpen())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }
#endif

    if (GetState() != STATE_INACTIVE)
    {
        IMS_TRACE_E(0, "To subscribe(poll) an event, the state MUST be an INACTIVE state", 0, 0, 0);
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    // if the state is in ACTIVE and refresh is started by the subscription,
    // keep the request and after refresh is completed, try to send a SUBSCRIBE request...

    ISipClientConnection* piScc = CreateConnection(SipMethod(SipMethod::SUBSCRIBE));

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Event header
    piSipMsg->SetHeader(ISipHeader::EVENT, GetEvent());

    // Do something for SubscriberRefreshHelper ???
    // Accept, Allow-Events

    if (!piSipMsg->IsHeaderPresent(ISipHeader::EXPIRES_ANY))
    {
        piSipMsg->SetHeader(ISipHeader::EXPIRES_ANY, "0");
    }

    // Try to send a SUBSCRIBE request to the network
    if (!SendNUpdateRequest(IMessage::SUBSCRIPTION_POLL, piScc))
    {
        piScc->Close();
        return IMS_FAILURE;
    }

    m_pSubState->SetOperation(SubState::OPERATION_FETCH);

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

PUBLIC
IMS_RESULT Subscription::Subscribe()
{
    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
#if 0
    if (!IsServiceOpen())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }
#endif

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

    ISipDialog* piDialog = GetDialog();
    SipMethod objMethod(SipMethod::SUBSCRIBE);
    ISipClientConnection* piScc;

    if (piDialog == IMS_NULL)
    {
        piScc = CreateConnection(objMethod);
    }
    else
    {
        piScc = CreateConnectionL(piDialog, objMethod);
    }

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Event header
    piSipMsg->SetHeader(ISipHeader::EVENT, GetEvent());

    // Do something for SubscriberRefreshHelper ???
    // Accept, Allow-Events

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

    // Try to send a SUBSCRIBE request to the network
    if (!SendNUpdateRequest(IMessage::SUBSCRIPTION_SUBSCRIBE, piScc))
    {
        piScc->Close();
        return IMS_FAILURE;
    }

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

    // Update the dialog state for SUBSCRIBE-created dialog (to handle early NOTIFY request)
    if ((GetState() == STATE_INACTIVE) && m_bSubscriptionInOtherDialog)
    {
        CheckNCreateDialog(piScc, IMS_TRUE);
    }

    SetState(STATE_PENDING);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Subscription::Unsubscribe()
{
    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
#if 0
    if (!IsServiceOpen())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }
#endif

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

    SipMethod objMethod(SipMethod::SUBSCRIBE);
    ISipClientConnection* piScc = CreateConnectionL(GetDialog(), objMethod);

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection inside of a dialog failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Event header
    piSipMsg->SetHeader(ISipHeader::EVENT, GetEvent());

    // Do something for SubscriberRefreshHelper ???
    // Accept, Allow-Events

    // Expires header (overwrites the header field if present)
    piSipMsg->SetHeader(ISipHeader::EXPIRES_ANY, "0");

    // Try to send a SUBSCRIBE request to the network
    if (!SendNUpdateRequest(IMessage::SUBSCRIPTION_UNSUBSCRIBE, piScc))
    {
        piScc->Close();
        return IMS_FAILURE;
    }

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

PUBLIC
void Subscription::SetImplicitRoutingRequired(IN IMS_BOOL bFlag)
{
    m_bImplicitRoutingRequired = bFlag;

    // FIXME: If the routing address needs to be provisioned by the application,
    // please add a second argument for it.
}

PUBLIC
void Subscription::SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
        IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt)
{
    if (GetState() != STATE_INACTIVE)
    {
        IMS_TRACE_E(0, "Refresh policy can't be applied in the state (%s)",
                StateToString(GetState()), 0, 0);
        return;
    }

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

PROTECTED VIRTUAL IMS_BOOL Subscription::DispatchMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case AMSG_SUBSCRIPTION_NOTIFY_RECEIVED:
            if (!m_objNotifyMessages.IsEmpty())
            {
                IMS_BOOL bDestroyNotify = IMS_TRUE;
                Message* pMessage = m_objNotifyMessages.GetAt(0);

                if (m_piListener != IMS_NULL)
                {
                    m_piListener->OnSubscription_NotifyReceived(this, pMessage, bDestroyNotify);
                }

                m_objNotifyMessages.RemoveAt(0);

                if (bDestroyNotify)
                {
                    delete pMessage;
                }
            }
            return IMS_TRUE;
        case AMSG_SUBSCRIPTION_STARTED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnSubscription_Started(this);
            }
            return IMS_TRUE;
        case AMSG_SUBSCRIPTION_START_FAILED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnSubscription_StartFailed(this);
            }
            return IMS_TRUE;
        case AMSG_SUBSCRIPTION_TERMINATED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnSubscription_Terminated(this);
            }
            return IMS_TRUE;
        case AMSG_SUBSCRIPTION_FORKED_NOTIFY_RECEIVED:
            if (!m_objForkedSubscriptions.IsEmpty())
            {
                Subscription* pSubscription = m_objForkedSubscriptions.GetAt(0);

                m_objForkedSubscriptions.RemoveAt(0);

                if (m_piListener != IMS_NULL)
                {
                    if (!m_piListener->OnSubscription_ForkedNotifyReceived(this, pSubscription))
                    {
                        pSubscription->Destroy();
                    }
                }
                else
                {
                    pSubscription->Destroy();
                }
            }
            return IMS_TRUE;
        default:
            break;
    }

    return EngineActivity::DispatchMessage(objMsg);
}

PROTECTED VIRTUAL IMS_BOOL Subscription::SendRequestToChallenge(IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nOperation = m_pSubState->GetOperation();

    if (nOperation == SubState::OPERATION_IMPLICIT_REFRESH)
    {
        if (!Method::SendRequestToChallenge(piScc))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        IMS_SINT32 nServiceMethod = IMessage::SUBSCRIPTION_SUBSCRIBE;

        if (nOperation == SubState::OPERATION_REMOVE)
        {
            nServiceMethod = IMessage::SUBSCRIPTION_UNSUBSCRIBE;
        }
        else if (nOperation == SubState::OPERATION_FETCH)
        {
            nServiceMethod = IMessage::SUBSCRIPTION_POLL;
        }

        // Clear the connection to preserve the SIP connection
        ClearConnection(nServiceMethod);

        if (!SendNUpdateRequestEx(nServiceMethod, piScc, MESSAGE_CLASS_RESUBMIT))
        {
            // Revert the SIP connection
            UpdateConnection(nServiceMethod, piScc);
            return IMS_FALSE;
        }
    }

    // Update the subscription state
    if (!m_pSubState->UpdateState(piScc->GetMessage()))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void Subscription::Exception_NotifyError(IN IMS_SINT32 nErrorCode)
{
    (void)nErrorCode;

    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
#if 0
    SetState(STATE_INACTIVE);
    PostMessage(AMSG_SUBSCRIPTION_TERMINATED, 0, 0);

    pSubState->Clear();
    DestroyDialog();
#endif
}

PROTECTED VIRTUAL IMS_BOOL Subscription::InitInstance()
{
    if (m_pSubState == IMS_NULL)
    {
        m_pSubState = new SubscriberState();

        if (m_pSubState == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a subscriber state failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (!m_pSubState->CreateEventPackage(m_strEvent))
        {
            IMS_TRACE_E(0, "Creating an event package for an event (%s) failed",
                    m_strEvent.GetStr(), 0, 0);
            return IMS_FALSE;
        }

        m_pSubState->SetConfiguration(SubState::CONFIG_USE_INITIAL_EXPIRES_ON_NO_EXPIRES_IN_200_OK);
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
    ForkedDialogMethodManager::GetInstance()->AddMethod(GetName(), this);
    // CALLER_PREFERENCE_MANAGER
    CoreContext::GetInstance()->GetCallerPreferenceManager()->CreatePreferenceWrapper(
            GetName(), AString::ConstNull());
    GetService()->RegisterMethod(this);

    if (GetDialog() != IMS_NULL)
    {
        m_bSubscriptionInOtherDialog = IMS_TRUE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void Subscription::NotifySipResponse(IN ISipClientConnection* piScc)
{
    ISipMessage* piSipMsg = piScc->GetMessage();
    const SipMethod& objMethod = piSipMsg->GetMethod();

    IMS_TRACE_I("The response is received in the %s", StateToString(GetState()), 0, 0);

    if (!objMethod.Equals(SipMethod::SUBSCRIBE))
    {
        piScc->Close();
        return;
    }

    IMS_SINT32 nOperation = m_pSubState->GetOperation();

    // Update the subscription state
    if (!m_pSubState->UpdateState(piSipMsg))
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_SUBSCRIPTION_START_FAILED, 0, 0);

        CloseConnection();

        if (nOperation == SubState::OPERATION_CREATE)
        {
            m_pSubState->Clear();

            if (!m_bSubscriptionInOtherDialog)
            {
                DestroyDialog();
            }
        }
        return;
    }

    // Handle the response according to the SIP method.
    IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

    UpdateResponse(piScc);

    // Handle the response to SUBSCRIBE request ...
    if (SipStatusCode::Is1XX(nStatusCode))
    {
        // Do nothing ...
        return;
    }
    else if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        // AUTH_SIP_DIGEST {
        // In case of other method except for REGISTER,
        // the UE only supports the authentication algorithm, MD5
        if (RespondToChallenge(piScc))
        {
            return;
        }
        // }
    }

    // Start or re-start a subscription refresh timer
    m_pRefreshHelper->UpdateOnMessageReceived(piScc);

    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        if (GetState() != STATE_PENDING)
        {
            CheckNCreateDialog(piScc, m_bSubscriptionInOtherDialog);
        }
        else
        {
            // RACE_CONDITION: NOTIFY (w/ terminated) & 200 OK to SUBSCRIBE
            CheckNCreateDialog(piScc, IMS_FALSE, IMS_TRUE);
        }

        // CALLER_PREFERENCE_MANAGER
        if ((nOperation == SubState::OPERATION_CREATE) ||
                (nOperation == SubState::OPERATION_REFRESH))
        {
            if (nOperation == SubState::OPERATION_CREATE)
            {
                ISipDialog* piDialog = GetDialog();

                if ((piDialog != IMS_NULL) && (piDialog->GetState() == ISipDialog::STATE_CONFIRMED))
                {
                    CoreContext::GetInstance()->GetCallerPreferenceManager()->UpdateDialogId(
                            GetName(), piDialog->GetDialogId());
                }
            }

            IMessage* piMessage = GetPreviousRequest(IMessage::SUBSCRIPTION_SUBSCRIBE);

            if (piMessage != IMS_NULL)
            {
                ISipMessage* piPreviousSIPMsg = piMessage->GetMessage();

                if (piPreviousSIPMsg != IMS_NULL)
                {
                    CoreContext::GetInstance()->GetCallerPreferenceManager()->UpdateAcceptContacts(
                            GetName(), piPreviousSIPMsg->GetHeaders(ISipHeader::ACCEPT_CONTACT));
                }
            }
        }

        SetState(STATE_ACTIVE);
        PostMessage(AMSG_SUBSCRIPTION_STARTED, 0, 0);
    }
    else
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_SUBSCRIPTION_START_FAILED, 0, 0);
    }

    CloseConnection();

    if ((nOperation != SubState::OPERATION_CREATE) && (nOperation != SubState::OPERATION_FETCH))
    {
        CheckDialogNCallListener();
    }

    // Initialize all the subscriber's state at this moment,
    // because the application MAY use this subscription
    // to add/fetch/refresh/remove the event state.
    if (GetState() == STATE_INACTIVE)
    {
        m_pSubState->Clear();

        if (!m_bSubscriptionInOtherDialog)
        {
            DestroyDialog();
        }
    }

    // 4 check "nPendingOperation" member field
}

PROTECTED VIRTUAL void Subscription::NotifySipError(
        IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    const SipMethod& objMethod = piSc->GetMethod();

    (void)nCode;
    (void)strMessage;

    if (!objMethod.Equals(SipMethod::SUBSCRIBE))
    {
        piSc->Close();
        return;
    }

    IMS_SINT32 nOperation = m_pSubState->GetOperation();

    if ((nOperation != SubState::NO_OPERATION) &&
            (nOperation != SubState::OPERATION_IMPLICIT_REFRESH))
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_SUBSCRIPTION_START_FAILED, 0, 0);
    }

    CloseConnection();

    // Initialize all the subscriber's state at this moment,
    // because the application MAY use this subscription
    // to add/fetch/refresh/remove the event state.
    if (GetState() == STATE_INACTIVE)
    {
        m_pSubState->Clear();
        DestroyDialog();
    }
}

PROTECTED VIRTUAL IMS_BOOL Subscription::Dialog_Compare(IN ISipServerConnection* piSsc) const
{
    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
#if 0
    if (!IsServiceOpen())
    {
        return IMS_FALSE;
    }
#endif

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
            ISipClientConnection* piScc = IMS_NULL;

            IMS_TRACE_I("Checks if the early NOTIFY is received or not ...", 0, 0, 0);

            if (nOperation == SubState::OPERATION_CREATE)
            {
                piScc = GetClientConnection(IMessage::SUBSCRIPTION_SUBSCRIBE);
            }
            else if (nOperation == SubState::OPERATION_FETCH)
            {
                piScc = GetClientConnection(IMessage::SUBSCRIPTION_POLL);
            }

            if (piScc != IMS_NULL)
            {
                piDialog = piScc->GetDialog();
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

PROTECTED VIRTUAL IMS_BOOL Subscription::Dialog_NotifyRequest(IN ISipServerConnection* piSsc)
{
    const SipMethod& objMethod = piSsc->GetMethod();

    if (!objMethod.Equals(SipMethod::NOTIFY))
    {
        // Handling of OPTIONS request
        if (objMethod.Equals(SipMethod::OPTIONS))
        {
            if (Capabilities::HandleOptionsRequestWithinDialog(GetService(), this, piSsc) !=
                    IMS_SUCCESS)
            {
                return IMS_FALSE;
            }

            return IMS_TRUE;
        }

        GetService()->SendResponse(piSsc, SipStatusCode::SC_405);
        piSsc->Close();
        return IMS_FALSE;
    }

    if (SipConfigProxy::IsRequestUriValidationRequiredInMidDialog(
                GetSlotId(), GetService()->GetSipProfile()))
    {
        // Checks if Request-URI is matched or not
        const AString& strRequestUri = piSsc->GetRequestUri();
        SipAddress objRequestUri(strRequestUri);

        if (!GetService()->ValidateRequestUri(objRequestUri, piSsc->GetDialog(), IMS_TRUE))
        {
            IMS_TRACE_D("Request-URI (%s) in mid-dialog request is not matched",
                    SipDebug::GetUri1(strRequestUri).GetStr(), 0, 0);

            GetService()->SendResponse(piSsc, SipStatusCode::SC_404);
            piSsc->Close();
            return IMS_FALSE;
        }
    }

    // Update the dialog info.
    CheckNCreateDialog(piSsc);

    // Update the dialog info.
    if (GetState() != STATE_ACTIVE)
    {
        CheckNCreateDialog(piSsc, m_bSubscriptionInOtherDialog);
    }
    else
    {
        CheckNCreateDialog(piSsc);
    }

    // Update the subscription state for NOTIFY request
    if (!m_pSubState->UpdateState(piSsc->GetMessage()))
    {
        GetService()->SendResponse(piSsc, SipStatusCode::SC_500);
        piSsc->Close();
        return IMS_FALSE;
    }

    Message* pMessage =
            Message::CreateReceivedMessage(GetService()->GetAppConfig(), piSsc->GetMessage());

    if (pMessage == IMS_NULL)
    {
        // Internal error ... ???
        if (GetService()->SendResponse(piSsc, SipStatusCode::SC_500))
        {
            m_pSubState->UpdateState(piSsc->GetMessage());
            m_pRefreshHelper->UpdateOnMessageSent(piSsc);
        }

        piSsc->Close();
        return IMS_FALSE;
    }

    // Send a 2xx response to NOTIFY request immediately
    if (GetService()->SendResponse(piSsc, SipStatusCode::SC_200) == IMS_FALSE)
    {
        delete pMessage;
        piSsc->Close();

        IMS_TRACE_E(0, "Creating & sending the response to NOTIFY request failed", 0, 0, 0);

        return IMS_FALSE;
    }

    // Update the subscription state for NOTIFY response
    if (!m_pSubState->UpdateState(piSsc->GetMessage()))
    {
        delete pMessage;
        piSsc->Close();

        return IMS_FALSE;
    }

    // Update the subscription refresh timer
    m_pRefreshHelper->UpdateOnMessageSent(piSsc);

    if (!m_objNotifyMessages.Append(pMessage))
    {
        delete pMessage;

        IMS_TRACE_E(0, "Queueing NOTIFY message failed", 0, 0, 0);
    }

    piSsc->Close();

    PostMessage(AMSG_SUBSCRIPTION_NOTIFY_RECEIVED, 0, 0);

    if (m_pSubState->GetState() == SubState::STATE_TERMINATED)
    {
        PostMessage(AMSG_SUBSCRIPTION_TERMINATED, 0, 0);
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL Subscription::ForkedDialog_Compare(IN ISipDialog* piOrigDialog) const
{
    if (!IsServiceOpen())
    {
        return IMS_FALSE;
    }

    ISipDialog* piDialog = GetDialog();

    if (piDialog == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!piDialog->Equals(piOrigDialog))
    {
        return IMS_FALSE;
    }

    // 4 check the flag whether the forked request is allowed to this subscription (event package)

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL Subscription::ForkedDialog_NotifyRequest(IN ISipServerConnection* piSsc)
{
    Subscription* pSubscription = new Subscription(GetService(), m_strEvent);

    if (pSubscription == IMS_NULL)
    {
        GetService()->SendResponse(piSsc, SipStatusCode::SC_500);
        piSsc->Close();

        return IMS_FALSE;
    }

    if (!pSubscription->InitMethod(this, IsMobileOriginated()))
    {
        delete pSubscription;

        GetService()->SendResponse(piSsc, SipStatusCode::SC_500);
        piSsc->Close();
        return IMS_FALSE;
    }

    // Update the dialog info. enforcelly
    pSubscription->CheckNCreateDialog(piSsc, IMS_TRUE);
    pSubscription->SetState(STATE_ACTIVE);

    // Update the subscription state from the initial SUBSCRIBE message
    pSubscription->m_pSubState->SetOperation(SubState::OPERATION_CREATE);
    pSubscription->m_pSubState->UpdateState(m_pSubState->GetInitialMessage());
    pSubscription->m_pSubState->SetOperation(SubState::NO_OPERATION);

    if (!m_objForkedSubscriptions.Append(pSubscription))
    {
        delete pSubscription;

        GetService()->SendResponse(piSsc, SipStatusCode::SC_500);
        piSsc->Close();
        return IMS_FALSE;
    }

    PostMessage(AMSG_SUBSCRIPTION_FORKED_NOTIFY_RECEIVED, 0, 0);

    if (!pSubscription->Dialog_NotifyRequest(piSsc))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void Subscription::Refreshable_RefreshCompleted(
        IN ISipClientConnection* piScc, IN IMS_SINT32 nCode /*= 0*/)
{
    IMS_TRACE_I("___ SUBSCRIPTION REFRESH COMPLETED ... Code (%d)", nCode, 0, 0);

    if (m_piRefreshListener != IMS_NULL)
    {
        m_piRefreshListener->Refresh_NotifyCompleted(piScc);
    }

    // In case, the subscription refresh request is successfully done.
    if (nCode == 0)
    {
        if (!m_pSubState->UpdateState(piScc->GetMessage()))
        {
            // Set the operation
            m_pSubState->SetOperation(SubState::NO_OPERATION);

            CheckDialogNCallListener();
            return;
        }

        // AUTH_SIP_DIGEST {
        IMS_SINT32 nStatusCode = piScc->GetStatusCode();

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

        CheckDialogNCallListener();
    }
    else if ((nCode == SipStatusCode::SC_408) || (nCode == RefreshHelper::TRANSACTION_TIMEOUT))
    {
        Refreshable_RefreshTerminated();
    }

    // Set the operation
    m_pSubState->SetOperation(SubState::NO_OPERATION);
}

PROTECTED VIRTUAL IMS_BOOL Subscription::Refreshable_RefreshStarted()
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    IMS_SINT32 nState = GetState();

    IMS_TRACE_I("___ SUBSCRIPTION REFRESH STARTED ... in the %s", StateToString(nState), 0, 0);

    if (m_piRefreshListener != IMS_NULL)
    {
        m_piRefreshListener->Refresh_NotifyTimerExpired(bDoImplicitRefresh);
    }

    if (bDoImplicitRefresh && (nState == STATE_ACTIVE))
    {
        // Set the operation
        m_pSubState->SetOperation(SubState::OPERATION_IMPLICIT_REFRESH);

        // Send a refresh request : SUBSCRIBE
        SipMethod objMethod(SipMethod::SUBSCRIBE);
        ISipClientConnection* piScc = GetService()->CreateConnection(GetDialog(), objMethod);

        if (piScc == IMS_NULL)
        {
            m_pSubState->SetOperation(SubState::NO_OPERATION);

            IMS_TRACE_E(
                    0, "Creating a new SIP connection for a subscription refresh failed", 0, 0, 0);
            return IMS_FALSE;
        }

        ISipMessage* piSipMsg = piScc->GetMessage();
        ISipMessage* piInitialSipMsg = m_pSubState->GetInitialMessage();

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

        // IMPLICIT_ROUTING_FOR_MID_DIALOG
        if (m_bImplicitRoutingRequired)
        {
            const AStringArray& objServiceRoutes = GetService()->GetServiceRoutes();

            if (!objServiceRoutes.IsEmpty())
            {
                piScc->SetImplicitRouteHeader(objServiceRoutes.GetElementAt(0));
            }
        }

        if (m_pRefreshHelper->SendRefreshRequest(piScc) != IMS_SUCCESS)
        {
            m_pSubState->SetOperation(SubState::NO_OPERATION);
            piScc->Close();

            IMS_TRACE_E(0, "Sending a subscription refresh request failed", 0, 0, 0);
            return IMS_FALSE;
        }

        // Update the subscription state
        m_pSubState->UpdateState(piSipMsg);

        return IMS_TRUE;
    }

    return (bDoImplicitRefresh == IMS_TRUE) ? IMS_FALSE : IMS_TRUE;
}

PROTECTED VIRTUAL void Subscription::Refreshable_RefreshTerminated()
{
    IMS_TRACE_D("_____ SUBSCRIPTION REFRESH TERMINATED ...", 0, 0, 0);

    if (m_piRefreshListener != IMS_NULL)
    {
        m_piRefreshListener->Refresh_NotifyTerminated();
    }

    if (GetState() == STATE_ACTIVE)
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_SUBSCRIPTION_TERMINATED, 0, 0);
    }

    if (GetState() == STATE_INACTIVE)
    {
        m_pSubState->Clear();
        DestroyDialog();
    }
}

PRIVATE
void Subscription::CheckDialogNCallListener()
{
    // Check if the dialog is terminated or not
    ISipDialog* piDialog = GetDialog();

    if (piDialog != IMS_NULL)
    {
        if (piDialog->GetState() == ISipDialog::STATE_TERMINATED)
        {
            PostMessage(AMSG_SUBSCRIPTION_TERMINATED, 0, 0);
        }
    }
    else
    {
        PostMessage(AMSG_SUBSCRIPTION_TERMINATED, 0, 0);
    }
}

PRIVATE
void Subscription::CleanupOnDestroy()
{
    if (m_pRefreshHelper != IMS_NULL)
    {
        if (m_pRefreshHelper->IsRequestPending())
        {
            m_pRefreshHelper->AbortConnection();
        }
    }

    DialogMethodManager::GetInstance()->RemoveMethod(GetName());

    ForkedDialogMethodManager::GetInstance()->RemoveMethod(GetName());

    // CALLER_PREFERENCE_MANAGER
    CoreContext::GetInstance()->GetCallerPreferenceManager()->DestroyPreferenceWrapper(GetName());

    // Clean up the resources
    GetService()->DeregisterMethod(this);
}

PRIVATE
void Subscription::CloseConnection()
{
    switch (m_pSubState->GetOperation())
    {
        case SubState::OPERATION_CREATE:  // FALL-THROUGH
        case SubState::OPERATION_REFRESH:
            ServiceMethod::CloseConnection(IMessage::SUBSCRIPTION_SUBSCRIBE);
            break;
        case SubState::OPERATION_FETCH:
            ServiceMethod::CloseConnection(IMessage::SUBSCRIPTION_POLL);
            break;
        case SubState::OPERATION_REMOVE:
            ServiceMethod::CloseConnection(IMessage::SUBSCRIPTION_UNSUBSCRIBE);
            break;
        default:
            // Do nothing ...
            break;
    }

    m_pSubState->SetOperation(SubState::NO_OPERATION);
}

PRIVATE
ISipClientConnection* Subscription::CreateConnectionL(
        IN ISipDialog* piDialog, IN const SipMethod& objMethod)
{
    ISipClientConnection* piScc = CreateConnection(piDialog, objMethod);

    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    if (m_bImplicitRoutingRequired && (piScc != IMS_NULL))
    {
        const AStringArray& objServiceRoutes = GetService()->GetServiceRoutes();

        if (!objServiceRoutes.IsEmpty())
        {
            piScc->SetImplicitRouteHeader(objServiceRoutes.GetElementAt(0));
        }
    }

    return piScc;
}

PRIVATE
void Subscription::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("Subscription :: %s to %s", StateToString(m_nState), StateToString(nState), 0);

    m_nState = nState;
}

PRIVATE
void Subscription::UpdateResponse(IN ISipClientConnection* piScc)
{
    switch (m_pSubState->GetOperation())
    {
        case SubState::OPERATION_CREATE:  // FALL-THROUGH
        case SubState::OPERATION_REFRESH:
            UpdateResponseOnReceived(IMessage::SUBSCRIPTION_SUBSCRIBE, piScc);
            break;
        case SubState::OPERATION_FETCH:
            UpdateResponseOnReceived(IMessage::SUBSCRIPTION_POLL, piScc);
            break;
        case SubState::OPERATION_REMOVE:
            UpdateResponseOnReceived(IMessage::SUBSCRIPTION_UNSUBSCRIBE, piScc);
            break;
        default:
            // Do nothing ...
            break;
    }
}

const IMS_CHAR* Subscription::StateToString(IN IMS_SINT32 nState)
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
