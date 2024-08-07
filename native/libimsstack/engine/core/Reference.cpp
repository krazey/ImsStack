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
#include "TextParser.h"

#include "private/SipConfigV.h"

#include "CallControlHelper.h"
#include "Capabilities.h"
#include "IOnNotificationListener.h"
#include "IOnReferenceListener.h"
#include "ISipDialog.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipServerConnection.h"
#include "ImplicitNotifierState.h"
#include "ImplicitSubscriberState.h"
#include "ImsCoreContext.h"
#include "Reference.h"
#include "Service.h"
#include "Sip.h"
#include "SipConfigProxy.h"
#include "SipDebug.h"
#include "SipFeatures.h"
#include "base/Ims.h"
#include "util/DialogMethodManager.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC GLOBAL const IMS_CHAR Reference::EVENT_REFER[] = "refer";
PUBLIC GLOBAL const IMS_CHAR Reference::MEDIA_TYPE[] = "message/sipfrag";

PUBLIC
Reference::NotifierState::NotifierState() :
        m_objSccs(ImsList<ISipClientConnection*>())
{
}

PUBLIC
Reference::NotifierState::~NotifierState()
{
    if (!m_objSccs.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objSccs.GetSize(); ++i)
        {
            ISipClientConnection* piScc = m_objSccs.GetAt(i);

            if (piScc != IMS_NULL)
            {
                piScc->Close();
            }
        }

        m_objSccs.Clear();
    }
}

PUBLIC
void Reference::NotifierState::RemoveScc(IN const ISipClientConnection* piScc)
{
    for (IMS_UINT32 i = 0; i < m_objSccs.GetSize(); ++i)
    {
        ISipClientConnection* piTempScc = m_objSccs.GetAt(i);

        if (piScc == piTempScc)
        {
            m_objSccs.RemoveAt(i);
            break;
        }
    }
}

PUBLIC
Reference::Reference(IN Service* pService, IN const AString& strReferToUri,
        IN const AString& strReferMethod, IN const Replaces& objReplaces,
        IN IMS_BOOL bImplicitRoutingRequired /*= IMS_FALSE*/) :
        ServiceMethod(pService),
        m_nState(STATE_INITIATED),
        m_strReferToUri(strReferToUri),
        m_objReferMethod(strReferMethod),
        m_pReplaces(IMS_NULL),
        m_bImplicitSubscription(IMS_FALSE),
        m_pReferredMethod(IMS_NULL),
        m_piListener(IMS_NULL),
        m_pSubState(IMS_NULL),
        m_bReferenceInOtherDialog(IMS_FALSE),
        m_bImplicitRoutingRequired(bImplicitRoutingRequired),
        m_objNotifyMessages(ImsList<Message*>()),
        m_piNotificationListener(IMS_NULL),
        m_pNotifierState(IMS_NULL)
{
    const AString& strSessionId =
            ImsCoreContext::GetInstance()->GetCallControlHelper()->GetSessionIdFromReplaces(
                    &objReplaces);

    if (!strSessionId.IsNULL())
    {
        m_pReplaces = new Replaces(objReplaces);
    }
}

PUBLIC VIRTUAL Reference::~Reference()
{
    CleanupOnDestroy();

    if (m_pReplaces != IMS_NULL)
    {
        delete m_pReplaces;
    }

    while (!m_objNotifyMessages.IsEmpty())
    {
        Message* pMessage = m_objNotifyMessages.GetAt(0);

        if (pMessage != IMS_NULL)
        {
            delete pMessage;
        }

        m_objNotifyMessages.RemoveAt(0);
    }

    if (m_pSubState != IMS_NULL)
    {
        delete m_pSubState;
    }

    if (m_pReferredMethod != IMS_NULL)
    {
        m_pReferredMethod->SetReferredMessageListener(IMS_NULL);
    }

    if (m_pNotifierState != IMS_NULL)
    {
        delete m_pNotifierState;
    }
}

PUBLIC VIRTUAL void Reference::Destroy()
{
    CleanupOnDestroy();
    ServiceMethod::Destroy();
}

