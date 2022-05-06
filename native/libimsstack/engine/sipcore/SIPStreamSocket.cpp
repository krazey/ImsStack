/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTimer.h"
#include "ServiceSystemTime.h"
#include "ByteArray.h"
#include "Connector.h"
#include "private/ConfigurationManager.h"
#include "ISIPSocketListener.h"
#include "ISIPStreamSocketListener.h"
#include "SIPPrivate.h"
#include "SipDebug.h"
#include "SIPRTConfigUtils.h"
#include "SIPMessageBuffer.h"
#include "SIPStreamSocket.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SIPStreamSocket::SIPStreamSocket(IN IMS_SINT32 nSlotId) :
        SIPSocket(nSlotId, SIPSocketAddress::SOCKET_TCP_CLIENT),
        bSecure(IMS_FALSE),
        bSIPKeepAliveConfigured(IMS_FALSE),
        piTxTimer(IMS_NULL),
        nLastAliveTime(0),
        piKeepAliveTimer(IMS_NULL),
        piListener(IMS_NULL)
{
    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    if (pSipConfig != IMS_NULL)
    {
        objTV_TCP = pSipConfig->GetTimerValueTCP();
    }
    else
    {
        objTV_TCP.m_nTvConnectionWaiting = CONNECTION_TIMER_VALUE;
        objTV_TCP.m_nTvKeepAlive = KEEPALIVE_TIMER_VALUE;
        objTV_TCP.m_nTvWouldblockWaiting = WOULDBLOCK_TIMER_VALUE;
    }
}

PUBLIC
SIPStreamSocket::SIPStreamSocket(IN IMS_SINT32 nSlotId, IN ISocket* piSocket_) :
        SIPSocket(nSlotId, SIPSocketAddress::SOCKET_TCP_CLIENT_BY_PEER),
        bSecure(IMS_FALSE),
        bSIPKeepAliveConfigured(IMS_FALSE),
        piTxTimer(IMS_NULL),
        nLastAliveTime(0),
        piKeepAliveTimer(IMS_NULL),
        piListener(IMS_NULL)
{
    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    if (pSipConfig != IMS_NULL)
    {
        objTV_TCP = pSipConfig->GetTimerValueTCP();
    }
    else
    {
        objTV_TCP.m_nTvConnectionWaiting = CONNECTION_TIMER_VALUE;
        objTV_TCP.m_nTvKeepAlive = KEEPALIVE_TIMER_VALUE;
        objTV_TCP.m_nTvWouldblockWaiting = WOULDBLOCK_TIMER_VALUE;
    }

    piSocket = piSocket_;

    SetState(STATE_CONNECTED);

    if (piSocket != IMS_NULL)
    {
        IMS_UINT32 nPort;
        IPAddress objIPA;

        piSocket->GetPeerName(objIPA, nPort);

        objSA.SetPort(nPort);
        objSA.SetIPAddress(objIPA);

        piSocket->SetListener(this);

        // Gets the local IP address and port
        objIPA = IPAddress::NONE;
        nPort = 0;
        piSocket->GetSockName(objIPA, nPort);

        // Check the socket option and set it if it is present...
        SetSocketOptions(objIPA, nPort);
    }

    // Start a keep-alive timer
    // Do not start a keep-alive timer if the socket is established by the remote end
    // StartKeepAliveTimer(0);
}

PUBLIC VIRTUAL SIPStreamSocket::~SIPStreamSocket()
{
    IMS_TRACE_D("StreamSocket(D) :: (FarEnd: %s, %d)",
            SIPRTConfigUtils::IsRoutingInfoHiddenInLog(GetSlotId())
                    ? "xxx"
                    : SipDebug::GetIp(objSA.GetIPAddress()),
            objSA.GetPort(), 0);

    StopTxTimer();
    StopKeepAliveTimer();
}

