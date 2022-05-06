/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090729  toastops@                 Created
    </table>

    Description
*/
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipDialog.h"
#include "ISipServerConnection.h"
#include "SipStatusCode.h"

#include "SipConfigProxy.h"
#include "SipParsingHelper.h"
#include "SipParameter.h"

#include "base/IMS.h"
#include "Service.h"
#include "ServiceMethod.h"

__IMS_TRACE_TAG_IMS_CORE__;

class PreviousMessage
{
public:
    PreviousMessage(IN Message* pRequest_);
    ~PreviousMessage();

private:
    PreviousMessage(IN const PreviousMessage& objRHS);
    PreviousMessage& operator=(IN const PreviousMessage& objRHS);

private:
    friend class ServiceMethod;

    ISipConnection* piSC;
    Message* pRequest;
    IMSList<Message*> objResponses;
};

PUBLIC
PreviousMessage::PreviousMessage(IN Message* pRequest_) :
        piSC(IMS_NULL),
        pRequest(pRequest_),
        objResponses(IMSList<Message*>())
{
}

PUBLIC
PreviousMessage::~PreviousMessage()
{
    //---------------------------------------------------------------------------------------------

    if (piSC != IMS_NULL)
    {
        piSC->Close();
        piSC = IMS_NULL;
    }

    if (pRequest != IMS_NULL)
    {
        delete pRequest;
        pRequest = IMS_NULL;
    }

    if (!objResponses.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objResponses.GetSize(); ++i)
        {
            Message* pResponse = objResponses.GetAt(i);

            if (pResponse != IMS_NULL)
            {
                delete pResponse;
                pResponse = IMS_NULL;
            }
        }

        objResponses.Clear();
    }
}

PUBLIC
ServiceMethod::ServiceMethod(IN Service* pService_) :
        Method(),
        pService(pService_),
        pNextRequest(IMS_NULL),
        pNextResponse(IMS_NULL),
        objPreviousMessages(IMSMap<IMS_SINT32, PreviousMessage*>())
{
}

PUBLIC VIRTUAL ServiceMethod::~ServiceMethod()
{
    //---------------------------------------------------------------------------------------------

    if (!objPreviousMessages.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objPreviousMessages.GetSize(); ++i)
        {
            PreviousMessage* pPreviousMessage = objPreviousMessages.GetValueAt(i);

            if (pPreviousMessage != IMS_NULL)
                delete pPreviousMessage;
        }

        objPreviousMessages.Clear();
    }

    if (pNextRequest != IMS_NULL)
    {
        delete pNextRequest;
        pNextRequest = IMS_NULL;
    }

    if (pNextResponse != IMS_NULL)
    {
        delete pNextResponse;
        pNextResponse = IMS_NULL;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL void ServiceMethod::Destroy()
{
    //---------------------------------------------------------------------------------------------

    // Clean up the resources

    // Abort the ongoing SIP connection
    if (!objPreviousMessages.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objPreviousMessages.GetSize(); ++i)
        {
            PreviousMessage* pPreviousMessage = objPreviousMessages.GetValueAt(i);

            if (pPreviousMessage != IMS_NULL)
            {
                if (pPreviousMessage->piSC != IMS_NULL)
                {
                    pPreviousMessage->piSC->Close();
                    pPreviousMessage->piSC = IMS_NULL;
                }
            }
        }
    }

    Method::Destroy();
}

/*

Remarks

*/
PUBLIC
Message* ServiceMethod::GetNextRequest()
{
    //---------------------------------------------------------------------------------------------

    if (pNextRequest == IMS_NULL)
    {
        pNextRequest = Message::CreateUnsentMessage(pService->GetAppConfig(), IMS_TRUE);
    }

    return pNextRequest;
}