PUBLIC
IMS_RESULT Reference::Accept()
{
    if (!IsServiceOpen())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_PROCEEDING)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    SetState(STATE_REFERRING);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Reference::ConnectReferMethod(IN Method* pReferMethod)
{
    if (pReferMethod == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_REFERRING)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    m_pReferredMethod = pReferMethod;

    if (!m_pReferredMethod->SetReferredMessageListener(this))
    {
        IMS_TRACE_E(0, "Not implemented method", 0, 0, 0);
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
const AString& Reference::GetReplaces() const
{
    if (m_pReplaces == IMS_NULL)
    {
        return AString::ConstNull();
    }

    IMS_TRACE_D("___ Replaces: %s ___",
            SipDebug::GetStr1(m_pReplaces->ToString(IMS_FALSE), 8, '@').GetStr(), 0, 0);

    return ImsCoreContext::GetInstance()->GetCallControlHelper()->GetSessionIdFromReplaces(
            m_pReplaces);
}

PUBLIC
IMS_RESULT Reference::Refer(IN IMS_BOOL bImplicitSubscription)
{
    if (!IsServiceOpen())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_INITIATED)
    {
        IMS_TRACE_E(
                0, "To send the reference request, the state MUST be an INITIATED state", 0, 0, 0);
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    ISipDialog* piDialog = GetDialog();
    SipMethod objMethod(SipMethod::REFER);
    ISipClientConnection* piScc = IMS_NULL;

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
        Ims::SetLastError(ImsError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    if (SipFeatures::IsEventHeaderApplicableForRefer(GetSlotId()))
    {
        // Event header
        piSipMsg->SetHeader(ISipHeader::EVENT, EVENT_REFER);
    }

    // Refer-To header
    SipAddress objReferToUri;

    if (!objReferToUri.Create(m_strReferToUri))
    {
        IMS_TRACE_E(0, "Parsing Refer-To (URI) failed", 0, 0, 0);
        Ims::SetLastError(ImsError::PARSING_ERROR);
        return IMS_FAILURE;
    }

    if (!m_objReferMethod.Equals(SipMethod::INVALID))
    {
        if (objReferToUri.SetParameter(Sip::STR_METHOD, m_objReferMethod.ToString().GetStr()) !=
                IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting a 'method' parameter failed", 0, 0, 0);
            Ims::SetLastError(ImsError::GENERAL_ERROR);
            return IMS_FAILURE;
        }
    }

    if (m_pReplaces != IMS_NULL)
    {
        objReferToUri.SetHeader(ISipHeader::REPLACES, m_pReplaces->ToString(IMS_TRUE));
    }

    piSipMsg->SetHeader(ISipHeader::REFER_TO, objReferToUri.ToString());

    if (SipFeatures::IsReferSubHeaderSupported(GetSlotId()))
    {
        // Supported header : "norefersub"
        piSipMsg->AddHeader(ISipHeader::SUPPORTED, "norefersub");

        // Refer-Sub header : "true" / "false"
        piSipMsg->SetHeader(ISipHeader::REFER_SUB,
                bImplicitSubscription ? TextParser::STR_SMALL_TRUE : TextParser::STR_SMALL_FALSE);
    }

    // Try to send a SUBSCRIBE request to the network
    if (!SendNUpdateRequest(IMessage::REFERENCE_REFER, piScc))
    {
        piScc->Close();
        return IMS_FAILURE;
    }

    m_pSubState->SetOperation(SubState::OPERATION_CREATE);

    // Update the subscription state
    if (!m_pSubState->UpdateState(piSipMsg))
    {
        m_pSubState->SetOperation(SubState::NO_OPERATION);
        return IMS_FAILURE;
    }

    // Store the flag to indicate if the implicit subscription is requested or not
    m_bImplicitSubscription = bImplicitSubscription;

    // Update the dialog state for REFER-created dialog (to handle early NOTIFY request)
    if (m_bReferenceInOtherDialog)
    {
        CheckNCreateDialog(piScc, IMS_TRUE);
    }

    SetState(STATE_PROCEEDING);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Reference::Reject()
{
    if (!IsServiceOpen())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_PROCEEDING)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    // Send a NOTIFY request with 'terminated' sub-state
    AString strRejectRequest("SIP/2.0 603 Declined");

    DoNotification(SubState::SUB_STATE_TERMINATED, strRejectRequest);

    SetState(STATE_TERMINATED);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Reference::SetReplaces(IN const AString& strSessionId)
{
    if (GetState() != STATE_INITIATED)
    {
        IMS_TRACE_E(0, "To set the Replaces header, the state MUST be an INITIATED state", 0, 0, 0);
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    if (strSessionId.IsNULL() || strSessionId.IsEmpty())
    {
        IMS_TRACE_E(0, "Session id is null or empty string", 0, 0, 0);
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (m_pReplaces != IMS_NULL)
    {
        delete m_pReplaces;
        m_pReplaces = IMS_NULL;
    }

    Replaces* pTmpReplaces =
            ImsCoreContext::GetInstance()->GetCallControlHelper()->GetReplacesFromSessionId(
                    strSessionId);

    if (pTmpReplaces == IMS_NULL)
    {
        IMS_TRACE_E(0, "Can't find a Replaces from sessionId (%s)", strSessionId.GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    m_pReplaces = new Replaces(*pTmpReplaces);

    if (m_pReplaces == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Reference::AcceptEx(IN IMS_SINT32 nStatusCode /*= SipStatusCode::SC_202*/,
        IN IMS_BOOL b100Trying /*= IMS_TRUE*/)
{
    if (!SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (!IsServiceOpen())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_INITIATED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSsc = GetServerConnection(IMessage::REFERENCE_REFER);

    if (piSsc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (CreateResponse(piSsc, nStatusCode) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to REFER request failed", 0, 0, 0);

        CloseConnection(IMessage::REFERENCE_REFER);
        return IMS_FAILURE;
    }

    // Send a response to REFER request immediately...
    if (!SendNUpdateResponse(IMessage::REFERENCE_REFER, piSsc))
    {
        IMS_TRACE_E(0, "Sending the response to REFER request failed ...", 0, 0, 0);

        CloseConnection(IMessage::REFERENCE_REFER);
        return IMS_FAILURE;
    }

    // Check & create a dialog
    CheckNCreateDialog(piSsc, m_bReferenceInOtherDialog);

    SetState(STATE_REFERRING);

    CloseConnection(IMessage::REFERENCE_REFER);

    // Send a NOTIFY request with 'active' sub-state
    if (b100Trying)
    {
        DoNotification(SubState::SUB_STATE_ACTIVE, AString("SIP/2.0 100 Trying"));
    }

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Reference::ReferEx(IN IMS_BOOL bImplicitSubscription,
        IN const AString& strHeadersForReferTo /*= AString::ConstNull()*/)
{
    if (!IsServiceOpen())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_INITIATED)
    {
        IMS_TRACE_E(
                0, "To send the reference request, the state MUST be an INITIATED state", 0, 0, 0);
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    ISipDialog* piDialog = GetDialog();
    SipMethod objMethod(SipMethod::REFER);
    ISipClientConnection* piScc = IMS_NULL;

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
        Ims::SetLastError(ImsError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    if (SipFeatures::IsEventHeaderApplicableForRefer(GetSlotId()))
    {
        // Event header
        piSipMsg->SetHeader(ISipHeader::EVENT, EVENT_REFER);
    }

    // Refer-To header
    SipAddress objReferToUri;

    if (!objReferToUri.Create(m_strReferToUri))
    {
        IMS_TRACE_E(0, "Parsing Refer-To (URI) failed", 0, 0, 0);
        Ims::SetLastError(ImsError::PARSING_ERROR);
        return IMS_FAILURE;
    }

    if (!m_objReferMethod.Equals(SipMethod::INVALID))
    {
        if (objReferToUri.SetParameter(Sip::STR_METHOD, m_objReferMethod.ToString().GetStr()) !=
                IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting a 'method' parameter failed", 0, 0, 0);
            Ims::SetLastError(ImsError::GENERAL_ERROR);
            return IMS_FAILURE;
        }
    }

    if (m_pReplaces != IMS_NULL)
    {
        objReferToUri.SetHeader(ISipHeader::REPLACES, m_pReplaces->ToString(IMS_TRUE));
    }

    // Sets the additional header parameters
    if (strHeadersForReferTo.GetLength() > 0)
    {
        if (objReferToUri.SetHeaders(strHeadersForReferTo, IMS_FALSE) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting the additional header parameters (%s) failed",
                    strHeadersForReferTo.GetStr(), 0, 0);
            Ims::SetLastError(ImsError::GENERAL_ERROR);
            return IMS_FAILURE;
        }
    }

    piSipMsg->SetHeader(ISipHeader::REFER_TO, objReferToUri.ToString());

    if (SipFeatures::IsReferSubHeaderSupported(GetSlotId()))
    {
        // Supported header : "norefersub"
        piSipMsg->AddHeader(ISipHeader::SUPPORTED, "norefersub");

        // Refer-Sub header : "true" / "false"
        piSipMsg->SetHeader(ISipHeader::REFER_SUB,
                bImplicitSubscription ? TextParser::STR_SMALL_TRUE : TextParser::STR_SMALL_FALSE);
    }

    // Try to send a SUBSCRIBE request to the network
    if (!SendNUpdateRequest(IMessage::REFERENCE_REFER, piScc))
    {
        piScc->Close();
        return IMS_FAILURE;
    }

    m_pSubState->SetOperation(SubState::OPERATION_CREATE);

    // Update the subscription state
    if (!m_pSubState->UpdateState(piSipMsg))
    {
        m_pSubState->SetOperation(SubState::NO_OPERATION);
        return IMS_FAILURE;
    }

    // Store the flag to indicate if the implicit subscription is requested or not
    m_bImplicitSubscription = bImplicitSubscription;

    // Update the dialog state for REFER-created dialog (to handle early NOTIFY request)
    if (m_bReferenceInOtherDialog)
    {
        CheckNCreateDialog(piScc, IMS_TRUE);
    }

    SetState(STATE_PROCEEDING);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Reference::RejectEx(IN IMS_SINT32 nStatusCode)
{
    if (!SipStatusCode::IsFinalFailure(nStatusCode))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (!IsServiceOpen())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_INITIATED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSsc = GetServerConnection(IMessage::REFERENCE_REFER);

    if (piSsc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (CreateResponse(piSsc, nStatusCode) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to REFER request failed", 0, 0, 0);

        CloseConnection(IMessage::REFERENCE_REFER);
        return IMS_FAILURE;
    }

    // Send a response to REFER request immediately...
    if (!SendNUpdateResponse(IMessage::REFERENCE_REFER, piSsc))
    {
        IMS_TRACE_E(0, "Sending the response to REFER request failed ...", 0, 0, 0);

        CloseConnection(IMessage::REFERENCE_REFER);
        return IMS_FAILURE;
    }

    SetState(STATE_TERMINATED);

    CloseConnection(IMessage::REFERENCE_REFER);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT Reference::SendNotification(IN IMS_SINT32 nSubState, IN const ByteArray& objContent,
        IN IMS_SINT32 nReason /*= ISubscriptionState::REASON_NONE*/,
        IN IMS_SINT32 nExpires /*= (-1)*/)
{
    if (GetState() != STATE_REFERRING)
    {
        IMS_TRACE_E(0, "Notification can't be sent in %s", StateToString(GetState()), 0, 0);
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    nSubState = SubState::GetSubStateFromSubscriptionState(nSubState);

    if (nSubState == SubState::SUB_STATE_INIT)
    {
        IMS_TRACE_E(0, "Invalid sub-state", 0, 0, 0);
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    nReason = SubState::GetReasonFromSubscriptionState(nReason);

    return DoNotification(nSubState, objContent, nReason, nExpires);
}

PUBLIC
void Reference::SetImplicitRoutingRequired(IN IMS_BOOL bFlag)
{
    m_bImplicitRoutingRequired = bFlag;

    // FIXME: If the routing address needs to be provisioned by the application,
    // please add a second argument for it.
}

PROTECTED VIRTUAL IMS_BOOL Reference::DispatchMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case AMSG_REFERENCE_DELIVERED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnReference_Delivered(this);
            }
            return IMS_TRUE;
        case AMSG_REFERENCE_DELIVERY_FAILED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnReference_DeliveryFailed(this);
            }
            return IMS_TRUE;
        case AMSG_REFERENCE_NOTIFY:
            if (!m_objNotifyMessages.IsEmpty())
            {
                Message* pMessage = m_objNotifyMessages.GetAt(0);

                if (m_piListener != IMS_NULL)
                {
                    m_piListener->OnReference_NotifyReceived(this, pMessage);
                }

                m_objNotifyMessages.RemoveAt(0);
                delete pMessage;
            }
            return IMS_TRUE;
        case AMSG_REFERENCE_TERMINATED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnReference_Terminated(this);
            }
            return IMS_TRUE;
        case AMSG_NOTIFICATION_DELIVERED:
            if (m_piNotificationListener != IMS_NULL)
            {
                m_piNotificationListener->OnNotification_Delivered(this);
            }
            return IMS_TRUE;
        case AMSG_NOTIFICATION_DELIVERY_FAILED:
            if (m_piNotificationListener != IMS_NULL)
            {
                m_piNotificationListener->OnNotification_DeliveryFailed(
                        this, LONG_TO_SINT(objMsg.nLparam));
            }
            return IMS_TRUE;
        default:
            break;
    }

    return EngineActivity::DispatchMessage(objMsg);
}

PROTECTED VIRTUAL IMS_BOOL Reference::SendRequestToChallenge(IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Clear the connection to preserve the SIP connection
    ClearConnection(IMessage::REFERENCE_REFER);

    if (!SendNUpdateRequestEx(IMessage::REFERENCE_REFER, piScc, MESSAGE_CLASS_RESUBMIT))
    {
        // Revert the SIP connection
        UpdateConnection(IMessage::REFERENCE_REFER, piScc);
        return IMS_FALSE;
    }

    // Update the subscription state
    if (!m_pSubState->UpdateState(piScc->GetMessage()))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void Reference::Exception_NotifyError(IN IMS_SINT32 nErrorCode)
{
    (void)nErrorCode;

    if (GetState() == STATE_PROCEEDING)
    {
        SetState(STATE_TERMINATED);
        PostMessage(AMSG_REFERENCE_DELIVERY_FAILED, 0, 0);
    }
    else
    {
        // SIP_TXN_N_DIALOG_HANDLING_ON_NO_REG
#if 0
        SetState(STATE_TERMINATED);
        PostMessage(AMSG_REFERENCE_TERMINATED, 0, 0);
#endif
    }
}

PROTECTED VIRTUAL IMS_BOOL Reference::InitInstance()
{
    if (m_pSubState == IMS_NULL)
    {
        if (IsMobileOriginated())
        {
            m_pSubState = new ImplicitSubscriberState();
        }
        else
        {
            m_pSubState = new ImplicitNotifierState();
            m_pNotifierState = new NotifierState();
        }

        if (m_pSubState == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a subscription state (MO: %s) failed",
                    TextParser::BooleanToString(IsMobileOriginated()), 0, 0);
            return IMS_FALSE;
        }

        if (!m_pSubState->CreateEventPackage(EVENT_REFER))
        {
            IMS_TRACE_E(0, "Creating an event package for an event (refer) failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    DialogMethodManager::GetInstance()->AddMethod(GetName(), this);
    GetService()->RegisterMethod(this);

    if (GetDialog() != IMS_NULL)
    {
        m_bReferenceInOtherDialog = IMS_TRUE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL Reference::NotifySipRequest(IN ISipServerConnection* piSsc)
{
    IMS_TRACE_I("Reference - REFER REQUEST RECEIVED ...", 0, 0, 0);

    if (!UpdateRequestOnReceived(IMessage::REFERENCE_REFER, piSsc))
    {
        IMS_TRACE_E(0, "Updating SIP message failed", 0, 0, 0);
    }

    // Check if the response needs to be handled by application.
    const SipConfigV* pSipConfigV = GetService()->GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->IsReferenceRespByApp())
        {
            IMS_TRACE_I("INCOMING REFER REQUEST WILL BE HANDLED BY APPLICATION", 0, 0, 0);

            // Notify the information which the Reference is received
            if (!m_bReferenceInOtherDialog)
            {
                GetService()->HandleReferenceReceived(this);
            }

            return IMS_TRUE;
        }
    }

    // Send a 202 ACCEPTED to REFER request
    if (CreateResponse(piSsc, SipStatusCode::SC_202) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to REFER request failed", 0, 0, 0);

        CloseConnection(IMessage::REFERENCE_REFER);
        return IMS_FALSE;
    }

    // Send a response to REFER request immediately...
    if (!SendNUpdateResponse(IMessage::REFERENCE_REFER, piSsc))
    {
        IMS_TRACE_E(0, "Sending the response to REFER request failed ...", 0, 0, 0);

        CloseConnection(IMessage::REFERENCE_REFER);
        return IMS_FALSE;
    }

    // Check & create a dialog
    CheckNCreateDialog(piSsc, m_bReferenceInOtherDialog);

    SetState(STATE_PROCEEDING);

    CloseConnection(IMessage::REFERENCE_REFER);

    // Send a NOTIFY request with 'active' sub-state
    DoNotification(SubState::SUB_STATE_ACTIVE, AString("SIP/2.0 100 Trying"));

    if (!m_bReferenceInOtherDialog)
    {
        GetService()->HandleReferenceReceived(this);
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void Reference::NotifySipResponse(IN ISipClientConnection* piScc)
{
    const SipMethod& objMethod = piScc->GetMethod();

    // REFER-Issuer behavior
    if (objMethod.Equals(SipMethod::REFER))
    {
        if (!m_pSubState->UpdateState(piScc->GetMessage()))
        {
            CloseConnection(IMessage::REFERENCE_REFER);
            SetState(STATE_TERMINATED);
            PostMessage(AMSG_REFERENCE_DELIVERY_FAILED, 0, 0);

            IMS_TRACE_E(0, "Updating an implicit subscription failed", 0, 0, 0);
            return;
        }

        // Add the response message received
        if (!UpdateResponseOnReceived(IMessage::REFERENCE_REFER, piScc))
        {
            CloseConnection(IMessage::REFERENCE_REFER);
            SetState(STATE_TERMINATED);
            PostMessage(AMSG_REFERENCE_DELIVERY_FAILED, 0, 0);

            IMS_TRACE_E(0, "Storing Message failed", 0, 0, 0);
            return;
        }

        IMS_SINT32 nStatusCode = piScc->GetStatusCode();

        // AUTH_SIP_DIGEST {
        // Handle 401/407 response
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

        // Check the status code
        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            CheckNCreateDialog(piScc, m_bReferenceInOtherDialog);

            CloseConnection(IMessage::REFERENCE_REFER);

            SetState(STATE_REFERRING);
            PostMessage(AMSG_REFERENCE_DELIVERED, 0, 0);
        }
        else
        {
            CloseConnection(IMessage::REFERENCE_REFER);

            SetState(STATE_TERMINATED);

            if (m_bImplicitSubscription)
            {
                PostMessage(AMSG_REFERENCE_DELIVERY_FAILED, 0, 0);
            }
            else
            {
                PostMessage(AMSG_REFERENCE_DELIVERED, 0, 0);
            }
        }
    }
    // REFER-Recipient behavior
    else if (objMethod.Equals(SipMethod::NOTIFY))
    {
        // Update the subscription state for NOTIFY response
        m_pSubState->UpdateState(piScc->GetMessage());

        IMS_SINT32 nStatusCode = piScc->GetStatusCode();

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            PostMessage(AMSG_NOTIFICATION_DELIVERED, 0, 0);
        }
        else
        {
            PostMessage(AMSG_NOTIFICATION_DELIVERY_FAILED, 0, nStatusCode);
        }

        if (m_pNotifierState != IMS_NULL)
        {
            m_pNotifierState->RemoveScc(piScc);
        }

        piScc->Close();

        if (m_pSubState->GetState() == SubState::STATE_TERMINATED)
        {
            SetState(STATE_TERMINATED);
            PostMessage(AMSG_REFERENCE_TERMINATED, 0, 0);
        }
    }
    else
    {
        IMS_TRACE_E(0, "___ NOT HANDLED METHOD (%s) ___", objMethod.ToString().GetStr(), 0, 0);

        piScc->Close();
    }
}

PROTECTED VIRTUAL void Reference::NotifySipError(
        IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    const SipMethod& objMethod = piSc->GetMethod();

    (void)nCode;
    (void)strMessage;

    // 1 : Needs to be handled according to the SIP method (REFER/NOTIFY) ?

    if (objMethod.Equals(SipMethod::REFER))
    {
        CloseConnection(IMessage::REFERENCE_REFER);
    }
    else if (objMethod.Equals(SipMethod::NOTIFY))
    {
        IMS_TRACE_E(0, "NOTIFY transaction will be closed", 0, 0, 0);

        PostMessage(AMSG_NOTIFICATION_DELIVERY_FAILED, 0, 0);

        if (m_pNotifierState != IMS_NULL)
        {
            m_pNotifierState->RemoveScc(DYNAMIC_CAST(ISipClientConnection*, piSc));
        }

        piSc->Close();
    }

    if (GetState() == STATE_PROCEEDING)
    {
        SetState(STATE_TERMINATED);
        PostMessage(AMSG_REFERENCE_DELIVERY_FAILED, 0, 0);
    }
    else
    {
        SetState(STATE_TERMINATED);
        PostMessage(AMSG_REFERENCE_TERMINATED, 0, 0);
    }
}

PROTECTED VIRTUAL IMS_BOOL Reference::Dialog_Compare(IN ISipServerConnection* piSsc) const
{
    if (!IsServiceOpen())
    {
        return IMS_FALSE;
    }

    // Filters some method which does not handle in the reference (implicit subscription)
    if (!piSsc->GetMethod().Equals(SipMethod::NOTIFY))
    {
        return IMS_FALSE;
    }

    ISipDialog* piDialog = GetDialog();

    if (piDialog == IMS_NULL)
    {
        // In case of an early NOTIFY received ...
        if (GetState() == STATE_PROCEEDING)
        {
            ISipClientConnection* piScc = GetClientConnection(IMessage::REFERENCE_REFER);

            IMS_TRACE_I("Checks if the early NOTIFY is received or not ...", 0, 0, 0);

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

PROTECTED VIRTUAL IMS_BOOL Reference::Dialog_NotifyRequest(IN ISipServerConnection* piSsc)
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

        IMS_TRACE_E(0, "___ UNHANDLED SIP METHOD (%s) ___", piSsc->GetMethod().ToString().GetStr(),
                0, 0);
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
    if (GetState() == STATE_PROCEEDING)
    {
        CheckNCreateDialog(piSsc, m_bReferenceInOtherDialog);
    }

    // Update the subscription state for NOTIFY request
    if (!m_pSubState->UpdateState(piSsc->GetMessage()))
    {
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

    if (!m_objNotifyMessages.Append(pMessage))
    {
        delete pMessage;

        IMS_TRACE_E(0, "Queueing NOTIFY message failed", 0, 0, 0);
    }

    piSsc->Close();

    PostMessage(AMSG_REFERENCE_NOTIFY, 0, 0);

    if (m_pSubState->GetState() == SubState::STATE_TERMINATED)
    {
        PostMessage(AMSG_REFERENCE_TERMINATED, 0, 0);
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void Reference::ReferredMessage_NotifyOnActive(IN ISipMessage* piSipMsg)
{
    // 1 In this moment, only includes the start line

    if (piSipMsg == IMS_NULL)
    {
        // Nothing to do...
        return;
    }

    // 4 ACK ?
    if (!m_objReferMethod.Equals(SipMethod::INVALID))
    {
        if (!m_objReferMethod.Equals(piSipMsg->GetMethod()))
        {
            IMS_TRACE_E(0, "Not matched method (Refer-To: %s, Message: %s)",
                    m_objReferMethod.ToString().GetStr(), piSipMsg->GetMethod().ToString().GetStr(),
                    0);
            return;
        }
    }
    else
    {
        // As a default, INVITE method
        if (!piSipMsg->GetMethod().Equals(SipMethod::INVITE))
        {
            IMS_TRACE_E(0, "Not matched method (Refer-To: empty, Message: %s)",
                    piSipMsg->GetMethod().ToString().GetStr(), 0, 0);
            return;
        }
    }

    ByteArray objContent = piSipMsg->ToByteArray(ISipMessage::OPT_START_LINE);

    if (objContent.GetLength() == 0)
    {
        IMS_TRACE_E(0, "Creating a message/sipfrag failed", 0, 0, 0);
        return;
    }

    DoNotification(SubState::SUB_STATE_ACTIVE, objContent);
}

PROTECTED VIRTUAL void Reference::ReferredMessage_NotifyOnTerminated(
        IN IMS_SINT32 nReasonCode /*= SubState::REASON_NONE*/,
        IN ISipMessage* piSipMsg /*= IMS_NULL*/)
{
    // 1 In this moment, only includes the start line

    ByteArray objContent;

    if (piSipMsg != IMS_NULL)
    {
        if (!m_objReferMethod.Equals(piSipMsg->GetMethod()))
        {
            IMS_TRACE_E(0, "Not matched method (Refer-To: %s, Message: %s)",
                    m_objReferMethod.ToString().GetStr(), piSipMsg->GetMethod().ToString().GetStr(),
                    0);
            return;
        }

        objContent = piSipMsg->ToByteArray(ISipMessage::OPT_START_LINE);
    }

    DoNotification(SubState::SUB_STATE_TERMINATED, objContent, nReasonCode);

    if (m_pReferredMethod != IMS_NULL)
    {
        m_pReferredMethod->SetReferredMessageListener(IMS_NULL);
        m_pReferredMethod = IMS_NULL;
    }
}

PRIVATE
void Reference::CleanupOnDestroy()
{
    if (GetState() != STATE_TERMINATED)
    {
        ISipDialog* piDialog = GetDialog();

        if (piDialog != IMS_NULL)
        {
            piDialog->TerminateDialogUsage();
        }

        SetState(STATE_TERMINATED);
    }

    DialogMethodManager::GetInstance()->RemoveMethod(GetName());

    GetService()->DeregisterMethod(this);
}

PRIVATE
ISipClientConnection* Reference::CreateConnectionL(
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
IMS_RESULT Reference::DoNotification(IN IMS_SINT32 nSubState, IN const ByteArray& objContent,
        IN IMS_SINT32 nReasonCode /*= SubState::REASON_NONE*/, IN IMS_SINT32 nExpires /*= (-1)*/)
{
    ISipDialog* piDialog = GetDialog();

    if (piDialog == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    SipMethod objMethod(SipMethod::NOTIFY);
    ISipClientConnection* piScc = GetService()->CreateConnection(piDialog, objMethod);

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        Ims::SetLastError(ImsError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    piScc->SetListener(this);
    piScc->SetErrorListener(this);

    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    if (m_bImplicitRoutingRequired)
    {
        const AStringArray& objServiceRoutes = GetService()->GetServiceRoutes();

        if (!objServiceRoutes.IsEmpty())
        {
            piScc->SetImplicitRouteHeader(objServiceRoutes.GetElementAt(0));
        }
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Event header
    piSipMsg->SetHeader(ISipHeader::EVENT, EVENT_REFER);

    // Subscription-State header
    AString strSubState;

    if (nSubState == SubState::SUB_STATE_ACTIVE)
    {
        strSubState = Sip::STR_ACTIVE;

        // 'expires' parameter ???
    }
    else if (nSubState == SubState::SUB_STATE_PENDING)
    {
        strSubState = Sip::STR_PENDING;
    }
    else if (nSubState == SubState::SUB_STATE_TERMINATED)
    {
        strSubState = Sip::STR_TERMINATED;

        if ((nReasonCode > SubState::REASON_NONE) && (nReasonCode < SubState::REASON_MAX))
        {
            strSubState += TextParser::CHAR_SEMICOLON;
            strSubState += SubState::STR_REASON;
            strSubState += TextParser::CHAR_EQUAL;

            switch (nReasonCode)
            {
                case SubState::REASON_DEACTIVATED:
                    strSubState += SubState::STR_REASON_DEACTIVATED;
                    break;
                case SubState::REASON_PROBATION:
                    strSubState += SubState::STR_REASON_PROBATION;
                    break;
                case SubState::REASON_REJECTED:
                    strSubState += SubState::STR_REASON_REJECTED;
                    break;
                case SubState::REASON_TIMEOUT:
                    strSubState += SubState::STR_REASON_TIMEOUT;
                    break;
                case SubState::REASON_GIVEUP:
                    strSubState += SubState::STR_REASON_GIVEUP;
                    break;
                case SubState::REASON_NORESOURCE:
                    strSubState += SubState::STR_REASON_NORESOURCE;
                    break;
                default:
                    break;
            }
        }
    }

    if (nExpires >= 0)
    {
        AString strExpires;
        strExpires.SetNumber(nExpires);

        strSubState += TextParser::CHAR_SEMICOLON;
        strSubState += Sip::STR_EXPIRES;
        strSubState += TextParser::CHAR_EQUAL;
        strSubState += strExpires;
    }

    piSipMsg->SetHeader(ISipHeader::SUBSCRIPTION_STATE, strSubState);

    // Set contents (message/sipfrag)
    if (!objContent.IsNULL())
    {
        piSipMsg->SetHeader(ISipHeader::CONTENT_TYPE, MEDIA_TYPE);

        ISipMessageBodyPart* piSipBodyPart = piSipMsg->CreateBodyPart();

        if (piSipBodyPart == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a new SIP message body part failed", 0, 0, 0);
            piScc->Close();
            return IMS_FAILURE;
        }

        piSipBodyPart->SetContent(objContent);
    }

    // SIP_MESSAGE_MEDIATOR
    (void)AdjustMessage(piScc->GetMessage(), MESSAGE_CLASS_AUTOMATIC);

    // Try to send a NOTIFY request to the network
    if (piScc->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(
                0, "Sending a NOTIFY request through the implicit subscription failed", 0, 0, 0);
        piScc->Close();
        return IMS_FAILURE;
    }

    m_pSubState->UpdateState(piScc->GetMessage());

    if (m_pNotifierState != IMS_NULL)
    {
        m_pNotifierState->AddScc(piScc);
    }

    return IMS_SUCCESS;
}

PRIVATE
void Reference::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("Reference :: %s to %s", StateToString(m_nState), StateToString(nState), 0);

    m_nState = nState;
}

PRIVATE GLOBAL const IMS_CHAR* Reference::StateToString(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case STATE_INITIATED:
            return "STATE_INITIATED";
        case STATE_PROCEEDING:
            return "STATE_PROCEEDING";
        case STATE_REFERRING:
            return "STATE_REFERRING";
        case STATE_TERMINATED:
            return "STATE_TERMINATED";
        default:
            return "__INVALID__";
    }
}
