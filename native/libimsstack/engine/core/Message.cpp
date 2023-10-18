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

#include "private/AppConfig.h"

#include "ISipConnection.h"
#include "ISipHeader.h"
#include "Message.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "base/Ims.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
Message::Message(IN AppConfig* pAppConfig, IN IMS_SINT32 nState) :
        m_pAppConfig(pAppConfig),
        m_nState(nState),
        m_piSipMsg(IMS_NULL),
        m_objBodyParts(ImsList<MessageBodyPart*>())
{
}

PUBLIC VIRTUAL Message::~Message()
{
    for (IMS_UINT32 i = 0; i < m_objBodyParts.GetSize(); ++i)
    {
        MessageBodyPart* pBodyPart = m_objBodyParts.GetAt(i);

        if (pBodyPart != IMS_NULL)
        {
            delete pBodyPart;
        }
    }

    m_objBodyParts.Clear();

    if (m_piSipMsg != IMS_NULL)
    {
        m_piSipMsg->Destroy();
        m_piSipMsg = IMS_NULL;
    }
}

PUBLIC
ISipMessageBodyPart* Message::CreateBodyPartEx()
{
    ISipMessageBodyPart* piBodyPart = m_piSipMsg->CreateBodyPart();

    if (piBodyPart == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);
        return IMS_NULL;
    }

    MessageBodyPart* pBodyPart = new MessageBodyPart(this, piBodyPart);

    if (pBodyPart == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);
        return IMS_NULL;
    }

    if (!m_objBodyParts.Append(pBodyPart))
    {
        delete pBodyPart;

        Ims::SetLastError(ImsError::LIST_OPERATION_FAILED);
        return IMS_NULL;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return piBodyPart;
}

PUBLIC
void Message::UpdateSentMessage(IN ISipMessage* piSipMsg)
{
    if (m_nState != STATE_UNSENT)
    {
        IMS_TRACE_E(0, "Invalid state (%d)", m_nState, 0, 0);
        return;
    }

    if (m_piSipMsg != IMS_NULL)
    {
        m_piSipMsg->Destroy();
        m_piSipMsg = IMS_NULL;
    }

    m_piSipMsg = DYNAMIC_CAST(ISipMessage*, piSipMsg->Clone());

    if (m_piSipMsg == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);
        return;
    }

    // Update a message body parts if present (except for SDP message body)
    ImsList<ISipMessageBodyPart*> objSipBodyParts = m_piSipMsg->GetBodyParts();

    if (!objSipBodyParts.IsEmpty())
    {
        if (objSipBodyParts.GetSize() != m_objBodyParts.GetSize())
        {
            // fatal error ????
        }

        for (IMS_UINT32 i = 0; i < m_objBodyParts.GetSize(); ++i)
        {
            MessageBodyPart* pBodyPart = m_objBodyParts.GetAt(i);

            pBodyPart->SetBodyPart(objSipBodyParts.GetAt(i));
        }
    }

    m_nState = STATE_SENT;
}

PUBLIC GLOBAL Message* Message::CreateMessage(IN Message* pMessage)
{
    if (pMessage == IMS_NULL)
    {
        return IMS_NULL;
    }

    Message* pNewMessage = new Message(pMessage->m_pAppConfig, pMessage->m_nState);

    if (pNewMessage == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);
        return IMS_NULL;
    }

    if (pMessage->m_piSipMsg != IMS_NULL)
    {
        pNewMessage->m_piSipMsg = pMessage->m_piSipMsg->Clone();

        if (pNewMessage->m_piSipMsg == IMS_NULL)
        {
            delete pNewMessage;

            Ims::SetLastError(ImsError::NO_MEMORY);
            return IMS_NULL;
        }

        if (!pNewMessage->CreateBodyParts())
        {
            delete pNewMessage;
            return IMS_NULL;
        }
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pNewMessage;
}

