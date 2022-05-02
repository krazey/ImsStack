/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090529  lovil@                    Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ISipConnection.h"
#include "ISipMessage.h"
#include "ISipHeader.h"
#include "SipHeaderName.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "base/IMS.h"
#include "private/AppConfig.h"
#include "Message.h"

__IMS_TRACE_TAG_IMS_CORE__;



PUBLIC
Message::Message(IN AppConfig *pAppConfig_, IN IMS_SINT32 nState_)
    : pAppConfig(pAppConfig_)
    , nState(nState_)
    , piSIPMessage(IMS_NULL)
    , objBodyParts(IMSList<MessageBodyPart*>())
{
}

PUBLIC VIRTUAL
Message::~Message()
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); ++i)
    {
        MessageBodyPart *pBodyPart = objBodyParts.GetAt(i);

        if (pBodyPart != IMS_NULL)
        {
            delete pBodyPart;
        }
    }

    objBodyParts.Clear();

    if (piSIPMessage != IMS_NULL)
    {
        piSIPMessage->Destroy();
        piSIPMessage = IMS_NULL;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
ISIPMessage* Message::GetMessage() const
{
    //---------------------------------------------------------------------------------------------

    return piSIPMessage;
}

/*

Remarks

*/
PUBLIC
ISIPMessageBodyPart* Message::CreateBodyPartEx()
{
    ISIPMessageBodyPart *piBodyPart = piSIPMessage->CreateBodyPart();

    //---------------------------------------------------------------------------------------------

    if (piBodyPart == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);
        return IMS_NULL;
    }

    MessageBodyPart *pBodyPart = new MessageBodyPart(this, piBodyPart);

    if (pBodyPart == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);
        return IMS_NULL;
    }

    if (!objBodyParts.Append(pBodyPart))
    {
        delete pBodyPart;

        IMS::SetLastError(IMSError::LIST_OPERATION_FAILED);
        return IMS_NULL;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return piBodyPart;
}

/*

Remarks

*/
PUBLIC
void Message::UpdateSentMessage(IN ISIPMessage *piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (nState != STATE_UNSENT)
    {
        IMS_TRACE_E(0, "Invalid state (%d)", nState, 0, 0);
        return;
    }

    if (piSIPMessage != IMS_NULL)
    {
        piSIPMessage->Destroy();
        piSIPMessage = IMS_NULL;
    }

    piSIPMessage = DYNAMIC_CAST(ISIPMessage*, piSIPMsg->Clone());

    if (piSIPMessage == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);
        return;
    }

    // Update a message body parts if present (except for SDP message body)
    IMSList<ISIPMessageBodyPart*> objIBodyParts = piSIPMessage->GetBodyParts();

    if (!objIBodyParts.IsEmpty())
    {
        if (objIBodyParts.GetSize() != objBodyParts.GetSize())
        {
            // fatal error ????
        }

        for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); ++i)
        {
            MessageBodyPart *pBodyPart = objBodyParts.GetAt(i);

            pBodyPart->SetBodyPart(objIBodyParts.GetAt(i));
        }
    }

    nState = STATE_SENT;
}

/*

Remarks

*/
PUBLIC GLOBAL
Message* Message::CreateMessage(IN Message *pMessage)
{
    //---------------------------------------------------------------------------------------------

    if (pMessage == IMS_NULL)
    {
        return IMS_NULL;
    }

    Message *pNewMessage = new Message(pMessage->pAppConfig, pMessage->nState);

    if (pNewMessage == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);
        return IMS_NULL;
    }

    if (pMessage->piSIPMessage != IMS_NULL)
    {
        pNewMessage->piSIPMessage = pMessage->piSIPMessage->Clone();

        if (pNewMessage->piSIPMessage == IMS_NULL)
        {
            delete pNewMessage;

            IMS::SetLastError(IMSError::NO_MEMORY);
            return IMS_NULL;
        }

        if (!pNewMessage->CreateBodyParts())
        {
            delete pNewMessage;
            return IMS_NULL;
        }
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return pNewMessage;
}

/*

Remarks

*/
PUBLIC GLOBAL
Message* Message::CreateUnsentMessage(IN AppConfig *pAppConfig, IN IMS_BOOL bRequest)
{
    Message *pMessage = new Message(pAppConfig, STATE_UNSENT);

    //---------------------------------------------------------------------------------------------

    if (pMessage == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);
        return IMS_NULL;
    }

    if (bRequest)
        pMessage->piSIPMessage = SIPParsingHelper::CreateMessage(ISIPMessage::TYPE_REQUEST);
    else
        pMessage->piSIPMessage = SIPParsingHelper::CreateMessage(ISIPMessage::TYPE_RESPONSE);

    if (pMessage->piSIPMessage == IMS_NULL)
    {
        delete pMessage;

        IMS::SetLastError(IMSError::NO_MEMORY);
        return IMS_NULL;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return pMessage;
}

