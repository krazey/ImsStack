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

#include "ISipDialog.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipServerConnection.h"
#include "ServiceMethod.h"
#include "SipConfigProxy.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "base/Ims.h"

__IMS_TRACE_TAG_IMS_CORE__;

class PreviousMessage
{
public:
    explicit PreviousMessage(IN Message* pRequest);
    ~PreviousMessage();

    PreviousMessage(IN const PreviousMessage&) = delete;
    PreviousMessage& operator=(IN const PreviousMessage&) = delete;

private:
    friend class ServiceMethod;

    ISipConnection* m_piSc;
    Message* m_pRequest;
    ImsList<Message*> m_objResponses;
};

PUBLIC
PreviousMessage::PreviousMessage(IN Message* pRequest) :
        m_piSc(IMS_NULL),
        m_pRequest(pRequest),
        m_objResponses(ImsList<Message*>())
{
}

PUBLIC
PreviousMessage::~PreviousMessage()
{
    if (m_piSc != IMS_NULL)
    {
        m_piSc->Close();
        m_piSc = IMS_NULL;
    }

    if (m_pRequest != IMS_NULL)
    {
        delete m_pRequest;
        m_pRequest = IMS_NULL;
    }

    if (!m_objResponses.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objResponses.GetSize(); ++i)
        {
            Message* pResponse = m_objResponses.GetAt(i);

            if (pResponse != IMS_NULL)
            {
                delete pResponse;
            }
        }

        m_objResponses.Clear();
    }
}

PUBLIC
ServiceMethod::ServiceMethod(IN Service* pService) :
        Method(),
        m_pService(pService),
        m_pNextRequest(IMS_NULL),
        m_pNextResponse(IMS_NULL),
        m_objPreviousMessages(ImsMap<IMS_SINT32, PreviousMessage*>())
{
}

PUBLIC VIRTUAL ServiceMethod::~ServiceMethod()
{
    if (!m_objPreviousMessages.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objPreviousMessages.GetSize(); ++i)
        {
            PreviousMessage* pPreviousMessage = m_objPreviousMessages.GetValueAt(i);

            if (pPreviousMessage != IMS_NULL)
            {
                delete pPreviousMessage;
            }
        }

        m_objPreviousMessages.Clear();
    }

    if (m_pNextRequest != IMS_NULL)
    {
        delete m_pNextRequest;
        m_pNextRequest = IMS_NULL;
    }

    if (m_pNextResponse != IMS_NULL)
    {
        delete m_pNextResponse;
        m_pNextResponse = IMS_NULL;
    }
}

PUBLIC VIRTUAL void ServiceMethod::Destroy()
{
    // Clean up the resources

    // Abort the ongoing SIP connection
    if (!m_objPreviousMessages.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objPreviousMessages.GetSize(); ++i)
        {
            PreviousMessage* pPreviousMessage = m_objPreviousMessages.GetValueAt(i);

            if (pPreviousMessage != IMS_NULL)
            {
                if (pPreviousMessage->m_piSc != IMS_NULL)
                {
                    pPreviousMessage->m_piSc->Close();
                    pPreviousMessage->m_piSc = IMS_NULL;
                }
            }
        }
    }

    Method::Destroy();
}

PUBLIC
Message* ServiceMethod::GetNextRequest()
{
    if (m_pNextRequest == IMS_NULL)
    {
        m_pNextRequest = Message::CreateUnsentMessage(m_pService->GetAppConfig(), IMS_TRUE);
    }

    return m_pNextRequest;
}

PUBLIC
Message* ServiceMethod::GetNextResponse()
{
    if (m_pNextResponse == IMS_NULL)
    {
        m_pNextResponse = Message::CreateUnsentMessage(m_pService->GetAppConfig(), IMS_FALSE);
    }

    return m_pNextResponse;
}

PUBLIC
Message* ServiceMethod::GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const
{
    IMS_SLONG nIndex = m_objPreviousMessages.GetIndexOfKey(nServiceMethod);

    if (nIndex < 0)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    const PreviousMessage* pPreviousMessage = m_objPreviousMessages.GetValueAt(nIndex);

    Ims::SetLastError(ImsError::NO_ERROR);

    return pPreviousMessage->m_pRequest;
}