PUBLIC VIRTUAL void SIPStreamSocket::ApplyIpSec(IN ISocket* /*piAcceptedSocket = IMS_NULL*/)
{
    IPAddress objIpAddr;
    IMS_UINT32 nPort = 0;

    piSocket->GetSockName(objIpAddr, nPort);

    SocketAddress objLocal(objIpAddr, static_cast<IMS_SINT32>(nPort));

    ApplyIpSecInternal(objLocal, &(objSA.GetSocketAddress()));
    ApplyIpSecInternal(objSA.GetSocketAddress(), &objLocal);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPStreamSocket::Connect()
{
    //---------------------------------------------------------------------------------------------

    if (piSocket == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if ((GetState() == STATE_CONNECTING) || (GetState() == STATE_CONNECTED))
    {
        return IMS_TRUE;
    }

    IMS_SINT32 nResult = piSocket->Connect(objSA.GetIPAddress(), objSA.GetPort());

    if (nResult == ISocket::RESULT_WOULDBLOCK)
    {
        SetState(STATE_CONNECTING);

        if (StartTxTimer(objTV_TCP.m_nTvConnectionWaiting) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Starting CONNECT_TIMER failed", 0, 0, 0);
        }
    }
    else if (nResult == ISocket::RESULT_SUCCESS)
    {
        SetState(STATE_CONNECTED);

        // Start a keep-alive timer
        StartKeepAliveTimer(0);
    }
    else
    {
        // Error
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPStreamSocket::Create(IN CONST IPAddress& objIPA,
        IN IMS_UINT32 nPort /* = 0 */, IN IMS_BOOL bSecure /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    if (!SIPSocket::Create(objIPA, nPort, bSecure))
    {
        return IMS_FALSE;
    }

    if (GetState() != STATE_INITIALIZED)
    {
        return IMS_FALSE;
    }

    if (piSocket->Bind(objIPA, nPort) == ISocket::RESULT_ERROR)
    {
        return IMS_FALSE;
    }

    this->bSecure = bSecure;

    IMS_TRACE_I("StreamSocket(C) :: (%s , %d)",
            SIPRTConfigUtils::IsRoutingInfoHiddenInLog(GetSlotId()) ? "xxx"
                                                                    : SipDebug::GetIp(objIPA),
            nPort, 0);

    ApplyIpSec();

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL void SIPStreamSocket::GetSockName(OUT IPAddress& objIPA, OUT IMS_UINT32& nPort)
{
    //---------------------------------------------------------------------------------------------

    if (piSocket != IMS_NULL)
    {
        piSocket->GetSockName(objIPA, nPort);
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_SINT32 SIPStreamSocket::Send(IN CONST IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen,
        IN IMS_UINT32 nPort /* = 0 */, IN CONST IPAddress& objIPA /* = IPAddress::NONE */)
{
    IMS_SINT32 nTotalSentBytes = 0;
    IMS_SINT32 nSentBytes;
    IMS_SINT32 nSendingBytes = nBuffLen;

    //---------------------------------------------------------------------------------------------

    (void)nPort;
    (void)objIPA;

    if (GetState() != STATE_CONNECTED)
    {
        return nTotalSentBytes;
    }

    // Update the last active time for KeepAlive timer
    UpdateLastAliveTime();

    while (IMS_TRUE)
    {
        nSentBytes = piSocket->Send(&pBuffer[nTotalSentBytes], nSendingBytes);

        if (nSentBytes == ISocket::RESULT_ERROR)
        {
            return ISocket::RESULT_ERROR;
        }
        else if (nSentBytes == ISocket::RESULT_WOULDBLOCK)
        {
            StartTxTimer(objTV_TCP.m_nTvWouldblockWaiting);
            return nTotalSentBytes;
        }
        else
        {
            nTotalSentBytes += nSentBytes;

            if (nTotalSentBytes == nBuffLen)
            {
                return nTotalSentBytes;
            }

            nSendingBytes -= nSentBytes;
        }
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL void SIPStreamSocket::NotifyForceClosed()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("StreamSocket(%p) is forcingly closed", this, 0, 0);

    StopTxTimer();
    StopKeepAliveTimer();

    SetForcinglyClosed(IMS_TRUE);
    SIPSocket::Socket_OnClosed(piSocket);
}

/*

Remarks

*/
PUBLIC
void SIPStreamSocket::DisableKeepAlive()
{
    //---------------------------------------------------------------------------------------------

    StopKeepAliveTimer();

    objTV_TCP.m_nTvKeepAlive = 0;

    IMS_TRACE_D("Keep-alive feature is disabled", 0, 0, 0);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPStreamSocket::IsKeepAliveTimerActive() const
{
    //---------------------------------------------------------------------------------------------

    return (piKeepAliveTimer != IMS_NULL);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPStreamSocket::IsKeepAlivePermanent() const
{
    //---------------------------------------------------------------------------------------------

    // C1 : Configuration-based
    // C2 : Do not start a keep-alive timer if the socket is established by the remote end
    return ((objTV_TCP.m_nTvKeepAlive == SipConfig::TcpTimerValues::PERMANENT) ||
            ((objTV_TCP.m_nTvKeepAlive > 0) &&
                    (GetType() == SIPSocketAddress::SOCKET_TCP_CLIENT_BY_PEER)));
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPStreamSocket::IsSecureSocket() const
{
    //---------------------------------------------------------------------------------------------

    return bSecure;
}

/*

Remarks

*/
PUBLIC
void SIPStreamSocket::ReuseSocket()
{
    //---------------------------------------------------------------------------------------------

    if (IsKeepAliveTimerActive() || IsKeepAlivePermanent())
    {
        IMS_TRACE_D("Stream socket is already having keep-alive timer", 0, 0, 0);
        return;
    }

    if ((GetState() == STATE_CONNECTING) || (GetState() == STATE_CONNECTED))
    {
        // Start a keep-alive timer
        StartKeepAliveTimer(0);

        IMS_TRACE_D("Stream socket will be re-used...", 0, 0, 0);
    }
}

/*

Remarks

*/
PUBLIC
void SIPStreamSocket::SetConfigForSIPKeepAlive(IN IMS_BOOL bSIPKeepAlive)
{
    //---------------------------------------------------------------------------------------------

    if (bSIPKeepAliveConfigured != bSIPKeepAlive)
    {
        IMS_TRACE_D("SIPKeepAlive is changed; %s", _TRACE_B_(bSIPKeepAlive), 0, 0);

        bSIPKeepAliveConfigured = bSIPKeepAlive;
    }
}

/*

Remarks

*/
PUBLIC
void SIPStreamSocket::SetFarEnd(IN CONST IPAddress& objIPA, IN IMS_UINT32 nPort)
{
    //---------------------------------------------------------------------------------------------

    objSA.SetPort(nPort);
    objSA.SetIPAddress(objIPA);
}

/*

Remarks

*/
PUBLIC
void SIPStreamSocket::SetKeepAlivePolicy(IN IMS_SINT32 nPolicy)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("keep-alive policy is changed (%d >> %d)", objTV_TCP.m_nTvKeepAlive, nPolicy, 0);

    objTV_TCP.m_nTvKeepAlive = nPolicy;

    // If the default value is set, it will be set to the value based on the configuration.
    if (objTV_TCP.m_nTvKeepAlive == (-1))
    {
        const SipConfig* pSipConfig =
                ConfigurationManager::GetInstance()->GetSipConfig(GetSlotId());

        if (pSipConfig != IMS_NULL)
        {
            objTV_TCP.m_nTvKeepAlive = pSipConfig->GetTimerValueTCP().m_nTvKeepAlive;
        }
        else
        {
            objTV_TCP.m_nTvKeepAlive = KEEPALIVE_TIMER_VALUE;
        }
    }

    // Adjust the keep-alive timer
    if ((objTV_TCP.m_nTvKeepAlive == 0) ||
            (objTV_TCP.m_nTvKeepAlive == SipConfig::TcpTimerValues::PERMANENT))
    {
        StopKeepAliveTimer();
    }
    else if (objTV_TCP.m_nTvKeepAlive > 0)
    {
        StartKeepAliveTimer(0);
    }
}

/*

Remarks

*/
PUBLIC
void SIPStreamSocket::SetListener(IN ISIPStreamSocketListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PROTECTED VIRTUAL void SIPStreamSocket::Timer_TimerExpired(IN ITimer* piTimer)
{
    //---------------------------------------------------------------------------------------------

    if ((piTxTimer != IMS_NULL) && (piTxTimer == piTimer))
    {
        IMS_TRACE_D("TransmissionTimer Expired", 0, 0, 0);

        StopTxTimer();

        IMSList<ISIPSocketListener*> objTmpListeners = objListeners;

        if (GetState() == STATE_CONNECTING)
        {
            for (IMS_UINT32 i = 0; i < objTmpListeners.GetSize(); ++i)
            {
                ISIPSocketListener* piListener = objTmpListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->Socket_NotifyError(this, ERROR_CONNECTION_TIMEDOUT);
                }
            }
        }
        else
        {
            for (IMS_UINT32 i = 0; i < objTmpListeners.GetSize(); ++i)
            {
                ISIPSocketListener* piListener = objTmpListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->Socket_NotifyError(this, ERROR_WOULDBLOCK_TIMEDOUT);
                }
            }
        }
    }
    else if ((piKeepAliveTimer != IMS_NULL) && (piKeepAliveTimer == piTimer))
    {
        IMS_TRACE_D("KeepAliveTimer Expired", 0, 0, 0);
        // DEBUGGING messages...
        {
            IMS_TRACE_D("KeepAliveTimer - ends (%s)", IMS_SYS_GetTimeString().GetStr(), 0, 0);
        }

        IMS_UINT32 nInactiveInterval = IMS_SYS_GetTimeInSeconds() - nLastAliveTime;

        // If the inactive interval is zero, it means that this socket was used a short time ago,
        // so, it needs to set the inactive interval to 1 second forcibly.
        if (nInactiveInterval == 0)
        {
            nInactiveInterval = 1;
        }

        // Make a time as a milli-seconds
        nInactiveInterval = nInactiveInterval * 1000;

        // Check the inactive interval of the socket.
        // In any case, we need to give the resolution for the elapsed time.
        if ((nInactiveInterval > 0) &&
                (nInactiveInterval < static_cast<IMS_UINT32>(objTV_TCP.m_nTvKeepAlive)))
        {
            // Re-start the KeepAlive timer
            StartKeepAliveTimer(nInactiveInterval);
        }
        else
        {
            // SIPSocket::Socket_OnClosed(piSocket);

            StopKeepAliveTimer();

            if ((piListener != IMS_NULL) && (objTV_TCP.m_nTvKeepAlive > 0))
            {
                piListener->StreamSocket_KeepAliveExpired(this);
            }
        }
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL void SIPStreamSocket::Socket_OnDataReceived(IN ISocket* piSocket)
{
    IMS_SINT32 nReadBytes;
    RCPtr<SIPMessageBuffer> pMessageBuffer = SIPMessageBuffer::GetInstance();
    IMS_BYTE* pRecvBuffer = pMessageBuffer->GetBuffer(GetSlotId());

    //---------------------------------------------------------------------------------------------

    IMS_MEM_Memset(pRecvBuffer, 0x00, pMessageBuffer->GetLength());

    // Update the last active time for KeepAlive timer
    UpdateLastAliveTime();

    while (IMS_TRUE)
    {
        nReadBytes = piSocket->Receive(pRecvBuffer, pMessageBuffer->GetLength());

        IMS_TRACE_I("SIPStreamSocket(%p) :: read-bytes=%d", piSocket, nReadBytes, 0);

        if (nReadBytes == ISocket::RESULT_ERROR)
        {
            break;
        }
        else if (nReadBytes == ISocket::RESULT_WOULDBLOCK)
        {
            break;
        }
        else
        {
            pRecvBuffer[nReadBytes] = '\0';
            objMFraming.AppendPacket(pRecvBuffer, nReadBytes);

            if (nReadBytes < pMessageBuffer->GetLength())
            {
                // All data in the socket buffer is read
                break;
            }
        }
    }

    if ((nReadBytes == ISocket::RESULT_ERROR) ||
            ((nReadBytes == ISocket::RESULT_WOULDBLOCK) && (objMFraming.IsEmpty())))
    {
        return;
    }

    // Ignore any CRLF appearing before the start-line
    if (objMFraming.IgnoreCRLF())
    {
        if (objMFraming.IsEmpty())
        {
            // Notifies application to receive "pong" (server-to-client message)
            // to "ping" (keep-alive)
            if (bSIPKeepAliveConfigured)
            {
                NotifyPongReceived();
            }
            return;
        }
    }

    if (objMFraming.IsEmpty())
    {
        return;
    }

    if (piListener != IMS_NULL)
    {
        ByteArray objRecvBuffer;

        while (objMFraming.CheckCompleteMessage())
        {
            if (!objMFraming.GetCompleteMessage(objRecvBuffer))
            {
                break;
            }

            piListener->StreamSocket_DataReceived(this, objRecvBuffer);

            objMFraming.UpdateState();
        }
    }

    SIPSocket::Socket_OnDataReceived(piSocket);
}

/*

Remarks

*/
PROTECTED VIRTUAL void SIPStreamSocket::Socket_OnSendEnabled(IN ISocket* piSocket)
{
    //---------------------------------------------------------------------------------------------

    if (GetState() == STATE_CONNECTING)
    {
        SetState(STATE_CONNECTED);
    }

    StopTxTimer();

    SIPSocket::Socket_OnSendEnabled(piSocket);
}

/*

Remarks

*/
PROTECTED VIRTUAL void SIPStreamSocket::Socket_OnConnected(IN ISocket* piSocket)
{
    //---------------------------------------------------------------------------------------------

    if (GetState() == STATE_CONNECTING)
    {
        StopTxTimer();
        SetState(STATE_CONNECTED);
    }

    if (GetState() == STATE_CONNECTED)
    {
        // Start a keep-alive timer
        StartKeepAliveTimer(0);
    }

    SIPSocket::Socket_OnConnected(piSocket);
}

/*

Remarks

*/
PROTECTED VIRTUAL void SIPStreamSocket::Socket_OnClosed(
        IN ISocket* piSocket, IN IMS_SINT32 nReason /* = ISocket::CLOSE_REASON_UNKNOWN */)
{
    //---------------------------------------------------------------------------------------------

    StopTxTimer();
    StopKeepAliveTimer();

    if (objListeners.IsEmpty())
    {
        SIPStreamSocket* pTmpSocket = this;
        ISIPStreamSocketListener* piTmpListener = piListener;
        IMS_BOOL bPassiveClosedNotification = IMS_FALSE;

        if ((piListener != IMS_NULL) && (IsKeepAlivePermanent() || (objTV_TCP.m_nTvKeepAlive > 0)))
        {
            bPassiveClosedNotification = IMS_TRUE;
        }

        SIPSocket::Socket_OnClosed(piSocket, nReason);

        if (bPassiveClosedNotification)
        {
            piTmpListener->StreamSocket_PassiveClosed(pTmpSocket);
        }

        return;
    }

    SIPSocket::Socket_OnClosed(piSocket, nReason);
}

/*

Remarks

*/
PRIVATE
IMS_RESULT SIPStreamSocket::StartTxTimer(IN IMS_SINT32 nDuration)
{
    //--------------------------------------------------------------------------------------------

    if (nDuration <= 0)
    {
        IMS_TRACE_D("Do not start the trasmission timer, TV (%d)", nDuration, 0, 0);
        return IMS_SUCCESS;
    }

    if (piTxTimer == IMS_NULL)
    {
        piTxTimer = TimerService::GetTimerService()->CreateTimer();

        if (piTxTimer == IMS_NULL)
        {
            return IMS_FAILURE;
        }

        piTxTimer->SetTimer(nDuration, this);

        IMS_TRACE_D("Transmission Timer Started", 0, 0, 0);
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
void SIPStreamSocket::StopTxTimer()
{
    //---------------------------------------------------------------------------------------------

    if (piTxTimer == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("Transmission Timer Stopped", 0, 0, 0);

    piTxTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(piTxTimer);
    piTxTimer = IMS_NULL;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT SIPStreamSocket::StartKeepAliveTimer(IN IMS_SINT32 nInactiveInterval)
{
    //---------------------------------------------------------------------------------------------

    if (objTV_TCP.m_nTvKeepAlive <= 0)
    {
        IMS_TRACE_D("Do not start a keep-alive timer, TV (%d)", objTV_TCP.m_nTvKeepAlive, 0, 0);
        return IMS_SUCCESS;
    }

    if (nInactiveInterval >= objTV_TCP.m_nTvKeepAlive)
    {
        // Run the KeepAlive timer during one second.
        nInactiveInterval = objTV_TCP.m_nTvKeepAlive - 1000;
    }

    // If KEEP_ALIVE timer is present, kill it.
    StopKeepAliveTimer();

    // DEBUGGING messages...
    {
        IMS_TRACE_D("KeepAliveTimer - starts (%s)", IMS_SYS_GetTimeString().GetStr(), 0, 0);
    }

    piKeepAliveTimer = TimerService::GetTimerService()->CreateTimer();

    if (piKeepAliveTimer == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a KeepAlive timer failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    piKeepAliveTimer->SetTimer(objTV_TCP.m_nTvKeepAlive - nInactiveInterval, this);

    // In case of Accept & Connect, the last active duration will be a zero.
    if (nInactiveInterval == 0)
    {
        UpdateLastAliveTime();
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
void SIPStreamSocket::StopKeepAliveTimer()
{
    //---------------------------------------------------------------------------------------------

    if (piKeepAliveTimer == IMS_NULL)
    {
        return;
    }

    piKeepAliveTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(piKeepAliveTimer);
    piKeepAliveTimer = IMS_NULL;
}

/*

Remarks

*/
PRIVATE
void SIPStreamSocket::UpdateLastAliveTime()
{
    //--------------------------------------------------------------------------------------------

    if (piKeepAliveTimer == IMS_NULL)
    {
        return;
    }

    nLastAliveTime = IMS_SYS_GetTimeInSeconds();
}