/*

Remarks

*/
PUBLIC
Message* ServiceMethod::GetNextResponse()
{
    //---------------------------------------------------------------------------------------------

    if (pNextResponse == IMS_NULL)
    {
        pNextResponse = Message::CreateUnsentMessage(pService->GetAppConfig(), IMS_FALSE);
    }

    return pNextResponse;
}

/*

Remarks

*/
PUBLIC
Message* ServiceMethod::GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const
{
    IMS_SLONG nIndex = objPreviousMessages.GetIndexOfKey(nServiceMethod);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    PreviousMessage* pPreviousMessage = objPreviousMessages.GetValueAt(nIndex);

    IMS::SetLastError(IMSError::NO_ERROR);

    return pPreviousMessage->pRequest;
}

/*

Remarks

*/
PUBLIC
Message* ServiceMethod::GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const
{
    IMS_SLONG nIndex = objPreviousMessages.GetIndexOfKey(nServiceMethod);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    PreviousMessage* pPreviousMessage = objPreviousMessages.GetValueAt(nIndex);

    if (pPreviousMessage->objResponses.IsEmpty())
    {
        IMS::SetLastError(IMSError::NOT_FOUND);
        return IMS_NULL;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return pPreviousMessage->objResponses.GetAt(pPreviousMessage->objResponses.GetSize() - 1);
}

/*

Remarks

*/
PUBLIC
IMSList<Message*> ServiceMethod::GetPreviousResponses(IN IMS_SINT32 nServiceMethod) const
{
    IMS_SLONG nIndex = objPreviousMessages.GetIndexOfKey(nServiceMethod);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        IMS::SetLastError(IMSError::ILLEGAL_ARGUMENT);
        return IMSList<Message*>();
    }

    PreviousMessage* pPreviousMessage = objPreviousMessages.GetValueAt(nIndex);

    IMS::SetLastError(IMSError::NO_ERROR);

    return pPreviousMessage->objResponses;
}

/*

Remarks

*/
PUBLIC
IMSList<AString> ServiceMethod::GetRemoteUserId() const
{
    //---------------------------------------------------------------------------------------------

    return GetRemoteUserIds();
}

/*

Remarks
 MULTI_SUBS
*/
PROTECTED VIRTUAL const AString& ServiceMethod::GetSubscriberId() const
{
    //---------------------------------------------------------------------------------------------

    return pService->GetSubscriberId();
}

/*

Remarks

*/
PROTECTED
void ServiceMethod::ClearConnection(IN IMS_SINT32 nServiceMethod)
{
    IMS_SLONG nIndex = objPreviousMessages.GetIndexOfKey(nServiceMethod);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        return;
    }

    PreviousMessage* pPreviousMessage = objPreviousMessages.GetValueAt(nIndex);

    if (pPreviousMessage->piSC == IMS_NULL)
    {
        IMS_TRACE_D("SIP connection (ServiceMethod: %s) is already cleared",
                Message::GetMessageType(nServiceMethod), 0, 0);
        return;
    }

    pPreviousMessage->piSC = IMS_NULL;
}

/*

Remarks

*/
PROTECTED
void ServiceMethod::CloseConnection(IN IMS_SINT32 nServiceMethod)
{
    IMS_SLONG nIndex = objPreviousMessages.GetIndexOfKey(nServiceMethod);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        return;
    }

    PreviousMessage* pPreviousMessage = objPreviousMessages.GetValueAt(nIndex);

    if (pPreviousMessage->piSC == IMS_NULL)
    {
        IMS_TRACE_D("SIP connection (ServiceMethod: %s) is already closed",
                Message::GetMessageType(nServiceMethod), 0, 0);
        return;
    }

    pPreviousMessage->piSC->Close();
    pPreviousMessage->piSC = IMS_NULL;
}

