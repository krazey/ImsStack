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

#include "ISipClientTransmissionListener.h"
#include "SipClientTransactionState.h"
#include "SipClientTransmissionProxy.h"
#include "SipConfigProxy.h"
#include "SipError.h"
#include "SipFactoryProxy.h"
#include "SipSocket.h"
#include "SipTransport.h"
#include "SipTransportHelper.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipClientTransmissionProxy::SipClientTransmissionProxy() :
        EngineActivity(),
        m_pCtState(IMS_NULL),
        m_piListener(IMS_NULL),
        m_pTimerValues(IMS_NULL),
        m_bIsResubmittedRequest(IMS_FALSE),
        m_pSocket(IMS_NULL)
{
}

PUBLIC VIRTUAL SipClientTransmissionProxy::~SipClientTransmissionProxy()
{
    DestroyStreamSocket();
}

PUBLIC VIRTUAL IMS_BOOL SipClientTransmissionProxy::DispatchMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case AMSG_SEND_MESSAGE:
            SendPendingMessage();
            DestroyStreamSocket();
            break;

        case AMSG_NOTIFY_TRANSPORT_ERROR:
            NotifyTransportError(LONG_TO_INT(objMsg.nLparam));
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

PUBLIC
void SipClientTransmissionProxy::Abort()
{
    DestroyStreamSocket();
}

PUBLIC
IMS_RESULT SipClientTransmissionProxy::Send()
{
    m_bIsResubmittedRequest = IMS_FALSE;

    IMS_RESULT nResult = SendMessage();

    if (nResult != RESULT_PENDING)
    {
        DestroyStreamSocket();
    }

    return nResult;
}

PUBLIC
IMS_RESULT SipClientTransmissionProxy::SendWithCredentials()
{
    m_bIsResubmittedRequest = IMS_TRUE;

    IMS_RESULT nResult = SendMessage();

    if (nResult != RESULT_PENDING)
    {
        DestroyStreamSocket();
    }

    return nResult;
}

PRIVATE VIRTUAL void SipClientTransmissionProxy::Socket_NotifyError(
        IN SipSocket* pSocket, IN IMS_SINT32 nErrorCode)
{
    if (m_pSocket == IMS_NULL)
    {
        return;
    }

    if (m_pSocket != pSocket)
    {
        return;
    }

    DestroyStreamSocket();

    PostMessage(AMSG_NOTIFY_TRANSPORT_ERROR, 0, nErrorCode);
}

PRIVATE VIRTUAL void SipClientTransmissionProxy::Socket_SendEnabled(IN SipSocket* pSocket)
{
    if (m_pSocket == IMS_NULL)
    {
        return;
    }

    if (m_pSocket != pSocket)
    {
        return;
    }

    PostMessage(AMSG_SEND_MESSAGE, 0, 0);
}

PRIVATE
void SipClientTransmissionProxy::DestroyStreamSocket()
{
    if (m_pSocket != IMS_NULL)
    {
        SipTransportHelper* pTransportHelper =
                SipFactoryProxy::GetInstance()->GetTransportHelper(GetSlotId());
        pTransportHelper->Destroy(m_pSocket, this);
        m_pSocket = IMS_NULL;
    }
}