PUBLIC
Message* ServiceMethod::GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const
{
    IMS_SLONG nIndex = m_objPreviousMessages.GetIndexOfKey(nServiceMethod);

    if (nIndex < 0)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    PreviousMessage* pPreviousMessage = m_objPreviousMessages.GetValueAt(nIndex);

    if (pPreviousMessage->m_objResponses.IsEmpty())
    {
        Ims::SetLastError(ImsError::NOT_FOUND);
        return IMS_NULL;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pPreviousMessage->m_objResponses.GetAt(pPreviousMessage->m_objResponses.GetSize() - 1);
}

PUBLIC
ImsList<Message*> ServiceMethod::GetPreviousResponses(IN IMS_SINT32 nServiceMethod) const
{
    IMS_SLONG nIndex = m_objPreviousMessages.GetIndexOfKey(nServiceMethod);

    if (nIndex < 0)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return ImsList<Message*>();
    }

    const PreviousMessage* pPreviousMessage = m_objPreviousMessages.GetValueAt(nIndex);

    Ims::SetLastError(ImsError::NO_ERROR);

    return pPreviousMessage->m_objResponses;
}

PROTECTED
void ServiceMethod::ClearConnection(IN IMS_SINT32 nServiceMethod)
{
    IMS_SLONG nIndex = m_objPreviousMessages.GetIndexOfKey(nServiceMethod);

    if (nIndex < 0)
    {
        return;
    }

    PreviousMessage* pPreviousMessage = m_objPreviousMessages.GetValueAt(nIndex);

    if (pPreviousMessage->m_piSc == IMS_NULL)
    {
        IMS_TRACE_D("SIP connection (ServiceMethod: %s) is already cleared",
                Message::GetMessageType(nServiceMethod), 0, 0);
        return;
    }

    pPreviousMessage->m_piSc = IMS_NULL;
}

PROTECTED
void ServiceMethod::CloseConnection(IN IMS_SINT32 nServiceMethod)
{
    IMS_SLONG nIndex = m_objPreviousMessages.GetIndexOfKey(nServiceMethod);

    if (nIndex < 0)
    {
        return;
    }

    PreviousMessage* pPreviousMessage = m_objPreviousMessages.GetValueAt(nIndex);

    if (pPreviousMessage->m_piSc == IMS_NULL)
    {
        IMS_TRACE_D("SIP connection (ServiceMethod: %s) is already closed",
                Message::GetMessageType(nServiceMethod), 0, 0);
        return;
    }

    pPreviousMessage->m_piSc->Close();
    pPreviousMessage->m_piSc = IMS_NULL;
}

PROTECTED
void ServiceMethod::CopyPreviousMessage(
        IN IMS_SINT32 nSrcServiceMethod, IN IMS_SINT32 nDstServiceMethod)
{
    IMS_SLONG nIndex = m_objPreviousMessages.GetIndexOfKey(nSrcServiceMethod);

    if (nIndex < 0)
    {
        return;
    }

    PreviousMessage* pPreviousMessage = m_objPreviousMessages.GetValueAt(nIndex);

    if (pPreviousMessage == IMS_NULL)
    {
        RemovePreviousMessage(nDstServiceMethod);
        return;
    }

    PreviousMessage* pNewPreviousMessage = new PreviousMessage(IMS_NULL);

    if (pNewPreviousMessage == IMS_NULL)
    {
        RemovePreviousMessage(nDstServiceMethod);
        return;
    }

    pNewPreviousMessage->m_pRequest = Message::CreateMessage(pPreviousMessage->m_pRequest);

    for (IMS_UINT32 i = 0; i < pPreviousMessage->m_objResponses.GetSize(); ++i)
    {
        pNewPreviousMessage->m_objResponses.Append(
                Message::CreateMessage(pPreviousMessage->m_objResponses.GetAt(i)));
    }

    RemovePreviousMessage(nDstServiceMethod);
    m_objPreviousMessages.Add(nDstServiceMethod, pNewPreviousMessage);
}