PUBLIC GLOBAL Message* Message::CreateUnsentMessage(IN AppConfig* pAppConfig, IN IMS_BOOL bRequest)
{
    Message* pMessage = new Message(pAppConfig, STATE_UNSENT);

    if (pMessage == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);
        return IMS_NULL;
    }

    if (bRequest)
    {
        pMessage->m_piSipMsg = SipParsingHelper::CreateMessage(ISipMessage::TYPE_REQUEST);
    }
    else
    {
        pMessage->m_piSipMsg = SipParsingHelper::CreateMessage(ISipMessage::TYPE_RESPONSE);
    }

    if (pMessage->m_piSipMsg == IMS_NULL)
    {
        delete pMessage;

        Ims::SetLastError(ImsError::NO_MEMORY);
        return IMS_NULL;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pMessage;
}

PUBLIC GLOBAL Message* Message::CreateReceivedMessage(
        IN AppConfig* pAppConfig, IN ISipMessage* piSipMsg)
{
    Message* pMessage = new Message(pAppConfig, STATE_RECEIVED);

    if (pMessage == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);
        return IMS_NULL;
    }

    if (piSipMsg != IMS_NULL)
    {
        pMessage->m_piSipMsg = piSipMsg->Clone();

        if (pMessage->m_piSipMsg == IMS_NULL)
        {
            delete pMessage;

            Ims::SetLastError(ImsError::NO_MEMORY);
            return IMS_NULL;
        }

        if (!pMessage->CreateBodyParts())
        {
            delete pMessage;
            return IMS_NULL;
        }
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pMessage;
}

PUBLIC GLOBAL const IMS_CHAR* Message::GetMessageType(IN IMS_SINT32 nServiceMethod)
{
    switch (nServiceMethod)
    {
        case CAPABILITIES_QUERY:
            return "CAPABILITIES_QUERY";
        case PAGEMESSAGE_SEND:
            return "PAGEMESSAGE_SEND";
        case PUBLICATION_PUBLISH:
            return "PUBLICATION_PUBLISH";
        case PUBLICATION_UNPUBLISH:
            return "PUBLICATION_UNPUBLISH";
        case REFERENCE_REFER:
            return "REFERENCE_REFER";
        case SESSION_START:
            return "SESSION_START";
        case SESSION_UPDATE:
            return "SESSION_UPDATE";
        case SESSION_TERMINATE:
            return "SESSION_TERMINATE";
        case SUBSCRIPTION_SUBSCRIBE:
            return "SUBSCRIPTION_SUBSCRIBE";
        case SUBSCRIPTION_UNSUBSCRIBE:
            return "SUBSCRIPTION_UNSUBSCRIBE";
        case SUBSCRIPTION_POLL:
            return "SUBSCRIPTION_POLL";
        case SESSION_ACK:
            return "SESSION_ACK";
        case SESSION_PRACK:
            return "SESSION_PRACK";
        case SESSION_EARLY_UPDATE:
            return "SESSION_EARLY_UPDATE";
        case SESSION_CANCEL:
            return "SESSION_CANCEL";
        case SESSION_STALE_UPDATE:
            return "SESSION_STALE_UPDATE";
        default:
            return "UNKNOWN_SERVICE_METHOD";
    }
}

PRIVATE VIRTUAL IMessageBodyPart* Message::CreateBodyPart()
{
    if (m_nState != STATE_UNSENT)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Invalid state (%d)", m_nState, 0, 0);
        return IMS_NULL;
    }

    ISipMessageBodyPart* piBodyPart = m_piSipMsg->CreateBodyPart();

    if (piBodyPart == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);
        return IMS_NULL;
    }

    MessageBodyPart* pBodyPart = new MessageBodyPart(this, piBodyPart);

    if (pBodyPart == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);
        return IMS_NULL;
    }

    if (!m_objBodyParts.Append(pBodyPart))
    {
        delete pBodyPart;

        Ims::SetLastError(ImsError::LIST_OPERATION_FAILED);
        return IMS_NULL;
    }

    return pBodyPart;
}

