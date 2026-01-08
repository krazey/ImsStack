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

#include "IOnPublicationListener.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "PubState.h"
#include "Publication.h"
#include "PublicationRefreshHelper.h"
#include "Service.h"
#include "SipStatusCode.h"
#include "base/IRefreshListener.h"
#include "base/Ims.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
Publication::Publication(IN Service* pService, IN const AString& strEvent,
        IN IMS_BOOL bImplicitRoutingRequired /*= IMS_TRUE*/) :
        ServiceMethod(pService),
        m_nState(STATE_INACTIVE),
        m_strEvent(strEvent),
        m_piListener(IMS_NULL),
        m_pPubState(IMS_NULL),
        m_piRefreshListener(IMS_NULL),
        m_pRefreshHelper(IMS_NULL),
        m_bImplicitRoutingRequired(bImplicitRoutingRequired)
{
}

PUBLIC VIRTUAL Publication::~Publication()
{
    if (m_pRefreshHelper != IMS_NULL)
    {
        delete m_pRefreshHelper;
        m_pRefreshHelper = IMS_NULL;
    }

    if (m_pPubState != IMS_NULL)
    {
        delete m_pPubState;
        m_pPubState = IMS_NULL;
    }
}

PUBLIC VIRTUAL void Publication::Destroy()
{
    if (m_pRefreshHelper != IMS_NULL)
    {
        if (m_pRefreshHelper->IsRequestPending())
        {
            m_pRefreshHelper->AbortConnection();
        }
    }

    ServiceMethod::Destroy();
}

PUBLIC VIRTUAL void Publication::SetMessageMediator(IN IMessageMediator* piMediator)
{
    Method::SetMessageMediator(piMediator);

    if (m_pRefreshHelper != IMS_NULL)
    {
        m_pRefreshHelper->SetMessageMediator(piMediator);
    }
}

