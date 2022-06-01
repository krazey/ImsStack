/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20160215  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ISipClientTransmissionListener.h"
#include "SipConfigProxy.h"
#include "SipFactoryProxy.h"
#include "SipTransportHelper.h"
#include "SipTransport.h"
#include "SipClientTransactionState.h"
#include "SipClientTransmissionProxy.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SIPClientTransmissionProxy::SIPClientTransmissionProxy() :
        EngineActivity(),
        pTV(IMS_NULL),
        pCTState(IMS_NULL),
        piListener(IMS_NULL),
        bIsResubmittedRequest(IMS_FALSE),
        pSocket(IMS_NULL)
{
}

PUBLIC VIRTUAL SIPClientTransmissionProxy::~SIPClientTransmissionProxy()
{
    DestroyStreamSocket();
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPClientTransmissionProxy::DispatchMessage(IN IMSMSG& objMSG)
{
    switch (objMSG.GetName())
    {
        case AMSG_SEND_MESSAGE:
            SendPendingMessage();
            DestroyStreamSocket();
            break;

        case AMSG_NOTIFY_TRANSPORT_ERROR:
            NotifyTransportError(LONG_TO_INT(objMSG.nLparam));
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
void SIPClientTransmissionProxy::Abort()
{
    //---------------------------------------------------------------------------------------------

    DestroyStreamSocket();
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPClientTransmissionProxy::Send()
{
    //---------------------------------------------------------------------------------------------

    bIsResubmittedRequest = IMS_FALSE;

    IMS_RESULT nResult = SendMessage();

    if (nResult != RESULT_PENDING)
    {
        DestroyStreamSocket();
    }

    return nResult;
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPClientTransmissionProxy::SendWithCredentials()
{
    //---------------------------------------------------------------------------------------------

    bIsResubmittedRequest = IMS_TRUE;

    IMS_RESULT nResult = SendMessage();

    if (nResult != RESULT_PENDING)
    {
        DestroyStreamSocket();
    }

    return nResult;
}

/*

Remarks

*/
PUBLIC
void SIPClientTransmissionProxy::SetListener(IN ISIPClientTransmissionListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PUBLIC
void SIPClientTransmissionProxy::SetTimerValues(IN SipTimerValues* pTV)
{
    //---------------------------------------------------------------------------------------------

    this->pTV = pTV;
}

/*

Remarks

*/
PUBLIC
void SIPClientTransmissionProxy::SetTransactionState(IN SIPClientTransactionState* pCTState)
{
    //---------------------------------------------------------------------------------------------

    this->pCTState = pCTState;
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPClientTransmissionProxy::Socket_NotifyError(
        IN SIPSocket* pSocket, IN IMS_SINT32 nErrorCode)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSocket == IMS_NULL)
    {
        return;
    }

    if (this->pSocket != pSocket)
    {
        return;
    }

    DestroyStreamSocket();

    PostMessage(AMSG_NOTIFY_TRANSPORT_ERROR, 0, nErrorCode);
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPClientTransmissionProxy::Socket_SendEnabled(IN SIPSocket* pSocket)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSocket == IMS_NULL)
    {
        return;
    }

    if (this->pSocket != pSocket)
    {
        return;
    }

    PostMessage(AMSG_SEND_MESSAGE, 0, 0);
}

/*

Remarks

*/
PRIVATE
void SIPClientTransmissionProxy::DestroyStreamSocket()
{
    //---------------------------------------------------------------------------------------------

    if (pSocket != IMS_NULL)
    {
        SIPTransportHelper* pTransportHelper =
                SIPFactoryProxy::GetInstance()->GetTransportHelper(GetSlotId());
        pTransportHelper->Destroy(pSocket, this);
        pSocket = IMS_NULL;
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPClientTransmissionProxy::IsUDPFallbackRequired() const
{
    SIPTransport* pTransport = (pCTState != IMS_NULL) ? pCTState->GetSIPTransport() : IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (pTransport == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nProtocol = pTransport->GetProtocol(SIPTransport::TA_FAR);

    return (!pTransport->IsTCPConnectionOnlyRequired() &&
            (nProtocol == SIPTransportAddress::PROTOCOL_TCP));
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPClientTransmissionProxy::IsUDPFallbackSupported() const
{
    SipProfile* pProfile = (pCTState != IMS_NULL) ? pCTState->GetSIPProfile() : IMS_NULL;

    //---------------------------------------------------------------------------------------------

    return SipConfigProxy::IsUdpFallbackConfigured(GetSlotId(), pProfile);
}

/*

Remarks

*/
PRIVATE
void SIPClientTransmissionProxy::NotifyTransportError(IN IMS_SINT32 nErrorCode)
{
    //---------------------------------------------------------------------------------------------

    if ((nErrorCode == SIPSocket::ERROR_CONNECTION_TIMEDOUT) ||
            (nErrorCode == SIPSocket::ERROR_CONNECT_FAILED))
    {
        // Change transport protocol to UDP
        if (pCTState != IMS_NULL)
        {
            IMS_TRACE_D("TransmissionProxy :: UDP fallback", 0, 0, 0);
            pCTState->AdjustTransportProtocolAsUDP();
            SendPendingMessage();
            return;
        }
    }

    if (piListener != IMS_NULL)
    {
        AString strError =
                SIPTransport::CreateSocketErrorMessage(nErrorCode, SIPSocketAddress::SOCKET_NONE);

        piListener->ClientTransmission_NotifyError(SipError::TRANSPORT_ERROR, strError);
    }
}

/*

Remarks

*/
PRIVATE
IMS_RESULT SIPClientTransmissionProxy::PrepareStreamSocket()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("TransmissionProxy :: Preparing a stream socket", 0, 0, 0);

    if (pSocket == IMS_NULL)
    {
        SIPTransport* pTransport = (pCTState != IMS_NULL) ? pCTState->GetSIPTransport() : IMS_NULL;

        if (pTransport == IMS_NULL)
        {
            return RESULT_NOK;
        }

        pSocket = pTransport->CreateTCPClientSocket();

        if (pSocket == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a stream socket failed", 0, 0, 0);
            return RESULT_NOK;
        }

        if (!pSocket->Connect())
        {
            DestroyStreamSocket();

            IMS_TRACE_E(0, "Connecting a stream socket failed", 0, 0, 0);
            return RESULT_NOK;
        }

        pSocket->SetListener(this);
    }

    if (pSocket->GetState() == SIPSocket::STATE_CONNECTED)
    {
        return RESULT_OK;
    }

    // Waits for TCP connection establishement.

    return RESULT_PENDING;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT SIPClientTransmissionProxy::SendMessage()
{
    //---------------------------------------------------------------------------------------------

    if (pCTState == IMS_NULL)
    {
        IMS_TRACE_E(0, "SIPClientTransactionState is null", 0, 0, 0);
        return RESULT_NOK;
    }

    if (IsUDPFallbackSupported() && IsUDPFallbackRequired())
    {
        IMS_RESULT nResult = PrepareStreamSocket();

        if (nResult == RESULT_NOK)
        {
            IMS_TRACE_E(0, "Preparing a stream socket failed", 0, 0, 0);
            return RESULT_NOK;
        }
        else if (nResult == RESULT_PENDING)
        {
            IMS_TRACE_D("TransmissionProxy :: Waits for stream socket", 0, 0, 0);
            return RESULT_PENDING;
        }
    }

    if (bIsResubmittedRequest)
    {
        if (!pCTState->SendWithCredentials(pTV))
        {
            IMS_TRACE_E(0, "Send() failed", 0, 0, 0);
            return RESULT_NOK;
        }
    }
    else
    {
        if (!pCTState->Send(pTV))
        {
            IMS_TRACE_E(0, "Send() failed", 0, 0, 0);
            return RESULT_NOK;
        }
    }

    return RESULT_OK;
}

/*

Remarks

*/
PRIVATE
void SIPClientTransmissionProxy::SendPendingMessage()
{
    IMS_RESULT nResult = RESULT_NOK;

    //---------------------------------------------------------------------------------------------

    if (pCTState == IMS_NULL)
    {
        IMS_TRACE_E(0, "SIPClientTransactionState is null", 0, 0, 0);
        goto EXIT_SendPendingMessage;
    }

    IMS_TRACE_D("TransmissionProxy :: Sending a pending message", 0, 0, 0);

    if (bIsResubmittedRequest)
    {
        if (!pCTState->SendWithCredentials(pTV))
        {
            IMS_TRACE_E(0, "SendWithCredentials() failed", 0, 0, 0);
            goto EXIT_SendPendingMessage;
        }

        nResult = RESULT_OK;
    }
    else
    {
        if (!pCTState->Send(pTV))
        {
            IMS_TRACE_E(0, "Send() failed", 0, 0, 0);
            goto EXIT_SendPendingMessage;
        }

        nResult = RESULT_OK;
    }

EXIT_SendPendingMessage:

    if (piListener != IMS_NULL)
    {
        if (nResult == RESULT_OK)
        {
            piListener->ClientTransmission_TransmissionCompleted();
        }
        else
        {
            AString strError = "SIP message transmission failed";
            piListener->ClientTransmission_NotifyError(SipError::TRANSPORT_ERROR, strError);
        }
    }
}
