/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100506  hwangoo.park@             Created
    </table>

    Description
*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "private/SipConfigV.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipServerConnection.h"
#include "SipStatusCode.h"
#include "SipHeaderName.h"
#include "SipConfigProxy.h"
#include "base/Ims.h"
#include "Service.h"
#include "Message.h"
#include "IOnPageMessageListener.h"
#include "PageMessage.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
PageMessage::PageMessage(IN Service* pService) :
        ServiceMethod(pService),
        nState(STATE_UNSENT),
        piListener(IMS_NULL)
{
}

PUBLIC VIRTUAL PageMessage::~PageMessage() {}

/*

Remarks

*/
PUBLIC VIRTUAL void PageMessage::Destroy()
{
    //---------------------------------------------------------------------------------------------

    ServiceMethod::Destroy();
}

/*

Remarks

*/
PUBLIC
const ByteArray& PageMessage::GetContent() const
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_RECEIVED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return ByteArray::ConstNull();
    }

    Message* pMessage = GetPreviousRequest(IMessage::PAGEMESSAGE_SEND);

    if (pMessage == IMS_NULL)
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return ByteArray::ConstNull();
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    ISipMessage* piSIPMsg = pMessage->GetMessage();
    IMSList<ISipMessageBodyPart*> objBodyParts = piSIPMsg->GetBodyParts();

    if (objBodyParts.IsEmpty())
    {
        return ByteArray::ConstNull();
    }

    ISipMessageBodyPart* piBodyPart = objBodyParts.GetAt(0);

    if (piBodyPart == IMS_NULL)
    {
        return ByteArray::ConstNull();
    }

    return piBodyPart->GetContent();
}