/*

Remarks

*/
PROTECTED
void ServiceMethod::CopyPreviousMessage(
        IN IMS_SINT32 nServiceMethod_From, IN IMS_SINT32 nServiceMethod_To)
{
    IMS_SLONG nIndex = objPreviousMessages.GetIndexOfKey(nServiceMethod_From);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        return;
    }

    PreviousMessage* pPreviousMessage = objPreviousMessages.GetValueAt(nIndex);

    if (pPreviousMessage == IMS_NULL)
    {
        RemovePreviousMessage(nServiceMethod_To);
        return;
    }

    PreviousMessage* pNewPreviousMessage = new PreviousMessage(IMS_NULL);

    if (pNewPreviousMessage == IMS_NULL)
    {
        RemovePreviousMessage(nServiceMethod_To);
        return;
    }

    pNewPreviousMessage->pRequest = Message::CreateMessage(pPreviousMessage->pRequest);

    for (IMS_UINT32 i = 0; i < pPreviousMessage->objResponses.GetSize(); ++i)
    {
        pNewPreviousMessage->objResponses.Append(
                Message::CreateMessage(pPreviousMessage->objResponses.GetAt(i)));
    }

    RemovePreviousMessage(nServiceMethod_To);

    if (!objPreviousMessages.Add(nServiceMethod_To, pNewPreviousMessage))
    {
        delete pNewPreviousMessage;

        IMS_TRACE_E(0, "Adding the previous message (%s) failed",
                Message::GetMessageType(nServiceMethod_To), 0, 0);
        return;
    }
}