PRIVATE VIRTUAL ImsList<IMessageBodyPart*> Message::GetBodyParts() const
{
    ImsList<IMessageBodyPart*> objIBodyParts;

    for (IMS_UINT32 i = 0; i < m_objBodyParts.GetSize(); ++i)
    {
        objIBodyParts.Append(m_objBodyParts.GetAt(i));
    }

    return objIBodyParts;
}

PRIVATE VIRTUAL IMS_RESULT Message::AddHeader(IN const AString& strName, IN const AString& strValue)
{
    AString strHName = strName.Trim();

    if (strHName.GetLength() == 0)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (strValue.IsNULL())
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    ISipHeader* piHeader = SipParsingHelper::CreateHeader(strHName);

    if (piHeader == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (IsReadOnlyHeader(piHeader->GetType(), strHName) ||
            IsInaccessibleHeader(piHeader->GetType(), strHName))
    {
        IMS_TRACE_E(0, "Header (%s: %s) is read-only or inaccessible.", strHName.GetStr(),
                strValue.GetStr(), 0);

        piHeader->Destroy();

        Ims::SetLastError(ImsError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    if (!m_pAppConfig->IsHeaderWritable(strHName))
    {
        IMS_TRACE_E(0, "Header (%s: %s) is not writable", strHName.GetStr(), strValue.GetStr(), 0);

        piHeader->Destroy();

        Ims::SetLastError(ImsError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    if (m_piSipMsg->AddHeader(piHeader->GetType(), strValue, strHName) != IMS_SUCCESS)
    {
        piHeader->Destroy();

        Ims::SetLastError(ImsError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    piHeader->Destroy();

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL ImsList<AString> Message::GetHeaders(IN const AString& strName) const
{
    AString strHName = strName.Trim();

    if (strHName.GetLength() == 0)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return ImsList<AString>();
    }

    ISipHeader* piHeader = SipParsingHelper::CreateHeader(strHName);

    if (piHeader == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return ImsList<AString>();
    }

    IMS_SINT32 nHType = piHeader->GetType();

    if (IsInaccessibleHeader(nHType, strHName))
    {
        IMS_TRACE_E(0, "Header (%s) is inaccessible.", strHName.GetStr(), 0, 0);

        piHeader->Destroy();

        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return ImsList<AString>();
    }

    if (!m_pAppConfig->IsHeaderReadable(strHName))
    {
        IMS_TRACE_E(0, "Header (%s) is not readable.", strHName.GetStr(), 0, 0);

        piHeader->Destroy();

        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return ImsList<AString>();
    }

    piHeader->Destroy();

    Ims::SetLastError(ImsError::NO_ERROR);

    return m_piSipMsg->GetHeaders(nHType, strHName);
}

PRIVATE
IMS_BOOL Message::CreateBodyParts()
{
    if (m_piSipMsg == IMS_NULL)
    {
        return IMS_TRUE;
    }

    ImsList<ISipMessageBodyPart*> objSipBodyParts = m_piSipMsg->GetBodyParts();

    if (!objSipBodyParts.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objSipBodyParts.GetSize(); ++i)
        {
            MessageBodyPart* pBodyPart = new MessageBodyPart(this, objSipBodyParts.GetAt(i));

            if (pBodyPart == IMS_NULL)
            {
                Ims::SetLastError(ImsError::NO_MEMORY);
                return IMS_FALSE;
            }

            if (!m_objBodyParts.Append(pBodyPart))
            {
                delete pBodyPart;

                Ims::SetLastError(ImsError::LIST_OPERATION_FAILED);
                return IMS_FALSE;
            }
        }
    }

    // In case of 2xx response to OPTIONS,
    // If SDP body is present, it needs to be added the body parts.
    if (m_piSipMsg->GetType() == ISipMessage::TYPE_RESPONSE)
    {
        const SipMethod& objMethod = m_piSipMsg->GetMethod();
        IMS_SINT32 nStatusCode = m_piSipMsg->GetStatusCode();

        if (objMethod.Equals(SipMethod::OPTIONS) && SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            ISipMessageBodyPart* piSipBodyPart = m_piSipMsg->GetSdpBodyPart();

            if (piSipBodyPart != IMS_NULL)
            {
                MessageBodyPart* pBodyPart = new MessageBodyPart(this, piSipBodyPart);

                if (pBodyPart == IMS_NULL)
                {
                    Ims::SetLastError(ImsError::NO_MEMORY);
                    return IMS_FALSE;
                }

                if (!m_objBodyParts.Prepend(pBodyPart))
                {
                    delete pBodyPart;

                    Ims::SetLastError(ImsError::LIST_OPERATION_FAILED);
                    return IMS_FALSE;
                }
            }
        }
    }

    return IMS_TRUE;
}

PRIVATE GLOBAL IMS_BOOL Message::IsInaccessibleHeader(
        IN IMS_SINT32 nHType, IN const AString& /*strHName*/)
{
    switch (nHType)
    {
        case ISipHeader::AUTHENTICATION_INFO:  // FALL-THROUGH
        case ISipHeader::AUTHORIZATION:        // FALL-THROUGH
        case ISipHeader::MAX_FORWARDS:         // FALL-THROUGH
        case ISipHeader::MIN_EXPIRES:          // FALL-THROUGH
        case ISipHeader::PROXY_AUTHENTICATE:   // FALL-THROUGH
        case ISipHeader::PROXY_AUTHORIZATION:  // FALL-THROUGH
        case ISipHeader::RECORD_ROUTE:         // FALL-THROUGH
        case ISipHeader::SECURITY_CLIENT:      // FALL-THROUGH
        case ISipHeader::SECURITY_SERVER:      // FALL-THROUGH
        case ISipHeader::SECURITY_VERIFY:      // FALL-THROUGH
        case ISipHeader::SERVICE_ROUTE:        // FALL-THROUGH
        case ISipHeader::VIA:
            return IMS_TRUE;
        default:
            break;
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL IMS_BOOL Message::IsReadOnlyHeader(IN IMS_SINT32 nHType, IN const AString& strHName)
{
    (void)strHName;

    switch (nHType)
    {
        case ISipHeader::ACCEPT_CONTACT:         // FALL-THROUGH
        case ISipHeader::CALL_ID:                // FALL-THROUGH
        case ISipHeader::CONTACT_NORMAL:         // FALL-THROUGH
        case ISipHeader::CONTACT_WILDCARD:       // FALL-THROUGH
        case ISipHeader::CONTACT_ANY:            // FALL-THROUGH
        case ISipHeader::CONTENT_LENGTH:         // FALL-THROUGH
        case ISipHeader::CSEQ:                   // FALL-THROUGH
        case ISipHeader::EVENT:                  // FALL-THROUGH
        case ISipHeader::FROM:                   // FALL-THROUGH
        case ISipHeader::P_ACCESS_NETWORK_INFO:  // FALL-THROUGH
        case ISipHeader::P_ASSERTED_IDENTITY:    // FALL-THROUGH
        case ISipHeader::P_ASSOCIATED_URI:       // FALL-THROUGH
        case ISipHeader::P_PREFERRED_IDENTITY:   // FALL-THROUGH
        case ISipHeader::RACK:                   // FALL-THROUGH
        case ISipHeader::REFER_TO:               // FALL-THROUGH
        case ISipHeader::REFERRED_BY:            // FALL-THROUGH
        case ISipHeader::REPLACES:               // FALL-THROUGH
        case ISipHeader::RSEQ:                   // FALL-THROUGH
        case ISipHeader::TO:                     // FALL-THROUGH
        case ISipHeader::WWW_AUTHENTICATE:
            return IMS_TRUE;
        default:
            break;
    }

    return IMS_FALSE;
}