/*

Remarks

*/
PUBLIC
AString PageMessage::GetContentType() const
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_RECEIVED)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return AString::ConstNull();
    }

    Message* pMessage = GetPreviousRequest(IMessage::PAGEMESSAGE_SEND);

    if (pMessage == IMS_NULL)
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return AString::ConstNull();
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    ISipMessage* piSIPMsg = pMessage->GetMessage();
    IMSList<ISipMessageBodyPart*> objBodyParts = piSIPMsg->GetBodyParts();

    if (objBodyParts.IsEmpty())
    {
        return AString::ConstNull();
    }

    ISipMessageBodyPart* piBodyPart = objBodyParts.GetAt(0);

    if (piBodyPart == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return piBodyPart->GetHeader(ISipMessageBodyPart::CONTENT_TYPE);
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 PageMessage::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT PageMessage::Send(IN CONST ByteArray& objContent, IN CONST AString& strContentType)
{
    //---------------------------------------------------------------------------------------------

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
    ISipClientConnection* piSCC = CreateConnection(SipMethod(SipMethod::MESSAGE));

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    ISipMessage* piSIPMsg = piSCC->GetMessage();

    // Set Content and Content-Type header
    if (!objContent.IsNULL() && !strContentType.IsNULL())
    {
        ISipMessageBodyPart* piBodyPart = piSIPMsg->CreateBodyPart();

        if (piBodyPart == IMS_NULL)
        {
            piSCC->Close();

            Ims::SetLastError(ImsError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        // Set headers
        piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, strContentType);

        // Set content
        piBodyPart->SetContent(objContent);
    }

    if (!SendNUpdateRequest(IMessage::PAGEMESSAGE_SEND, piSCC))
    {
        piSCC->Close();
        return IMS_FAILURE;
    }

    // Update the state
    SetState(STATE_SENT);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
void PageMessage::SetListener(IN IOnPageMessageListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT PageMessage::Accept(IN IMS_SINT32 nStatusCode /* = 200 */)
{
    //---------------------------------------------------------------------------------------------

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

    ISipServerConnection* piSSC = GetServerConnection(IMessage::PAGEMESSAGE_SEND);

    if (piSSC == IMS_NULL)
        return IMS_FAILURE;

    // Send a 200 OK to MESSAGE request
    if (CreateResponse(piSSC, nStatusCode) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to MESSAGE request failed", 0, 0, 0);
    }

    if (!SendNUpdateResponse(IMessage::PAGEMESSAGE_SEND, piSSC))
    {
        CloseConnection(IMessage::PAGEMESSAGE_SEND);

        IMS_TRACE_E(0, "Sending a response to MESSAGE request failed!", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Close the incoming SIP connection
    CloseConnection(IMessage::PAGEMESSAGE_SEND);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT PageMessage::Reject(IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nRetryAfter /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

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

    ISipServerConnection* piSSC = GetServerConnection(IMessage::PAGEMESSAGE_SEND);

    if (piSSC == IMS_NULL)
        return IMS_FAILURE;

    // Send a failure final response to OPTIONS request
    if (CreateResponse(piSSC, nStatusCode) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Initializing the response to MESSAGE request failed", 0, 0, 0);
    }

    // Now, the variable, piSIPMsg will be used for MESSAGE response message

    if (nRetryAfter > 0)
    {
        ISipMessage* piSIPMsg = piSSC->GetMessage();
        AString strRetryAfter;

        strRetryAfter.SetNumber(nRetryAfter);

        piSIPMsg->AddHeader(ISipHeader::RETRY_AFTER_SEC, strRetryAfter);
    }

    if (!SendNUpdateResponse(IMessage::PAGEMESSAGE_SEND, piSSC))
    {
        CloseConnection(IMessage::PAGEMESSAGE_SEND);

        IMS_TRACE_E(0, "Sending a response to MESSAGE request failed!", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Close the incoming SIP connection
    CloseConnection(IMessage::PAGEMESSAGE_SEND);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL PageMessage::DispatchMessage(IN IMSMSG& objMSG)
{
    //---------------------------------------------------------------------------------------------

    switch (objMSG.GetName())
    {
        case AMSG_PAGE_MESSAGE_RECEIVED:
            GetService()->HandlePageMessageReceived(this);
            return IMS_TRUE;

        case AMSG_PAGE_MESSAGE_DELIVERED:
            if (piListener != IMS_NULL)
            {
                piListener->OnPageMessage_Delivered(this);
            }
            return IMS_TRUE;

        case AMSG_PAGE_MESSAGE_DELIVERY_FAILED:
            if (piListener != IMS_NULL)
            {
                piListener->OnPageMessage_DeliveryFailed(this);
            }
            return IMS_TRUE;

        default:
            break;
    }

    return EngineActivity::DispatchMessage(objMSG);
}

/*

Remarks

*/
// IMS_AUTH_SIP_DIGEST
PRIVATE VIRTUAL IMS_BOOL PageMessage::SendRequestToChallenge(IN ISipClientConnection* piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Clear the connection to preserve the SIP connection
    ClearConnection(IMessage::PAGEMESSAGE_SEND);

    if (!SendNUpdateRequestEx(IMessage::PAGEMESSAGE_SEND, piSCC, MESSAGE_CLASS_RESUBMIT))
    {
        // Revert the SIP connection
        UpdateConnection(IMessage::PAGEMESSAGE_SEND, piSCC);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL PageMessage::NotifySIPRequest(IN ISipServerConnection* piSSC)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("PageMessage - MESSAGE REQUEST RECEIVED ...", 0, 0, 0);

    if (!UpdateRequestOnReceived(IMessage::PAGEMESSAGE_SEND, piSSC))
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
    if (piSSC->InitResponse(SipStatusCode::SC_200) != IMS_SUCCESS)
    {
        CloseConnection(IMessage::PAGEMESSAGE_SEND);

        IMS_TRACE_E(0, "Initializing a response to MESSAGE request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!SendNUpdateResponse(IMessage::PAGEMESSAGE_SEND, piSSC))
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

/*

Remarks

*/
PRIVATE VIRTUAL void PageMessage::NotifySIPResponse(IN ISipClientConnection* piSCC)
{
    IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

    //---------------------------------------------------------------------------------------------

    // Add the received response message
    UpdateResponseOnReceived(IMessage::PAGEMESSAGE_SEND, piSCC);

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
        if (RespondToChallenge(piSCC))
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

/*

Remarks

*/
PRIVATE VIRTUAL void PageMessage::NotifySIPError(
        IN ISipConnection* piSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage)
{
    const SipMethod& objMethod = piSC->GetMethod();

    //---------------------------------------------------------------------------------------------

    (void)nCode;
    (void)strMessage;

    if (!objMethod.Equals(SipMethod::MESSAGE))
    {
        piSC->Close();
        return;
    }

    CloseConnection(IMessage::PAGEMESSAGE_SEND);

    SetState(STATE_UNSENT);
    PostMessage(AMSG_PAGE_MESSAGE_DELIVERY_FAILED, 0, 0);
}

/*

Remarks

*/
PRIVATE
void PageMessage::SetState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("PageMessage :: %s to %s", StateToString(this->nState), StateToString(nState), 0);

    this->nState = nState;
}

/*

Remarks

*/
PRIVATE GLOBAL const IMS_CHAR* PageMessage::StateToString(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

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