PUBLIC VIRTUAL IMS_RESULT Publication::Publish(
        IN const ByteArray& objState, IN const AString& strContentType)
{
    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
    ISipDialog* piDialog = GetDialog();

    if (piDialog == IMS_NULL && !IsServiceOpen())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() == STATE_PENDING)
    {
        IMS_TRACE_E(0, "To publish an event state, the state MUST be an INACTIVE or ACTIVE state",
                0, 0, 0);
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    if ((objState.IsNULL() && !strContentType.IsNULL()) ||
            (!objState.IsNULL() && strContentType.IsNULL()))
    {
        IMS_TRACE_E(0, "One of the arguments is NULL", 0, 0, 0);
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (!strContentType.IsNULL() && !TextParser::IsValidMediaType(strContentType))
    {
        IMS_TRACE_E(0, "Invalid content type (%s)", strContentType.GetStr(), 0, 0);
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // if the state is in ACTIVE and refresh is started by the publication,
    // keep the request and after refresh is completed, try to send a PUBLISH request...
    if (m_pPubState->GetOperation() == PubState::OPERATION_REFRESH)
    {
        // 1 : save all the information ? try to send later .... ???
        // SetState(STATE_PENDING);

        // Ims::SetLastError(ImsError::NO_ERROR);
        // return IMS_FAILURE;
    }

    SipMethod objMethod(SipMethod::PUBLISH);
    ISipClientConnection* piScc;

    if (piDialog == IMS_NULL)
    {
        piScc = CreateConnection(objMethod);
    }
    else
    {
        piScc = CreateConnectionWithDialog(piDialog, objMethod);
    }

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Event header
    piSipMsg->SetHeader(ISipHeader::EVENT, GetEvent());

    // Accept, Allow-Events, Expires

    // Basically, Expires header will be automatically added according to the event package,
    // but IMS engine will not add the Expires header even though it's not present and it will
    // be controlled by the enabler.
#if 0
    if (!piSipMsg->IsHeaderPresent(ISipHeader::EXPIRES_ANY))
    {
        // Expires header ?
        AString strExpires;

        if (m_pPubState->GetEventPackage()->GetDuration() != (-1))
        {
            strExpires.Sprintf("%d", m_pPubState->GetEventPackage()->GetDuration());
        }
        else
        {
            strExpires.Sprintf("%d", m_pPubState->GetEventPackage()->GetDefaultDuration());
        }

        piSipMsg->SetHeader(ISipHeader::EXPIRES_ANY, strExpires);
    }
#endif

    // OPERATION_MODIFY
    if (m_nState == STATE_ACTIVE)
    {
        // SIP-If-Match header
        if (!piSipMsg->IsHeaderPresent(ISipHeader::SIP_IF_MATCH))
        {
            if (!m_pPubState->GetEntityTag().IsNULL())
            {
                piSipMsg->SetHeader(ISipHeader::SIP_IF_MATCH, m_pPubState->GetEntityTag());
            }
        }
    }

    // Set an additional message body part at the last position
    if (!objState.IsNULL() && !strContentType.IsNULL())
    {
        ISipMessageBodyPart* piBodyPart = piSipMsg->CreateBodyPart();

        if (piBodyPart == IMS_NULL)
        {
            piScc->Close();
            Ims::SetLastError(ImsError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        // Set headers
        piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, strContentType);

        // Set content
        piBodyPart->SetContent(objState);
    }

    // Try to send a PUBLISH request to the network
    if (!SendNUpdateRequest(IMessage::PUBLICATION_PUBLISH, piScc))
    {
        piScc->Close();
        return IMS_FAILURE;
    }

    if (GetState() == STATE_INACTIVE)
    {
        m_pPubState->SetOperation(PubState::OPERATION_CREATE);
    }
    else
    {
        m_pPubState->SetOperation(PubState::OPERATION_MODIFY);
    }

    // Update the publication state
    if (!m_pPubState->UpdateState(piSipMsg))
    {
        m_pPubState->SetOperation(PubState::NO_OPERATION);
        return IMS_FAILURE;
    }

    SetState(STATE_PENDING);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT Publication::Unpublish()
{
    // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
    ISipDialog* piDialog = GetDialog();

    if (piDialog == IMS_NULL && !IsServiceOpen())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "To unpublish an event state, the state MUST be an ACTIVE state", 0, 0, 0);
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    // If the state is in ACTIVE and refresh is started by the publication,
    // keep the request and after refresh is completed, try to send a PUBLISH request...
    if (m_pPubState->GetOperation() == PubState::OPERATION_REFRESH)
    {
        // 4 : save all the information ? try to send later .... ???
        // nPendingOperation = PubState::OPERATION_REMOVE;

        // SetState(STATE_PENDING);

        // Ims::SetLastError(ImsError::NO_ERROR);
        // return IMS_FAILURE;
    }

    SipMethod objMethod(SipMethod::PUBLISH);
    ISipClientConnection* piScc;

    if (piDialog == IMS_NULL)
    {
        piScc = CreateConnection(objMethod);
    }
    else
    {
        piScc = CreateConnectionWithDialog(piDialog, objMethod);
    }

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Event header
    piSipMsg->SetHeader(ISipHeader::EVENT, GetEvent());

    // Expires header (overwrites the header field if present)
    piSipMsg->SetHeader(ISipHeader::EXPIRES_ANY, "0");

    // SIP-If-Match header
    if (!m_pPubState->GetEntityTag().IsNULL())
    {
        piSipMsg->SetHeader(ISipHeader::SIP_IF_MATCH, m_pPubState->GetEntityTag());
    }

    // Try to send a PUBLISH request to the network
    if (!SendNUpdateRequest(IMessage::PUBLICATION_UNPUBLISH, piScc))
    {
        piScc->Close();
        return IMS_FAILURE;
    }

    m_pPubState->SetOperation(PubState::OPERATION_REMOVE);

    // Update the publication state
    if (!m_pPubState->UpdateState(piSipMsg))
    {
        m_pPubState->SetOperation(PubState::NO_OPERATION);
        return IMS_FAILURE;
    }

    SetState(STATE_PENDING);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
void Publication::SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
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

PRIVATE VIRTUAL IMS_BOOL Publication::DispatchMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case AMSG_PUBLICATION_DELIVERED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnPublication_Delivered(this);
            }
            return IMS_TRUE;
        case AMSG_PUBLICATION_DELIVERY_FAILED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnPublication_DeliveryFailed(this);
            }
            return IMS_TRUE;
        case AMSG_PUBLICATION_TERMINATED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnPublication_Terminated(this);
            }
            return IMS_TRUE;
        case AMSG_PUBLICATION_REFRESH_STARTED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnPublication_RefreshStarted(this);
            }
            return IMS_TRUE;
        case AMSG_PUBLICATION_REFRESH_COMPLETED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnPublication_RefreshCompleted(this);
            }
            return IMS_TRUE;
        default:
            break;
    }

    return EngineActivity::DispatchMessage(objMsg);
}

