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

#include "private/SipConfigV.h"

#include "IOnPageMessageListener.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipServerConnection.h"
#include "Message.h"
#include "PageMessage.h"
#include "Service.h"
#include "SipConfigProxy.h"
#include "SipHeaderName.h"
#include "base/Ims.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
PageMessage::PageMessage(IN Service* pService) :
        ServiceMethod(pService),
        m_nState(STATE_UNSENT),
        m_piListener(IMS_NULL)
{
}

PUBLIC
const ByteArray& PageMessage::GetContent() const
{
    if (GetState() != STATE_RECEIVED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return ByteArray::ConstNull();
    }

    const Message* pMessage = GetPreviousRequest(IMessage::PAGEMESSAGE_SEND);

    if (pMessage == IMS_NULL)
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return ByteArray::ConstNull();
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    const ISipMessage* piSipMsg = pMessage->GetMessage();
    ImsList<ISipMessageBodyPart*> objBodyParts = piSipMsg->GetBodyParts();

    if (objBodyParts.IsEmpty())
    {
        return ByteArray::ConstNull();
    }

    const ISipMessageBodyPart* piBodyPart = objBodyParts.GetAt(0);

    if (piBodyPart == IMS_NULL)
    {
        return ByteArray::ConstNull();
    }

    return piBodyPart->GetContent();
}