PROTECTED
ISipClientConnection* ServiceMethod::CreateCancelConnection(IN ISipClientConnection* piScc)
{
    ISipClientConnection* piCancelScc = m_pService->CreateCancelConnection(piScc);

    if (piCancelScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating CANCEL connection failed", 0, 0, 0);
        return IMS_NULL;
    }

    if (m_pNextRequest != IMS_NULL)
    {
        ISipMessage* piSipMsg = piCancelScc->GetMessage();

        // Set the headers and body parts if application already sets
        if (piSipMsg->CopyHeadersAndBodyParts(m_pNextRequest->GetMessage()) != IMS_SUCCESS)
        {
            Ims::SetLastError(ImsError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Copying SIP headers and message body parts failed", 0, 0, 0);

            piCancelScc->Close();

            return IMS_NULL;
        }
    }

    piCancelScc->SetErrorListener(this);
    piCancelScc->SetListener(this);

    return piCancelScc;
}

PROTECTED
ISipClientConnection* ServiceMethod::CreateConnection(IN const SipMethod& objMethod)
{
    ISipClientConnection* piScc = m_pService->CreateConnection(
            GetUserAor(), GetRemoteUserAor(), objMethod, IsPrivacyRequested());

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(Ims::GetLastError(), "Creating SIP connection (%s) failed",
                objMethod.ToString().GetStr(), 0, 0);
        return IMS_NULL;
    }

    if (m_pNextRequest != IMS_NULL)
    {
        ISipMessage* piSipMsg = piScc->GetMessage();
        const AString strSecAgree(Sip::STR_SEC_AGREE);
        IMS_BOOL bSecAgreeRequired = piSipMsg->IsOptionRequired(strSecAgree);
        const AString strGruu(Sip::STR_GRUU);
        IMS_BOOL bGruuSupported = piSipMsg->IsOptionSupported(strGruu);

        // Set the headers and body parts if application already sets
        if (piSipMsg->CopyHeadersAndBodyParts(m_pNextRequest->GetMessage()) != IMS_SUCCESS)
        {
            Ims::SetLastError(ImsError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Copying SIP headers and message body parts failed", 0, 0, 0);

            piScc->Close();

            return IMS_NULL;
        }

        if (bSecAgreeRequired && !piSipMsg->IsOptionRequired(strSecAgree))
        {
            piSipMsg->AddHeader(ISipHeader::REQUIRE, strSecAgree);
        }

        if (bGruuSupported && !piSipMsg->IsOptionSupported(strGruu))
        {
            piSipMsg->AddHeader(ISipHeader::SUPPORTED, strGruu);
        }
    }

    piScc->SetErrorListener(this);
    piScc->SetListener(this);

    return piScc;
}

PROTECTED
ISipClientConnection* ServiceMethod::CreateConnection(
        IN ISipDialog* piDialog, IN const SipMethod& objMethod)
{
    ISipClientConnection* piScc =
            m_pService->CreateConnection(piDialog, objMethod, IsPrivacyRequested());

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(Ims::GetLastError(), "Creating SIP connection (%s) within a dialog failed",
                objMethod.ToString().GetStr(), 0, 0);
        return IMS_NULL;
    }

    if (m_pNextRequest != IMS_NULL)
    {
        ISipMessage* piSipMsg = piScc->GetMessage();
        const AString strSecAgree(Sip::STR_SEC_AGREE);
        IMS_BOOL bSecAgreeRequired = piSipMsg->IsOptionRequired(strSecAgree);
        const AString strGruu(Sip::STR_GRUU);
        IMS_BOOL bGruuSupported = piSipMsg->IsOptionSupported(strGruu);

        // Set the headers and body parts if application already sets
        if (piSipMsg->CopyHeadersAndBodyParts(m_pNextRequest->GetMessage()) != IMS_SUCCESS)
        {
            Ims::SetLastError(ImsError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Copying SIP headers and message body parts failed", 0, 0, 0);

            piScc->Close();

            return IMS_NULL;
        }

        if (bSecAgreeRequired && !piSipMsg->IsOptionRequired(strSecAgree))
        {
            piSipMsg->AddHeader(ISipHeader::REQUIRE, strSecAgree);
        }

        if (bGruuSupported && !piSipMsg->IsOptionSupported(strGruu))
        {
            piSipMsg->AddHeader(ISipHeader::SUPPORTED, strGruu);
        }
    }

    piScc->SetErrorListener(this);
    piScc->SetListener(this);

    return piScc;
}

PROTECTED
IMS_BOOL ServiceMethod::CreateResponse(IN_OUT ISipServerConnection* piSsc,
        IN IMS_SINT32 nStatusCode, IN const AString& strPhrase /*= AString::ConstNull()*/)
{
    if (!m_pService->CreateResponse(piSsc, nStatusCode, strPhrase, IsPrivacyRequested(IMS_FALSE)))
    {
        IMS_TRACE_E(0, "Initializing SIP response (%s) failed",
                piSsc->GetMethod().ToString().GetStr(), 0, 0);
        return IMS_FALSE;
    }

    if (m_pNextResponse != IMS_NULL)
    {
        ISipMessage* piSipMsg = piSsc->GetMessage();

        const AString strGruu(Sip::STR_GRUU);
        IMS_BOOL bGruuSupported = piSipMsg->IsOptionSupported(strGruu);

        // Set the headers and body parts if application already sets
        if (piSipMsg->CopyHeadersAndBodyParts(m_pNextResponse->GetMessage()) != IMS_SUCCESS)
        {
            Ims::SetLastError(ImsError::GENERAL_ERROR);

            IMS_TRACE_E(0, "Copying SIP headers and message body parts failed", 0, 0, 0);
            return IMS_FALSE;
        }

        if (bGruuSupported && !piSipMsg->IsOptionSupported(strGruu))
        {
            piSipMsg->AddHeader(ISipHeader::SUPPORTED, strGruu);
        }
    }

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL ServiceMethod::IsPrivacyRequested(IN IMS_BOOL bRequest /*= IMS_TRUE*/) const
{
    if (SipConfigProxy::IsGruuConfigured(GetSlotId(), m_pService->GetSipProfile()))
    {
        const ISipMessage* piSipMsg = IMS_NULL;

        // Outgoing request message
        if (bRequest)
        {
            if (m_pNextRequest == IMS_NULL)
            {
                return IMS_FALSE;
            }

            piSipMsg = m_pNextRequest->GetMessage();
        }
        // Outgoing response message
        else
        {
            if (m_pNextResponse == IMS_NULL)
            {
                return IMS_FALSE;
            }

            piSipMsg = m_pNextResponse->GetMessage();
        }

        if (piSipMsg == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (!piSipMsg->IsHeaderPresent(ISipHeader::PRIVACY))
        {
            return IMS_FALSE;
        }

        ImsList<AString> objHeaders = piSipMsg->GetHeaders(ISipHeader::PRIVACY);

        for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
        {
            ISipHeader* piHeader =
                    SipParsingHelper::CreateHeader(ISipHeader::PRIVACY, objHeaders.GetAt(i));

            if (piHeader == IMS_NULL)
            {
                continue;
            }

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

PROTECTED
IMS_BOOL ServiceMethod::RemovePreviousMessage(IN IMS_SINT32 nServiceMethod)
{
    IMS_SLONG nIndex = m_objPreviousMessages.GetIndexOfKey(nServiceMethod);

    if (nIndex < 0)
    {
        return IMS_TRUE;
    }

    PreviousMessage* pPreviousMessage = m_objPreviousMessages.GetValueAt(nIndex);

    if (pPreviousMessage == IMS_NULL)
    {
        m_objPreviousMessages.RemoveAt(nIndex);
        return IMS_TRUE;
    }

    if ((pPreviousMessage->m_pRequest != IMS_NULL) &&
            (pPreviousMessage->m_objResponses.IsEmpty()) && (pPreviousMessage->m_piSc != IMS_NULL))
    {
        // Do not update the previous message; The transaction is not completed.
        IMS_TRACE_D("The transaction of the service method (%s) is not completed",
                Message::GetMessageType(nServiceMethod), 0, 0);
        return IMS_FALSE;
    }

    delete pPreviousMessage;

    m_objPreviousMessages.RemoveAt(nIndex);

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL ServiceMethod::SendNUpdateRequestEx(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSc,
        IN IMS_SINT32 nMessageClass /*= MESSAGE_CLASS_NORMAL*/)
{
    (void)AdjustMessage(piSc->GetMessage(), nMessageClass);

    if (piSc->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending a request message (%s) failed - Error (%d)",
                Message::GetMessageType(nServiceMethod), SipError::GetLastError(), 0);
        return IMS_FALSE;
    }

    return UpdateRequestOnSent(nServiceMethod, piSc);
}

PROTECTED
IMS_BOOL ServiceMethod::SendNUpdateResponseEx(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSc,
        IN IMS_SINT32 nMessageClass /*= MESSAGE_CLASS_NORMAL*/)
{
    (void)AdjustMessage(piSc->GetMessage(), nMessageClass);

    if (piSc->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending a response message failed - Error (%d)", SipError::GetLastError(),
                0, 0);
        return IMS_FALSE;
    }

    return UpdateResponseOnSent(nServiceMethod, piSc);
}

PROTECTED
void ServiceMethod::UpdateConnection(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSc)
{
    IMS_SLONG nIndex = m_objPreviousMessages.GetIndexOfKey(nServiceMethod);

    if (nIndex < 0)
    {
        return;
    }

    PreviousMessage* pPreviousMessage = m_objPreviousMessages.GetValueAt(nIndex);

    if (pPreviousMessage->m_piSc != IMS_NULL)
    {
        if (pPreviousMessage->m_piSc == piSc)
        {
            // Do not update the SIP connection
            return;
        }

        pPreviousMessage->m_piSc->Close();
    }

    pPreviousMessage->m_piSc = piSc;
}

PROTECTED
IMS_BOOL ServiceMethod::UpdateRequestOnReceived(
        IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSc)
{
    Message* pMessage =
            Message::CreateReceivedMessage(m_pService->GetAppConfig(), piSc->GetMessage());

    if (pMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a received message(%s) failed",
                Message::GetMessageType(nServiceMethod), 0, 0);
        return IMS_FALSE;
    }

    if (RemovePreviousMessage(nServiceMethod))
    {
        if (!SetPreviousRequest(nServiceMethod, pMessage, piSc))
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
        Ims::SetLastError(ImsError::ALREADY_EXISTS);

        IMS_TRACE_E(
                0, "MESSAGE (%s) ALREADY EXISTS", Message::GetMessageType(nServiceMethod), 0, 0);
        return IMS_FALSE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL ServiceMethod::UpdateRequestOnSent(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSc)
{
    if (piSc == IMS_NULL)
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

    m_pNextRequest->UpdateSentMessage(piSc->GetMessage());

    if (RemovePreviousMessage(nServiceMethod))
    {
        if (!SetPreviousRequest(nServiceMethod, m_pNextRequest, piSc))
        {
            delete m_pNextRequest;
            m_pNextRequest = IMS_NULL;

            return IMS_FALSE;
        }
    }
    else
    {
        // When the SIP request is sent by the engine,
        // it will not be handled by the normal service method.
        // So, the listener of SCC will be set to SCCListener object.
        ISipClientConnection* piScc = DYNAMIC_CAST(ISipClientConnection*, piSc);

        if ((piScc != IMS_NULL) && !piScc->GetMethod().Equals(SipMethod::ACK))
        {
            SccListener* pListener = new SccListener();

            piScc->SetErrorListener(pListener);
            piScc->SetListener(pListener);
        }

        delete m_pNextRequest;
    }

    m_pNextRequest = IMS_NULL;

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL ServiceMethod::UpdateResponseOnReceived(
        IN IMS_SINT32 nServiceMethod, IN const ISipConnection* piSc)
{
    Message* pMessage =
            Message::CreateReceivedMessage(m_pService->GetAppConfig(), piSc->GetMessage());

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

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL ServiceMethod::UpdateResponseOnSent(
        IN IMS_SINT32 nServiceMethod, IN const ISipConnection* piSc)
{
    // Make sure that Message MUST be created before SEND operation.
    if (GetNextResponse() == IMS_NULL)
    {
        IMS_TRACE_E(0, "Message for next response is not created", 0, 0, 0);
        return IMS_FALSE;
    }

    // Update a sent SIP message
    m_pNextResponse->UpdateSentMessage(piSc->GetMessage());

    if (!AddPreviousResponse(nServiceMethod, m_pNextResponse))
    {
        delete m_pNextResponse;
    }

    m_pNextResponse = IMS_NULL;

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL ServiceMethod::AddPreviousResponse(IN IMS_SINT32 nServiceMethod, IN Message* pMessage)
{
    IMS_SLONG nIndex = m_objPreviousMessages.GetIndexOfKey(nServiceMethod);

    if (nIndex < 0)
    {
        IMS_TRACE_E(0, "The previous message for method (%s) does not exist",
                Message::GetMessageType(nServiceMethod), 0, 0);
        return IMS_FALSE;
    }

    PreviousMessage* pPreviousMessage = m_objPreviousMessages.GetValueAt(nIndex);

    if (!pPreviousMessage->m_objResponses.Append(pMessage))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL ServiceMethod::SetPreviousRequest(
        IN IMS_SINT32 nServiceMethod, IN Message* pMessage, IN ISipConnection* piSc)
{
    IMS_SLONG nIndex = m_objPreviousMessages.GetIndexOfKey(nServiceMethod);

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

    m_objPreviousMessages.Add(nServiceMethod, pPreviousMessage);
    // Store the SIP connection for the response message
    pPreviousMessage->m_piSc = piSc;

    return IMS_TRUE;
}

PRIVATE
ISipConnection* ServiceMethod::GetConnection(IN IMS_SINT32 nServiceMethod) const
{
    IMS_SLONG nIndex = m_objPreviousMessages.GetIndexOfKey(nServiceMethod);

    if (nIndex < 0)
    {
        Ims::SetLastError(ImsError::NO_SIP_CONNECTION);
        return IMS_NULL;
    }

    const PreviousMessage* pPreviousMessage = m_objPreviousMessages.GetValueAt(nIndex);

    if (pPreviousMessage->m_piSc == IMS_NULL)
    {
        IMS_TRACE_D("SIP connection (ServiceMethod: %s) is not found",
                Message::GetMessageType(nServiceMethod), 0, 0);

        Ims::SetLastError(ImsError::NO_SIP_CONNECTION);
        return IMS_NULL;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pPreviousMessage->m_piSc;
}