PRIVATE VIRTUAL IMS_BOOL Publication::SendRequestToChallenge(IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_pPubState->GetOperation() == PubState::OPERATION_REFRESH)
    {
        if (!Method::SendRequestToChallenge(piScc))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        IMS_SINT32 nServiceMethod = IMessage::PUBLICATION_PUBLISH;

        if (m_pPubState->GetOperation() == PubState::OPERATION_REMOVE)
        {
            nServiceMethod = IMessage::PUBLICATION_UNPUBLISH;
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

    // Update the publication state
    if (!m_pPubState->UpdateState(piScc->GetMessage()))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL void Publication::Exception_NotifyError(IN IMS_SINT32 nErrorCode)
{
    (void)nErrorCode;

    SetState(STATE_INACTIVE);
    PostMessage(AMSG_PUBLICATION_TERMINATED, 0, 0);

    m_pPubState->Clear();
}

PRIVATE VIRTUAL IMS_BOOL Publication::InitInstance()
{
    if (m_pPubState == IMS_NULL)
    {
        m_pPubState = new PubState();

        if (m_pPubState == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a publication state failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (!m_pPubState->CreateEventPackage(m_strEvent))
        {
            IMS_TRACE_E(0, "Creating an event package for an event (%s) failed",
                    m_strEvent.GetStr(), 0, 0);
            return IMS_FALSE;
        }
    }

    if (m_pRefreshHelper == IMS_NULL)
    {
        m_pRefreshHelper = new PublicationRefreshHelper(this, m_pPubState);

        if (m_pRefreshHelper == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a publication refresh helper failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL void Publication::NotifySipResponse(IN ISipClientConnection* piScc)
{
    const ISipMessage* piSipMsg = piScc->GetMessage();
    const SipMethod& objMethod = piSipMsg->GetMethod();

    IMS_TRACE_I("The response is received in the %s", StateToString(GetState()), 0, 0);

    if (!objMethod.Equals(SipMethod::PUBLISH))
    {
        piScc->Close();
        return;
    }

    // Update the publication state
    if (!m_pPubState->UpdateState(piSipMsg))
    {
        PostMessage(AMSG_PUBLICATION_DELIVERY_FAILED, 0, 0);

        CloseConnection();

        if (m_pPubState->GetOperation() == PubState::OPERATION_CREATE)
        {
            m_pPubState->Clear();
        }
        return;
    }

    // Handle the response according to the SIP method.
    IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

    ReceiveResponse(piScc);

    // Handle the response to PUBLISH request ...
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

    // Start or re-start a publication refresh timer
    m_pRefreshHelper->UpdateOnMessageReceived(piScc);

    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        if (m_pPubState->GetOperation() == PubState::OPERATION_REMOVE)
        {
            SetState(STATE_INACTIVE);
            PostMessage(AMSG_PUBLICATION_TERMINATED, 0, 0);
        }
        else
        {
            SetState(STATE_ACTIVE);
            PostMessage(AMSG_PUBLICATION_DELIVERED, 0, 0);
        }
    }
    else
    {
        if (GetState() != STATE_ACTIVE)
        {
            SetState(STATE_INACTIVE);
        }

        PostMessage(AMSG_PUBLICATION_DELIVERY_FAILED, 0, 0);
    }

    CloseConnection();

    if (GetState() == STATE_ACTIVE)
    {
        if (m_pPubState->IsTerminated())
        {
            SetState(STATE_INACTIVE);
            PostMessage(AMSG_PUBLICATION_TERMINATED, 0, 0);
        }
    }

    // Initialize all the EPA's state at this moment,
    // because the application MAY use this subscription to add/modify/remove the event state.
    if (GetState() == STATE_INACTIVE)
    {
        m_pPubState->Clear();
    }

    // 4 check "nPendingOperation" member field
}

PRIVATE VIRTUAL void Publication::NotifySipError(
        IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    const SipMethod& objMethod = piSc->GetMethod();

    (void)nCode;
    (void)strMessage;

    if (!objMethod.Equals(SipMethod::PUBLISH))
    {
        piSc->Close();
        return;
    }

    IMS_SINT32 nOperation = m_pPubState->GetOperation();

    if ((nOperation != PubState::NO_OPERATION) && (nOperation != PubState::OPERATION_REFRESH))
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_PUBLICATION_DELIVERY_FAILED, 0, 0);
    }

    CloseConnection();

    // Initialize all the EPA's state at this moment,
    // because the application MAY use this publication to add/modify/remove the event state.
    if (GetState() == STATE_INACTIVE)
    {
        m_pPubState->Clear();
    }
}

PRIVATE VIRTUAL void Publication::Refreshable_RefreshCompleted(
        IN ISipClientConnection* piScc, IN IMS_SINT32 nCode /*= 0*/)
{
    IMS_TRACE_I("___ PUBLICATION REFRESH COMPLETED ... Code (%d)", nCode, 0, 0);

    if (m_piRefreshListener != IMS_NULL)
    {
        m_piRefreshListener->Refresh_NotifyCompleted(piScc);
    }

    // In case, the publication refresh request is successfully done.
    if (nCode == 0)
    {
        // for handling refresh error
        ReceiveResponse(piScc);

        if (!m_pPubState->UpdateState(piScc->GetMessage()))
        {
            // Set the operation
            m_pPubState->SetOperation(PubState::NO_OPERATION);

            if (m_pPubState->IsTerminated() && !m_pRefreshHelper->IsTimerActive())
            {
                SetState(STATE_INACTIVE);
                PostMessage(AMSG_PUBLICATION_TERMINATED, 0, 0);
            }

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
        // sending refresh started message
        PostMessage(AMSG_PUBLICATION_REFRESH_COMPLETED, 0, 0);
    }
    // The subscription refresh request is timed out.
    else if (nCode == RefreshHelper::TRANSACTION_TIMEOUT)
    {
        m_pPubState->UpdateStateOnTxnTimerExpired();

        Refreshable_RefreshTerminated();
    }

    // Set the operation
    m_pPubState->SetOperation(PubState::NO_OPERATION);

    if (m_pPubState->IsTerminated() && !m_pRefreshHelper->IsTimerActive())
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_PUBLICATION_TERMINATED, 0, 0);
    }
}

PRIVATE VIRTUAL IMS_BOOL Publication::Refreshable_RefreshStarted()
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    IMS_SINT32 nState = GetState();

    IMS_TRACE_I("___ PUBLICATION REFRESH STARTED ... State(%d)", nState, 0, 0);

    if (m_piRefreshListener != IMS_NULL)
    {
        m_piRefreshListener->Refresh_NotifyTimerExpired(bDoImplicitRefresh);
    }

    if (bDoImplicitRefresh && (nState == STATE_ACTIVE))
    {
        // Set the operation
        m_pPubState->SetOperation(PubState::OPERATION_REFRESH);

        // Send a refresh request : PUBLISH
        ISipDialog* piDialog = GetDialog();
        SipMethod objMethod(SipMethod::PUBLISH);
        ISipClientConnection* piScc;

        if (piDialog == IMS_NULL)
        {
            piScc = CreateConnection(objMethod);
        }
        else
        {
            piScc = CreateConnectionWithDialog(piDialog, objMethod);
        }

        if (piScc == IMS_NULL)
        {
            m_pPubState->SetOperation(PubState::NO_OPERATION);

            IMS_TRACE_E(
                    0, "Creating a new SIP connection for a publication refresh failed", 0, 0, 0);
            return IMS_FALSE;
        }

        ISipMessage* piSipMsg = piScc->GetMessage();

        if (!m_pPubState->SetHeaders(piSipMsg))
        {
            m_pPubState->SetOperation(PubState::NO_OPERATION);
            piScc->Close();

            IMS_TRACE_E(0, "Setting SIP headers to refresh PUBLISH request failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (m_pRefreshHelper->SendRefreshRequest(piScc) != IMS_SUCCESS)
        {
            m_pPubState->SetOperation(PubState::NO_OPERATION);
            piScc->Close();

            IMS_TRACE_E(0, "Sending a publication refresh request failed", 0, 0, 0);
            return IMS_FALSE;
        }

        // Update the publication state
        m_pPubState->UpdateState(piSipMsg);

        PostMessage(AMSG_PUBLICATION_REFRESH_STARTED, 0, 0);

        return IMS_TRUE;
    }

    return (bDoImplicitRefresh == IMS_TRUE) ? IMS_FALSE : IMS_TRUE;
}

PRIVATE VIRTUAL void Publication::Refreshable_RefreshTerminated()
{
    IMS_TRACE_D("_____ PUBLICATION REFRESH TERMINATED ...", 0, 0, 0);

    if (m_piRefreshListener != IMS_NULL)
    {
        m_piRefreshListener->Refresh_NotifyTerminated();
    }

    if (GetState() == STATE_ACTIVE)
    {
        SetState(STATE_INACTIVE);
        PostMessage(AMSG_PUBLICATION_TERMINATED, 0, 0);
    }

    if (GetState() == STATE_INACTIVE)
    {
        m_pPubState->Clear();
    }
}

PRIVATE
void Publication::CloseConnection()
{
    switch (m_pPubState->GetOperation())
    {
        case PubState::OPERATION_CREATE:  // FALL-THROUGH
        case PubState::OPERATION_MODIFY:
            ServiceMethod::CloseConnection(IMessage::PUBLICATION_PUBLISH);
            break;
        case PubState::OPERATION_REMOVE:
            ServiceMethod::CloseConnection(IMessage::PUBLICATION_UNPUBLISH);
            break;
        default:
            // Do nothing ...
            break;
    }

    m_pPubState->SetOperation(PubState::NO_OPERATION);
}

PRIVATE
ISipClientConnection* Publication::CreateConnectionWithDialog(
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
void Publication::ReceiveResponse(IN const ISipClientConnection* piScc)
{
    switch (m_pPubState->GetOperation())
    {
        case PubState::OPERATION_CREATE:  // FALL-THROUGH
        case PubState::OPERATION_MODIFY:  // FALL-THROUGH
        case PubState::OPERATION_REFRESH:
            UpdateResponseOnReceived(IMessage::PUBLICATION_PUBLISH, piScc);
            break;
        case PubState::OPERATION_REMOVE:
            UpdateResponseOnReceived(IMessage::PUBLICATION_UNPUBLISH, piScc);
            break;
        default:
            // Do nothing ...
            break;
    }
}

PRIVATE
void Publication::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("Publication :: %s to %s", StateToString(m_nState), StateToString(nState), 0);

    m_nState = nState;
}

PRIVATE GLOBAL const IMS_CHAR* Publication::StateToString(IN IMS_SINT32 nState)
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