/*

Remarks

*/
PUBLIC GLOBAL
Message* Message::CreateReceivedMessage(IN AppConfig *pAppConfig, IN ISIPMessage *piSIPMsg)
{
    Message *pMessage = new Message(pAppConfig, STATE_RECEIVED);

    //---------------------------------------------------------------------------------------------

    if (pMessage == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);
        return IMS_NULL;
    }

    if (piSIPMsg != IMS_NULL)
    {
        pMessage->piSIPMessage = piSIPMsg->Clone();

        if (pMessage->piSIPMessage == IMS_NULL)
        {
            delete pMessage;

            IMS::SetLastError(IMSError::NO_MEMORY);
            return IMS_NULL;
        }

        if (!pMessage->CreateBodyParts())
        {
            delete pMessage;
            return IMS_NULL;
        }
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return pMessage;
}

PUBLIC GLOBAL
const IMS_CHAR* Message::GetMessageType(IN IMS_SINT32 nServiceMethod)
{
    //---------------------------------------------------------------------------------------------

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

/*

Remarks

*/
PRIVATE VIRTUAL
IMessageBodyPart* Message::CreateBodyPart()
{
    //---------------------------------------------------------------------------------------------

    if (nState != STATE_UNSENT)
    {
        IMS::SetLastError(IMSError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Invalid state (%d)", nState, 0, 0);
        return IMS_NULL;
    }

    ISIPMessageBodyPart *piBodyPart = piSIPMessage->CreateBodyPart();

    if (piBodyPart == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);
        return IMS_NULL;
    }

    MessageBodyPart *pBodyPart = new MessageBodyPart(this, piBodyPart);

    if (pBodyPart == IMS_NULL)
    {
        IMS::SetLastError(IMSError::NO_MEMORY);
        return IMS_NULL;
    }

    if (!objBodyParts.Append(pBodyPart))
    {
        delete pBodyPart;

        IMS::SetLastError(IMSError::LIST_OPERATION_FAILED);
        return IMS_NULL;
    }

    return pBodyPart;
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMSList<IMessageBodyPart*> Message::GetBodyParts() const
{
    IMSList<IMessageBodyPart*> objIBodyParts;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); ++i)
    {
        objIBodyParts.Append(objBodyParts.GetAt(i));
    }

    return objIBodyParts;
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_RESULT Message::AddHeader(IN CONST AString &strName, IN CONST AString &strValue)
{
    AString strHName = strName.Trim();

    //---------------------------------------------------------------------------------------------

    if (strHName.GetLength() == 0)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (strValue.IsNULL())
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    ISIPHeader *piHeader = SIPParsingHelper::CreateHeader(strHName);

    if (piHeader == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    if (IsReadOnlyHeader(piHeader->GetType(), strHName)
            || IsInaccessibleHeader(piHeader->GetType(), strHName))
    {
        IMS_TRACE_E(0, "Header (%s: %s) is read-only or inaccessible.",
                strHName.GetStr(), strValue.GetStr(), 0);

        piHeader->Destroy();

        IMS::SetLastError(IMSError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    if (!pAppConfig->IsHeaderWritable(strHName))
    {
        IMS_TRACE_E(0, "Header (%s: %s) is not writable",
                strHName.GetStr(), strValue.GetStr(), 0);

        piHeader->Destroy();

        IMS::SetLastError(IMSError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    if (piSIPMessage->AddHeader(piHeader->GetType(), strValue, strHName) != IMS_SUCCESS)
    {
        piHeader->Destroy();

        IMS::SetLastError(IMSError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    piHeader->Destroy();

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMSList<AString> Message::GetHeaders(IN CONST AString &strName) const
{
    AString strHName = strName.Trim();

    //---------------------------------------------------------------------------------------------

    if (strHName.GetLength() == 0)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMSList<AString>();
    }

    ISIPHeader *piHeader = SIPParsingHelper::CreateHeader(strHName);

    if (piHeader == IMS_NULL)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMSList<AString>();
    }

    IMS_SINT32 nHType = piHeader->GetType();

    if (IsInaccessibleHeader(nHType, strHName))
    {
        IMS_TRACE_E(0, "Header (%s) is inaccessible.", strHName.GetStr(), 0, 0);

        piHeader->Destroy();

        IMS::SetLastError(IMSError::INVALID_OPERATION);
        return IMSList<AString>();
    }

    if (!pAppConfig->IsHeaderReadable(strHName))
    {
        IMS_TRACE_E(0, "Header (%s) is not readable.", strHName.GetStr(), 0, 0);

        piHeader->Destroy();

        IMS::SetLastError(IMSError::INVALID_OPERATION);
        return IMSList<AString>();
    }

    piHeader->Destroy();

    IMS::SetLastError(IMSError::NO_ERROR);

    return piSIPMessage->GetHeaders(nHType, strHName);
}

/*

Remarks

*/
PRIVATE VIRTUAL
const SIPMethod& Message::GetMethod() const
{
    //---------------------------------------------------------------------------------------------

    return piSIPMessage->GetMethod();
}

/*

Remarks

*/
PRIVATE VIRTUAL
const AString& Message::GetReasonPhrase() const
{
    //---------------------------------------------------------------------------------------------

    return piSIPMessage->GetReasonPhrase();
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_SINT32 Message::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_SINT32 Message::GetStatusCode() const
{
    //---------------------------------------------------------------------------------------------

    return piSIPMessage->GetStatusCode();
}

/*

Remarks

*/
PRIVATE
IMS_BOOL Message::CreateBodyParts()
{
    //---------------------------------------------------------------------------------------------

    if (piSIPMessage == IMS_NULL)
    {
        return IMS_TRUE;
    }

    IMSList<ISIPMessageBodyPart*> objSIPBodyParts = piSIPMessage->GetBodyParts();

    if (!objSIPBodyParts.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objSIPBodyParts.GetSize(); ++i)
        {
            MessageBodyPart *pBodyPart = new MessageBodyPart(this, objSIPBodyParts.GetAt(i));

            if (pBodyPart == IMS_NULL)
            {
                IMS::SetLastError(IMSError::NO_MEMORY);
                return IMS_FALSE;
            }

            if (!objBodyParts.Append(pBodyPart))
            {
                delete pBodyPart;

                IMS::SetLastError(IMSError::LIST_OPERATION_FAILED);
                return IMS_FALSE;
            }
        }
    }

    // In case of 2xx response to OPTIONS,
    // If SDP body is present, it needs to be added the body parts.
    if (piSIPMessage->GetType() == ISIPMessage::TYPE_RESPONSE)
    {
        const SIPMethod &objMethod = piSIPMessage->GetMethod();
        IMS_SINT32 nStatusCode = piSIPMessage->GetStatusCode();

        if (objMethod.Equals(SIPMethod::OPTIONS)
                && SIPStatusCode::IsFinalSuccess(nStatusCode))
        {
            ISIPMessageBodyPart *piSIPBodyPart = piSIPMessage->GetSDPBodyPart();

            if (piSIPBodyPart != IMS_NULL)
            {
                MessageBodyPart *pBodyPart = new MessageBodyPart(this, piSIPBodyPart);

                if (pBodyPart == IMS_NULL)
                {
                    IMS::SetLastError(IMSError::NO_MEMORY);
                    return IMS_FALSE;
                }

                if (!objBodyParts.Prepend(pBodyPart))
                {
                    delete pBodyPart;

                    IMS::SetLastError(IMSError::LIST_OPERATION_FAILED);
                    return IMS_FALSE;
                }
            }
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE GLOBAL
IMS_BOOL Message::IsInaccessibleHeader(IN IMS_SINT32 nHType, IN CONST AString &strHName)
{
    //---------------------------------------------------------------------------------------------

    switch (nHType)
    {
    case ISIPHeader::AUTHORIZATION:
    case ISIPHeader::MAX_FORWARDS:
    case ISIPHeader::MIN_EXPIRES:
    case ISIPHeader::PROXY_AUTHENTICATE:
    case ISIPHeader::PROXY_AUTHORIZATION:
    case ISIPHeader::RECORD_ROUTE:
    case ISIPHeader::SECURITY_CLIENT:
    case ISIPHeader::SECURITY_SERVER:
    case ISIPHeader::SECURITY_VERIFY:
    case ISIPHeader::SERVICE_ROUTE:
    case ISIPHeader::VIA:
        return IMS_TRUE;

    case ISIPHeader::UNKNOWN:
        switch (strHName[0])
        {
        case 'a':
        case 'A':
            if (strHName.EqualsIgnoreCase(SIPHeaderName::AUTHENTICATION_INFO))
                return IMS_TRUE;
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE GLOBAL
IMS_BOOL Message::IsReadOnlyHeader(IN IMS_SINT32 nHType, IN CONST AString &strHName)
{
    //---------------------------------------------------------------------------------------------

    (void) strHName;

    switch (nHType)
    {
    case ISIPHeader::ACCEPT_CONTACT:
    case ISIPHeader::CALL_ID:
    case ISIPHeader::CONTACT_NORMAL:
    case ISIPHeader::CONTACT_WILDCARD:
    case ISIPHeader::CONTACT_ANY:
    case ISIPHeader::CONTENT_LENGTH:
    case ISIPHeader::CSEQ:
    case ISIPHeader::EVENT:
    case ISIPHeader::FROM:
    case ISIPHeader::P_ACCESS_NETWORK_INFO:
    case ISIPHeader::P_ASSERTED_IDENTITY:
    case ISIPHeader::P_ASSOCIATED_URI:
    case ISIPHeader::P_PREFERRED_IDENTITY:
    case ISIPHeader::RACK:
    case ISIPHeader::REFER_TO:
    case ISIPHeader::REFERRED_BY:
    case ISIPHeader::REPLACES:
    case ISIPHeader::RSEQ:
    case ISIPHeader::TO:
    case ISIPHeader::WWW_AUTHENTICATE:
        return IMS_TRUE;

    default:
        break;
    }

    return IMS_FALSE;
}