/*

Remarks

*/
PROTECTED
ISipClientConnection* ServiceMethod::CreateCancelConnection(IN ISipClientConnection* piSCC)
{
    ISipClientConnection* piCancelSCC = pService->CreateCancelConnection(piSCC);

    if (piCancelSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating CANCEL connection failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (pNextRequest != IMS_NULL)
    {
        ISipMessage* piSIPMsg = piCancelSCC->GetMessage();

        // Set the headers and body parts if application already sets
        if (piSIPMsg->CopyHeadersAndBodyParts(pNextRequest->GetMessage()) != IMS_SUCCESS)
        {
            IMS::SetLastError(IMSError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Copying SIP headers and message body parts failed", 0, 0, 0);

            piCancelSCC->Close();

            return IMS_NULL;
        }
    }

    piCancelSCC->SetErrorListener(this);
    piCancelSCC->SetListener(this);

    return piCancelSCC;
}

/*

Remarks

*/
PROTECTED
ISipClientConnection* ServiceMethod::CreateConnection(IN CONST SipMethod& objMethod)
{
    ISipClientConnection* piSCC = pService->CreateConnection(
            GetUserAOR(), GetRemoteUserAOR(), objMethod, IsPrivacyRequested());

    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(IMS::GetLastError(), "Creating SIP connection (%s) failed",
                objMethod.ToString().GetStr(), 0, 0);
        return IMS_NULL;
    }

    if (pNextRequest != IMS_NULL)
    {
        ISipMessage* piSIPMsg = piSCC->GetMessage();
        const AString strSecAgree(Sip::STR_SEC_AGREE);
        IMS_BOOL bSecAgreeRequired = piSIPMsg->IsOptionRequired(strSecAgree);
        const AString strGRUU(Sip::STR_GRUU);
        IMS_BOOL bGRUUSupported = piSIPMsg->IsOptionSupported(strGRUU);

        // Set the headers and body parts if application already sets
        if (piSIPMsg->CopyHeadersAndBodyParts(pNextRequest->GetMessage()) != IMS_SUCCESS)
        {
            IMS::SetLastError(IMSError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Copying SIP headers and message body parts failed", 0, 0, 0);

            piSCC->Close();

            return IMS_NULL;
        }

        if (bSecAgreeRequired && !piSIPMsg->IsOptionRequired(strSecAgree))
        {
            piSIPMsg->AddHeader(ISipHeader::REQUIRE, strSecAgree);
        }

        if (bGRUUSupported && !piSIPMsg->IsOptionSupported(strGRUU))
        {
            piSIPMsg->AddHeader(ISipHeader::SUPPORTED, strGRUU);
        }
    }

    piSCC->SetErrorListener(this);
    piSCC->SetListener(this);

    return piSCC;
}

/*

Remarks

*/
PROTECTED
ISipClientConnection* ServiceMethod::CreateConnection(
        IN ISipDialog* piDialog, IN CONST SipMethod& objMethod)
{
    ISipClientConnection* piSCC =
            pService->CreateConnection(piDialog, objMethod, IsPrivacyRequested());

    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(IMS::GetLastError(), "Creating SIP connection (%s) within a dialog failed",
                objMethod.ToString().GetStr(), 0, 0);
        return IMS_NULL;
    }

    if (pNextRequest != IMS_NULL)
    {
        ISipMessage* piSIPMsg = piSCC->GetMessage();
        const AString strSecAgree(Sip::STR_SEC_AGREE);
        IMS_BOOL bSecAgreeRequired = piSIPMsg->IsOptionRequired(strSecAgree);
        const AString strGRUU(Sip::STR_GRUU);
        IMS_BOOL bGRUUSupported = piSIPMsg->IsOptionSupported(strGRUU);

        // Set the headers and body parts if application already sets
        if (piSIPMsg->CopyHeadersAndBodyParts(pNextRequest->GetMessage()) != IMS_SUCCESS)
        {
            IMS::SetLastError(IMSError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Copying SIP headers and message body parts failed", 0, 0, 0);

            piSCC->Close();

            return IMS_NULL;
        }

        if (bSecAgreeRequired && !piSIPMsg->IsOptionRequired(strSecAgree))
        {
            piSIPMsg->AddHeader(ISipHeader::REQUIRE, strSecAgree);
        }

        if (bGRUUSupported && !piSIPMsg->IsOptionSupported(strGRUU))
        {
            piSIPMsg->AddHeader(ISipHeader::SUPPORTED, strGRUU);
        }
    }

    piSCC->SetErrorListener(this);
    piSCC->SetListener(this);

    return piSCC;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL ServiceMethod::CreateResponse(IN_OUT ISipServerConnection* piSSC,
        IN IMS_SINT32 nStatusCode, IN CONST AString& strPhrase /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    if (!pService->CreateResponse(piSSC, nStatusCode, strPhrase, IsPrivacyRequested(IMS_FALSE)))
    {
        IMS_TRACE_E(0, "Initializing SIP response (%s) failed",
                piSSC->GetMethod().ToString().GetStr(), 0, 0);
        return IMS_FALSE;
    }

    if (pNextResponse != IMS_NULL)
    {
        ISipMessage* piSIPMsg = piSSC->GetMessage();

        const AString strGRUU(Sip::STR_GRUU);
        IMS_BOOL bGRUUSupported = piSIPMsg->IsOptionSupported(strGRUU);

        // Set the headers and body parts if application already sets
        if (piSIPMsg->CopyHeadersAndBodyParts(pNextResponse->GetMessage()) != IMS_SUCCESS)
        {
            IMS::SetLastError(IMSError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Copying SIP headers and message body parts failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (bGRUUSupported && !piSIPMsg->IsOptionSupported(strGRUU))
        {
            piSIPMsg->AddHeader(ISipHeader::SUPPORTED, strGRUU);
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED
ISipClientConnection* ServiceMethod::GetClientConnection(IN IMS_SINT32 nServiceMethod) const
{
    //---------------------------------------------------------------------------------------------

    return DYNAMIC_CAST(ISipClientConnection*, GetConnection(nServiceMethod));
}

/*

Remarks

*/
PROTECTED
ISipServerConnection* ServiceMethod::GetServerConnection(IN IMS_SINT32 nServiceMethod) const
{
    //---------------------------------------------------------------------------------------------

    return DYNAMIC_CAST(ISipServerConnection*, GetConnection(nServiceMethod));
}

/*

Remarks

*/
PROTECTED
Service* ServiceMethod::GetService() const
{
    //---------------------------------------------------------------------------------------------

    return pService;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL ServiceMethod::IsPrivacyRequested(IN IMS_BOOL bRequest /* = IMS_TRUE */) const
{
    if (SipConfigProxy::IsGruuConfigured(GetSlotId(), pService->GetSIPProfile()))
    {
        ISipMessage* piSIPMsg = IMS_NULL;

        // Outgoing request message
        if (bRequest)
        {
            if (pNextRequest == IMS_NULL)
            {
                return IMS_FALSE;
            }

            piSIPMsg = pNextRequest->GetMessage();
        }
        // Outgoing response message
        else
        {
            if (pNextResponse == IMS_NULL)
            {
                return IMS_FALSE;
            }

            piSIPMsg = pNextResponse->GetMessage();
        }

        if (piSIPMsg == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (!piSIPMsg->IsHeaderPresent(ISipHeader::PRIVACY))
        {
            return IMS_FALSE;
        }

        IMSList<AString> objHeaders = piSIPMsg->GetHeaders(ISipHeader::PRIVACY);

        for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
        {
            ISipHeader* piHeader =
                    SipParsingHelper::CreateHeader(ISipHeader::PRIVACY, objHeaders.GetAt(i));

            if (piHeader == IMS_NULL)
                continue;

            const SipParameter* pParameter = piHeader->GetParameter("id");

            if (pParameter != IMS_NULL)
            {
                IMS_TRACE_D("Privacy is requested...", 0, 0, 0);

                piHeader->Destroy();
                return IMS_TRUE;
            }

            piHeader->Destroy();
        }
    }
    else
    {
        (void)bRequest;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL ServiceMethod::IsServiceOpen() const
{
    //---------------------------------------------------------------------------------------------

    return pService->IsImsConnected();
}

/*

Remarks

*/
PROTECTED
IMS_BOOL ServiceMethod::RemovePreviousMessage(IN IMS_SINT32 nServiceMethod)
{
    IMS_SLONG nIndex = objPreviousMessages.GetIndexOfKey(nServiceMethod);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        return IMS_TRUE;
    }

    PreviousMessage* pPreviousMessage = objPreviousMessages.GetValueAt(nIndex);

    if (pPreviousMessage == IMS_NULL)
    {
        objPreviousMessages.RemoveAt(nIndex);
        return IMS_TRUE;
    }

    if ((pPreviousMessage->pRequest != IMS_NULL) && (pPreviousMessage->objResponses.IsEmpty()) &&
            (pPreviousMessage->piSC != IMS_NULL))
    {
        // Do not update the previous message; The transaction is not completed.
        IMS_TRACE_D("The transaction of the service method (%s) is not completed",
                Message::GetMessageType(nServiceMethod), 0, 0);
        return IMS_FALSE;
    }

    delete pPreviousMessage;

    objPreviousMessages.RemoveAt(nIndex);

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL ServiceMethod::SendNUpdateRequest(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC)
{
    //---------------------------------------------------------------------------------------------

    return SendNUpdateRequestEx(nServiceMethod, piSC, MESSAGE_CLASS_NORMAL);
}

/*

Remarks

*/
PROTECTED
IMS_BOOL ServiceMethod::SendNUpdateResponse(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC)
{
    //---------------------------------------------------------------------------------------------

    return SendNUpdateResponseEx(nServiceMethod, piSC, MESSAGE_CLASS_NORMAL);
}

/*

Remarks

*/
PROTECTED
IMS_BOOL ServiceMethod::SendNUpdateRequestEx(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC,
        IN IMS_SINT32 nMessageClass /* = MESSAGE_CLASS_NORMAL */)
{
    // SIP_MESSAGE_MEDIATOR
    (void)AdjustMessage(piSC->GetMessage(), nMessageClass);

    if (piSC->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending a request message (%s) failed - Error (%d)",
                Message::GetMessageType(nServiceMethod), SipError::GetLastError(), 0);
        return IMS_FALSE;
    }

    return UpdateRequestOnSent(nServiceMethod, piSC);
}

/*

Remarks

*/
PROTECTED
IMS_BOOL ServiceMethod::SendNUpdateResponseEx(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC,
        IN IMS_SINT32 nMessageClass /* = MESSAGE_CLASS_NORMAL */)
{
    //---------------------------------------------------------------------------------------------

    // SIP_MESSAGE_MEDIATOR
    (void)AdjustMessage(piSC->GetMessage(), nMessageClass);

    if (piSC->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending a response message failed - Error (%d)", SipError::GetLastError(),
                0, 0);
        return IMS_FALSE;
    }

    return UpdateResponseOnSent(nServiceMethod, piSC);
}

/*

Remarks

*/
PROTECTED
void ServiceMethod::UpdateConnection(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC)
{
    IMS_SLONG nIndex = objPreviousMessages.GetIndexOfKey(nServiceMethod);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        return;
    }

    PreviousMessage* pPreviousMessage = objPreviousMessages.GetValueAt(nIndex);

    if (pPreviousMessage->piSC != IMS_NULL)
    {
        if (pPreviousMessage->piSC == piSC)
        {
            // Do not update the SIP connection
            return;
        }

        pPreviousMessage->piSC->Close();
    }

    pPreviousMessage->piSC = piSC;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL ServiceMethod::UpdateRequestOnReceived(
        IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC)
{
    Message* pMessage =
            Message::CreateReceivedMessage(pService->GetAppConfig(), piSC->GetMessage());

    //---------------------------------------------------------------------------------------------

    if (pMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a received message(%s) failed",
                Message::GetMessageType(nServiceMethod), 0, 0);
        return IMS_FALSE;
    }

    if (RemovePreviousMessage(nServiceMethod))
    {
        if (!SetPreviousRequest(nServiceMethod, pMessage, piSC))
        {
            delete pMessage;

            IMS_TRACE_E(0, "Setting the request (%s) failed",
                    Message::GetMessageType(nServiceMethod), 0, 0);
            return IMS_FALSE;
        }
    }
    else
    {
        delete pMessage;
        IMS::SetLastError(IMSError::ALREADY_EXISTS);

        IMS_TRACE_E(
                0, "MESSAGE (%s) ALREADY EXISTS", Message::GetMessageType(nServiceMethod), 0, 0);
        return IMS_FALSE;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL ServiceMethod::UpdateRequestOnSent(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC)
{
    //---------------------------------------------------------------------------------------------

    if (piSC == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Make sure that Message MUST be created before SEND operation.
    if (GetNextRequest() == IMS_NULL)
    {
        IMS_TRACE_E(0, "Message for next request (%s) is not created",
                Message::GetMessageType(nServiceMethod), 0, 0);
        return IMS_FALSE;
    }

    pNextRequest->UpdateSentMessage(piSC->GetMessage());

    if (RemovePreviousMessage(nServiceMethod))
    {
        if (!SetPreviousRequest(nServiceMethod, pNextRequest, piSC))
        {
            delete pNextRequest;
            pNextRequest = IMS_NULL;

            return IMS_FALSE;
        }
    }
    else
    {
        // When the SIP request is sent by the engine,
        // it will not be handled by the normal service method.
        // So, the listener of SCC will be set to SCCListener object.
        ISipClientConnection* piSCC = DYNAMIC_CAST(ISipClientConnection*, piSC);

        if ((piSCC != IMS_NULL) && (!piSCC->GetMethod().Equals(SipMethod::ACK)))
        {
            SCCListener* pListener = new SCCListener();

            piSCC->SetErrorListener(pListener);
            piSCC->SetListener(pListener);
        }

        delete pNextRequest;
    }

    pNextRequest = IMS_NULL;

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL ServiceMethod::UpdateResponseOnReceived(
        IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC)
{
    Message* pMessage =
            Message::CreateReceivedMessage(pService->GetAppConfig(), piSC->GetMessage());

    //---------------------------------------------------------------------------------------------

    if (pMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a received message(%s) failed",
                Message::GetMessageType(nServiceMethod), 0, 0);

        return IMS_FALSE;
    }

    if (!AddPreviousResponse(nServiceMethod, pMessage))
    {
        IMS_TRACE_E(0, "Setting the response (%s) failed", Message::GetMessageType(nServiceMethod),
                0, 0);

        delete pMessage;
        return IMS_FALSE;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL ServiceMethod::UpdateResponseOnSent(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC)
{
    //---------------------------------------------------------------------------------------------

    // Make sure that Message MUST be created before SEND operation.
    if (GetNextResponse() == IMS_NULL)
    {
        IMS_TRACE_E(0, "Message for next response is not created", 0, 0, 0);
        return IMS_FALSE;
    }

    // Update a sent SIP message
    pNextResponse->UpdateSentMessage(piSC->GetMessage());

    if (!AddPreviousResponse(nServiceMethod, pNextResponse))
    {
        delete pNextResponse;
    }

    pNextResponse = IMS_NULL;

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL ServiceMethod::AddPreviousResponse(IN IMS_SINT32 nServiceMethod, IN Message* pMessage)
{
    IMS_SLONG nIndex = objPreviousMessages.GetIndexOfKey(nServiceMethod);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        IMS_TRACE_E(0, "The previous message for method (%s) does not exist",
                Message::GetMessageType(nServiceMethod), 0, 0);
        return IMS_FALSE;
    }

    PreviousMessage* pPreviousMessage = objPreviousMessages.GetValueAt(nIndex);

    if (!pPreviousMessage->objResponses.Append(pMessage))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL ServiceMethod::SetPreviousRequest(
        IN IMS_SINT32 nServiceMethod, IN Message* pMessage, IN ISipConnection* piSC)
{
    IMS_SLONG nIndex = objPreviousMessages.GetIndexOfKey(nServiceMethod);

    //---------------------------------------------------------------------------------------------

    if (nIndex >= 0)
    {
        IMS_TRACE_E(0, "The previous message for method (%s) already exists",
                Message::GetMessageType(nServiceMethod), 0, 0);
        return IMS_FALSE;
    }

    PreviousMessage* pPreviousMessage = new PreviousMessage(pMessage);

    if (pPreviousMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating the previous message (%s) failed",
                Message::GetMessageType(nServiceMethod), 0, 0);
        return IMS_FALSE;
    }

    if (!objPreviousMessages.Add(nServiceMethod, pPreviousMessage))
    {
        pPreviousMessage->pRequest = IMS_NULL;
        delete pPreviousMessage;

        IMS_TRACE_E(0, "Adding the previous message (%s) failed",
                Message::GetMessageType(nServiceMethod), 0, 0);
        return IMS_FALSE;
    }

    // Store the SIP connection for the response message
    pPreviousMessage->piSC = piSC;

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
ISipConnection* ServiceMethod::GetConnection(IN IMS_SINT32 nServiceMethod) const
{
    IMS_SLONG nIndex = objPreviousMessages.GetIndexOfKey(nServiceMethod);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        IMS::SetLastError(IMSError::NO_SIP_CONNECTION);
        return IMS_NULL;
    }

    PreviousMessage* pPreviousMessage = objPreviousMessages.GetValueAt(nIndex);

    if (pPreviousMessage->piSC == IMS_NULL)
    {
        IMS_TRACE_D("SIP connection (ServiceMethod: %s) is not found",
                Message::GetMessageType(nServiceMethod), 0, 0);

        IMS::SetLastError(IMSError::NO_SIP_CONNECTION);
        return IMS_NULL;
    }

    IMS::SetLastError(IMSError::NO_ERROR);

    return pPreviousMessage->piSC;
}