PRIVATE
IMS_BOOL SipClientTransmissionProxy::IsUdpFallbackRequired() const
{
    const SipTransport* pTransport =
            (m_pCtState != IMS_NULL) ? m_pCtState->GetSipTransport() : IMS_NULL;

    if (pTransport == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nProtocol = pTransport->GetProtocol(SipTransport::TA_FAR);

    return (!pTransport->IsTcpConnectionOnlyRequired() &&
            (nProtocol == SipTransportAddress::PROTOCOL_TCP));
}

PRIVATE
IMS_BOOL SipClientTransmissionProxy::IsUdpFallbackSupported() const
{
    const SipProfile* pProfile = (m_pCtState != IMS_NULL) ? m_pCtState->GetSipProfile() : IMS_NULL;
    return SipConfigProxy::IsUdpFallbackConfigured(GetSlotId(), pProfile);
}

PRIVATE
void SipClientTransmissionProxy::NotifyTransportError(IN IMS_SINT32 nErrorCode)
{
    if ((nErrorCode == SipSocket::ERROR_CONNECTION_TIMEDOUT) ||
            (nErrorCode == SipSocket::ERROR_CONNECT_FAILED))
    {
        // Change transport protocol to UDP
        if (m_pCtState != IMS_NULL)
        {
            IMS_TRACE_D("TransmissionProxy: UDP fallback", 0, 0, 0);
            m_pCtState->AdjustTransportProtocolAsUdp();
            SendPendingMessage();
            return;
        }
    }

    if (m_piListener != IMS_NULL)
    {
        AString strError =
                SipTransport::CreateSocketErrorMessage(nErrorCode, SipSocketAddress::SOCKET_NONE);

        m_piListener->ClientTransmission_NotifyError(SipError::TRANSPORT_ERROR, strError);
    }
}

PRIVATE
IMS_RESULT SipClientTransmissionProxy::PrepareStreamSocket()
{
    IMS_TRACE_D("TransmissionProxy: Preparing a stream socket", 0, 0, 0);

    if (m_pSocket == IMS_NULL)
    {
        SipTransport* pTransport =
                (m_pCtState != IMS_NULL) ? m_pCtState->GetSipTransport() : IMS_NULL;

        if (pTransport == IMS_NULL)
        {
            return RESULT_NOK;
        }

        m_pSocket = pTransport->CreateTcpClientSocket();

        if (m_pSocket == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a stream socket failed", 0, 0, 0);
            return RESULT_NOK;
        }

        if (!m_pSocket->Connect())
        {
            DestroyStreamSocket();

            IMS_TRACE_E(0, "Connecting a stream socket failed", 0, 0, 0);
            return RESULT_NOK;
        }

        m_pSocket->SetListener(this);
    }

    if (m_pSocket->GetState() == SipSocket::STATE_CONNECTED)
    {
        return RESULT_OK;
    }

    // Waits for TCP connection establishement.

    return RESULT_PENDING;
}

PRIVATE
IMS_RESULT SipClientTransmissionProxy::SendMessage()
{
    if (m_pCtState == IMS_NULL)
    {
        IMS_TRACE_E(0, "SipClientTransactionState is null", 0, 0, 0);
        return RESULT_NOK;
    }

    if (IsUdpFallbackSupported() && IsUdpFallbackRequired())
    {
        IMS_RESULT nResult = PrepareStreamSocket();

        if (nResult == RESULT_NOK)
        {
            IMS_TRACE_E(0, "Preparing a stream socket failed", 0, 0, 0);
            return RESULT_NOK;
        }
        else if (nResult == RESULT_PENDING)
        {
            IMS_TRACE_D("TransmissionProxy: Waits for stream socket", 0, 0, 0);
            return RESULT_PENDING;
        }
    }

    if (m_bIsResubmittedRequest)
    {
        if (!m_pCtState->SendWithCredentials(m_pTimerValues))
        {
            IMS_TRACE_E(0, "Send() failed", 0, 0, 0);
            return RESULT_NOK;
        }
    }
    else
    {
        if (!m_pCtState->Send(m_pTimerValues))
        {
            IMS_TRACE_E(0, "Send() failed", 0, 0, 0);
            return RESULT_NOK;
        }
    }

    return RESULT_OK;
}

PRIVATE
void SipClientTransmissionProxy::SendPendingMessage()
{
    IMS_RESULT nResult = RESULT_NOK;

    if (m_pCtState == IMS_NULL)
    {
        IMS_TRACE_E(0, "SipClientTransactionState is null", 0, 0, 0);
        goto EXIT_SendPendingMessage;
    }

    IMS_TRACE_D("TransmissionProxy: Sending a pending message", 0, 0, 0);

    if (m_bIsResubmittedRequest)
    {
        if (!m_pCtState->SendWithCredentials(m_pTimerValues))
        {
            IMS_TRACE_E(0, "SendWithCredentials() failed", 0, 0, 0);
            goto EXIT_SendPendingMessage;
        }

        nResult = RESULT_OK;
    }
    else
    {
        if (!m_pCtState->Send(m_pTimerValues))
        {
            IMS_TRACE_E(0, "Send() failed", 0, 0, 0);
            goto EXIT_SendPendingMessage;
        }

        nResult = RESULT_OK;
    }

EXIT_SendPendingMessage:

    if (m_piListener != IMS_NULL)
    {
        if (nResult == RESULT_OK)
        {
            m_piListener->ClientTransmission_TransmissionCompleted();
        }
        else
        {
            AString strError = "SIP message transmission failed";
            m_piListener->ClientTransmission_NotifyError(SipError::TRANSPORT_ERROR, strError);
        }
    }
}