PUBLIC
AString PageMessage::GetContentType() const
{
    if (GetState() != STATE_RECEIVED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return AString::ConstNull();
    }

    const Message* pMessage = GetPreviousRequest(IMessage::PAGEMESSAGE_SEND);

    if (pMessage == IMS_NULL)
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return AString::ConstNull();
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    const ISipMessage* piSipMsg = pMessage->GetMessage();
    ImsList<ISipMessageBodyPart*> objBodyParts = piSipMsg->GetBodyParts();

    if (objBodyParts.IsEmpty())
    {
        return AString::ConstNull();
    }

    const ISipMessageBodyPart* piBodyPart = objBodyParts.GetAt(0);

    if (piBodyPart == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return piBodyPart->GetHeader(ISipMessageBodyPart::CONTENT_TYPE);
}

PUBLIC
IMS_RESULT PageMessage::Send(IN const ByteArray& objContent, IN const AString& strContentType)
{
    if (!IsServiceOpen())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_UNSENT)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    if ((objContent.IsNULL() && !strContentType.IsNULL()) ||
            (!objContent.IsNULL() && strContentType.IsNULL()))
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

    // Create new connection and get SipMessage
    ISipClientConnection* piScc = CreateConnection(SipMethod(SipMethod::MESSAGE));

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    // Set Content and Content-Type header
    if (!objContent.IsNULL() && !strContentType.IsNULL())
    {
        ISipMessageBodyPart* piBodyPart = piSipMsg->CreateBodyPart();

        if (piBodyPart == IMS_NULL)
        {
            piScc->Close();

            Ims::SetLastError(ImsError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        // Set headers
        AString strContentLength;

        strContentLength.SetNumber(objContent.GetLength());

        piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_UNKNOWN, strContentLength,
                SipHeaderName::CONTENT_LENGTH);
        piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, strContentType);

        // Set content
        piBodyPart->SetContent(objContent);
    }

    if (!SendNUpdateRequest(IMessage::PAGEMESSAGE_SEND, piScc))
    {
        piScc->Close();
        return IMS_FAILURE;
    }

    // Update the state
    SetState(STATE_SENT);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT PageMessage::Accept(IN IMS_SINT32 nStatusCode /*= SipStatusCode::SC_200*/)
{
    if (!IsServiceOpen())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_RECEIVED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSsc = GetServerConnection(IMessage::PAGEMESSAGE_SEND);

    if (piSsc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    // Send a 200 OK to MESSAGE request
    if (CreateResponse(piSsc, nStatusCode) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to MESSAGE request failed", 0, 0, 0);
    }

    if (!SendNUpdateResponse(IMessage::PAGEMESSAGE_SEND, piSsc))
    {
        CloseConnection(IMessage::PAGEMESSAGE_SEND);

        IMS_TRACE_E(0, "Sending a response to MESSAGE request failed!", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Close the incoming SIP connection
    CloseConnection(IMessage::PAGEMESSAGE_SEND);

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT PageMessage::Reject(IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nRetryAfter /*= 0*/)
{
    if (!IsServiceOpen())
    {
        Ims::SetLastError(ImsError::SERVICE_CLOSED);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_RECEIVED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return IMS_FAILURE;
    }

    ISipServerConnection* piSsc = GetServerConnection(IMessage::PAGEMESSAGE_SEND);

    if (piSsc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    // Send a failure final response to OPTIONS request
    if (CreateResponse(piSsc, nStatusCode) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to MESSAGE request failed", 0, 0, 0);
    }

    // Now, the variable, piSIPMsg will be used for MESSAGE response message

    if (nRetryAfter > 0)
    {
        ISipMessage* piSipMsg = piSsc->GetMessage();
        AString strRetryAfter;

        strRetryAfter.SetNumber(nRetryAfter);

        piSipMsg->AddHeader(ISipHeader::RETRY_AFTER_SEC, strRetryAfter);
    }

    if (!SendNUpdateResponse(IMessage::PAGEMESSAGE_SEND, piSsc))
    {
        CloseConnection(IMessage::PAGEMESSAGE_SEND);

        IMS_TRACE_E(0, "Sending a response to MESSAGE request failed!", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Close the incoming SIP connection
    CloseConnection(IMessage::PAGEMESSAGE_SEND);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_BOOL PageMessage::DispatchMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case AMSG_PAGE_MESSAGE_RECEIVED:
            GetService()->HandlePageMessageReceived(this);
            return IMS_TRUE;
        case AMSG_PAGE_MESSAGE_DELIVERED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnPageMessage_Delivered(this);
            }
            return IMS_TRUE;
        case AMSG_PAGE_MESSAGE_DELIVERY_FAILED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->OnPageMessage_DeliveryFailed(this);
            }
            return IMS_TRUE;
        default:
            break;
    }

    return EngineActivity::DispatchMessage(objMsg);
}

PRIVATE VIRTUAL IMS_BOOL PageMessage::SendRequestToChallenge(IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Clear the connection to preserve the SIP connection
    ClearConnection(IMessage::PAGEMESSAGE_SEND);

    if (!SendNUpdateRequestEx(IMessage::PAGEMESSAGE_SEND, piScc, MESSAGE_CLASS_RESUBMIT))
    {
        // Revert the SIP connection
        UpdateConnection(IMessage::PAGEMESSAGE_SEND, piScc);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL IMS_BOOL PageMessage::NotifySipRequest(IN ISipServerConnection* piSsc)
{
    IMS_TRACE_I("PageMessage - MESSAGE REQUEST RECEIVED ...", 0, 0, 0);

    if (!UpdateRequestOnReceived(IMessage::PAGEMESSAGE_SEND, piSsc))
    {
        IMS_TRACE_E(0, "Updating SIP message failed", 0, 0, 0);
    }

    // Check if the response needs to be handled by application.
    const SipConfigV* pSipConfigV = GetService()->GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        if (pSipConfigV->IsPageMessageRespByApp())
        {
            IMS_TRACE_I("INCOMING MESSAGE REQUEST WILL BE HANDLED BY APPLICATION", 0, 0, 0);

            SetState(STATE_RECEIVED);

            // Notify the information which the PageMessage is received
            PostMessage(AMSG_PAGE_MESSAGE_RECEIVED, 0, 0);

            return IMS_TRUE;
        }
    }

    // Create a response and send it to the remote user.
    if (piSsc->InitResponse(SipStatusCode::SC_200) != IMS_SUCCESS)
    {
        CloseConnection(IMessage::PAGEMESSAGE_SEND);

        IMS_TRACE_E(0, "Initializing a response to MESSAGE request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!SendNUpdateResponse(IMessage::PAGEMESSAGE_SEND, piSsc))
    {
        CloseConnection(IMessage::PAGEMESSAGE_SEND);

        IMS_TRACE_E(0, "Sending a response to MESSAGE request failed!", 0, 0, 0);
        return IMS_FALSE;
    }

    // Close the incoming SIP connection
    CloseConnection(IMessage::PAGEMESSAGE_SEND);

    SetState(STATE_RECEIVED);

    // Notify the information which the PageMessage is received
    PostMessage(AMSG_PAGE_MESSAGE_RECEIVED, 0, 0);

    return IMS_TRUE;
}

PRIVATE VIRTUAL void PageMessage::NotifySipResponse(IN ISipClientConnection* piScc)
{
    IMS_SINT32 nStatusCode = piScc->GetStatusCode();

    // Add the received response message
    UpdateResponseOnReceived(IMessage::PAGEMESSAGE_SEND, piScc);

    // Handle the response to MESSAGE request ...
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

    CloseConnection(IMessage::PAGEMESSAGE_SEND);

    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        PostMessage(AMSG_PAGE_MESSAGE_DELIVERED, 0, 0);
    }
    else
    {
        PostMessage(AMSG_PAGE_MESSAGE_DELIVERY_FAILED, 0, 0);
    }
}

PRIVATE VIRTUAL void PageMessage::NotifySipError(
        IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    const SipMethod& objMethod = piSc->GetMethod();

    (void)nCode;
    (void)strMessage;

    if (!objMethod.Equals(SipMethod::MESSAGE))
    {
        piSc->Close();
        return;
    }

    CloseConnection(IMessage::PAGEMESSAGE_SEND);

    SetState(STATE_UNSENT);
    PostMessage(AMSG_PAGE_MESSAGE_DELIVERY_FAILED, 0, 0);
}

PRIVATE
void PageMessage::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("PageMessage :: %s to %s", StateToString(m_nState), StateToString(nState), 0);

    m_nState = nState;
}

PRIVATE GLOBAL const IMS_CHAR* PageMessage::StateToString(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case STATE_UNSENT:
            return "STATE_UNSENT";
        case STATE_SENT:
            return "STATE_SENT";
        case STATE_RECEIVED:
            return "STATE_RECEIVED";
        default:
            return "__INVALID__";
    }
}
