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
#include "ByteArray.h"
#include "ServiceMemory.h"
#include "ServiceSystemTime.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

#include "private/ConfigurationManager.h"

#include "ISipSocketListener.h"
#include "ISipStreamSocketListener.h"
#include "SipDebug.h"
#include "SipMessageBuffer.h"
#include "SipPrivate.h"
#include "SipRtConfigUtils.h"
#include "SipStreamSocket.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipStreamSocket::SipStreamSocket(IN IMS_SINT32 nSlotId) :
        SipSocket(nSlotId, SipSocketAddress::SOCKET_TCP_CLIENT),
        m_bSecure(IMS_FALSE),
        m_bSipKeepAliveConfigured(IMS_FALSE),
        m_piTxTimer(IMS_NULL),
        m_nLastAliveTime(0),
        m_piKeepAliveTimer(IMS_NULL),
        m_piListener(IMS_NULL)
{
    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    if (pSipConfig != IMS_NULL)
    {
        m_objTcpTimerValues = pSipConfig->GetTimerValueTcp();
    }
    else
    {
        m_objTcpTimerValues.m_nTvConnectionWaiting = CONNECTION_TIMER_VALUE;
        m_objTcpTimerValues.m_nTvKeepAlive = KEEPALIVE_TIMER_VALUE;
        m_objTcpTimerValues.m_nTvWouldblockWaiting = WOULDBLOCK_TIMER_VALUE;
    }
}

PUBLIC
SipStreamSocket::SipStreamSocket(IN IMS_SINT32 nSlotId, IN ISocket* piSocket) :
        SipSocket(nSlotId, SipSocketAddress::SOCKET_TCP_CLIENT_BY_PEER),
        m_bSecure(IMS_FALSE),
        m_bSipKeepAliveConfigured(IMS_FALSE),
        m_piTxTimer(IMS_NULL),
        m_nLastAliveTime(0),
        m_piKeepAliveTimer(IMS_NULL),
        m_piListener(IMS_NULL)
{
    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(nSlotId);

    if (pSipConfig != IMS_NULL)
    {
        m_objTcpTimerValues = pSipConfig->GetTimerValueTcp();
    }
    else
    {
        m_objTcpTimerValues.m_nTvConnectionWaiting = CONNECTION_TIMER_VALUE;
        m_objTcpTimerValues.m_nTvKeepAlive = KEEPALIVE_TIMER_VALUE;
        m_objTcpTimerValues.m_nTvWouldblockWaiting = WOULDBLOCK_TIMER_VALUE;
    }

    m_piSocket = piSocket;

    SetState(STATE_CONNECTED);

    if (m_piSocket != IMS_NULL)
    {
        IMS_UINT32 nPort;
        IpAddress objIp;

        m_piSocket->GetPeerName(objIp, nPort);

        m_objSockAddr.SetPort(nPort);
        m_objSockAddr.SetIpAddress(objIp);

        m_piSocket->SetListener(this);

        // Gets the local IP address and port
        objIp = IpAddress::NONE;
        nPort = 0;
        m_piSocket->GetSockName(objIp, nPort);

        // Check the socket option and set it if it is present.
        SetSocketOptions(objIp, nPort);
    }

    // Start a keep-alive timer
    // Do not start a keep-alive timer if the socket is established by the remote end
    // StartKeepAliveTimer(0);
}

PUBLIC VIRTUAL SipStreamSocket::~SipStreamSocket()
{
    IMS_TRACE_D("StreamSocket(D) (Far=%s|%d)",
            SipRtConfigUtils::IsRoutingInfoHiddenInLog(GetSlotId())
                    ? "***"
                    : SipDebug::GetIp(m_objSockAddr.GetIpAddress()),
            m_objSockAddr.GetPort(), 0);

    StopTxTimer();
    StopKeepAliveTimer();
}

PUBLIC VIRTUAL void SipStreamSocket::ApplyIpSec(IN SipSocket* /*pAcceptedSocket = IMS_NULL*/)
{
    IpAddress objIpAddr;
    IMS_UINT32 nPort = 0;

    m_piSocket->GetSockName(objIpAddr, nPort);

    SocketAddress objLocal(objIpAddr, static_cast<IMS_SINT32>(nPort));

    ApplyIpSecInternal(objLocal, &(m_objSockAddr.GetSocketAddress()));
    ApplyIpSecInternal(m_objSockAddr.GetSocketAddress(), &objLocal);
}

PUBLIC VIRTUAL IMS_BOOL SipStreamSocket::Connect()
{
    if (m_piSocket == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if ((GetState() == STATE_CONNECTING) || (GetState() == STATE_CONNECTED))
    {
        return IMS_TRUE;
    }

    IMS_SINT32 nResult = m_piSocket->Connect(m_objSockAddr.GetIpAddress(), m_objSockAddr.GetPort());

    if (nResult == ISocket::RESULT_WOULDBLOCK)
    {
        SetState(STATE_CONNECTING);

        if (StartTxTimer(m_objTcpTimerValues.m_nTvConnectionWaiting) != IMS_SUCCESS)
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

PUBLIC VIRTUAL IMS_BOOL SipStreamSocket::Create(
        IN const IpAddress& objIp, IN IMS_UINT32 nPort /*= 0*/, IN IMS_BOOL bSecure /*= IMS_FALSE*/)
{
    if (!SipSocket::Create(objIp, nPort, bSecure))
    {
        return IMS_FALSE;
    }

    if (GetState() != STATE_INITIALIZED)
    {
        return IMS_FALSE;
    }

    if (m_piSocket->Bind(objIp, nPort) == ISocket::RESULT_ERROR)
    {
        return IMS_FALSE;
    }

    m_bSecure = bSecure;

    IMS_TRACE_I("StreamSocket(C) (%s|%d)",
            SipRtConfigUtils::IsRoutingInfoHiddenInLog(GetSlotId()) ? "***"
                                                                    : SipDebug::GetIp(objIp),
            nPort, 0);

    ApplyIpSec();

    return IMS_TRUE;
}

PUBLIC VIRTUAL void SipStreamSocket::GetSockName(OUT IpAddress& objIp, OUT IMS_UINT32& nPort)
{
    if (m_piSocket != IMS_NULL)
    {
        m_piSocket->GetSockName(objIp, nPort);
    }
}

PUBLIC VIRTUAL IMS_SINT32 SipStreamSocket::Send(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen,
        IN IMS_UINT32 /*nPort = 0*/, IN const IpAddress& /*objIp = IpAddress::NONE*/)
{
    IMS_SINT32 nTotalSentBytes = 0;
    IMS_SINT32 nSendingBytes = nBuffLen;

    if (GetState() != STATE_CONNECTED)
    {
        return nTotalSentBytes;
    }

    // Update the last active time for KeepAlive timer
    UpdateLastAliveTime();

    while (IMS_TRUE)
    {
        IMS_SINT32 nSentBytes = m_piSocket->Send(&pBuffer[nTotalSentBytes], nSendingBytes);

        if (nSentBytes == ISocket::RESULT_ERROR)
        {
            return ISocket::RESULT_ERROR;
        }
        else if (nSentBytes == ISocket::RESULT_WOULDBLOCK)
        {
            StartTxTimer(m_objTcpTimerValues.m_nTvWouldblockWaiting);
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

PUBLIC VIRTUAL void SipStreamSocket::NotifyForceClosed()
{
    IMS_TRACE_D("StreamSocket(%p) is forcingly closed", this, 0, 0);

    StopTxTimer();
    StopKeepAliveTimer();

    SetForcinglyClosed(IMS_TRUE);
    SipSocket::Socket_OnClosed(m_piSocket);
}

PUBLIC
void SipStreamSocket::DisableKeepAlive()
{
    StopKeepAliveTimer();

    m_objTcpTimerValues.m_nTvKeepAlive = 0;

    IMS_TRACE_D("Keep-alive feature is disabled", 0, 0, 0);
}

PUBLIC
IMS_BOOL SipStreamSocket::IsKeepAlivePermanent() const
{
    // C1 : Configuration-based
    // C2 : Do not start a keep-alive timer if the socket is established by the remote end
    return ((m_objTcpTimerValues.m_nTvKeepAlive == SipConfig::TcpTimerValues::PERMANENT) ||
            ((m_objTcpTimerValues.m_nTvKeepAlive > 0) &&
                    (GetType() == SipSocketAddress::SOCKET_TCP_CLIENT_BY_PEER)));
}

PUBLIC
void SipStreamSocket::ReuseSocket()
{
    if (IsKeepAliveTimerActive() || IsKeepAlivePermanent())
    {
        IMS_TRACE_D("Stream socket is already having keep-alive timer", 0, 0, 0);
        return;
    }

    if ((GetState() == STATE_CONNECTING) || (GetState() == STATE_CONNECTED))
    {
        // Start a keep-alive timer
        StartKeepAliveTimer(0);

        IMS_TRACE_D("Stream socket being re-used", 0, 0, 0);
    }
}

PUBLIC
void SipStreamSocket::SetConfigForSipKeepAlive(IN IMS_BOOL bSipKeepAlive)
{
    if (m_bSipKeepAliveConfigured != bSipKeepAlive)
    {
        IMS_TRACE_D("SipKeepAlive is changed to %s", _TRACE_B_(bSipKeepAlive), 0, 0);

        m_bSipKeepAliveConfigured = bSipKeepAlive;
    }
}

PUBLIC
void SipStreamSocket::SetFarEnd(IN const IpAddress& objIp, IN IMS_UINT32 nPort)
{
    m_objSockAddr.SetPort(nPort);
    m_objSockAddr.SetIpAddress(objIp);
}

PUBLIC
void SipStreamSocket::SetKeepAlivePolicy(IN IMS_SINT32 nPolicy)
{
    IMS_TRACE_D("keep-alive policy is changed (%d >> %d)", m_objTcpTimerValues.m_nTvKeepAlive,
            nPolicy, 0);

    m_objTcpTimerValues.m_nTvKeepAlive = nPolicy;

    // If the default value is set, it will be set to the value based on the configuration.
    if (m_objTcpTimerValues.m_nTvKeepAlive == (-1))
    {
        const SipConfig* pSipConfig =
                ConfigurationManager::GetInstance()->GetSipConfig(GetSlotId());

        if (pSipConfig != IMS_NULL)
        {
            m_objTcpTimerValues.m_nTvKeepAlive = pSipConfig->GetTimerValueTcp().m_nTvKeepAlive;
        }
        else
        {
            m_objTcpTimerValues.m_nTvKeepAlive = KEEPALIVE_TIMER_VALUE;
        }
    }

    // Adjust the keep-alive timer
    if ((m_objTcpTimerValues.m_nTvKeepAlive == 0) ||
            (m_objTcpTimerValues.m_nTvKeepAlive == SipConfig::TcpTimerValues::PERMANENT))
    {
        StopKeepAliveTimer();
    }
    else if (m_objTcpTimerValues.m_nTvKeepAlive > 0)
    {
        StartKeepAliveTimer(0);
    }
}

PROTECTED VIRTUAL void SipStreamSocket::Timer_TimerExpired(IN ITimer* piTimer)
{
    if ((m_piTxTimer != IMS_NULL) && (m_piTxTimer == piTimer))
    {
        IMS_TRACE_D("TransmissionTimer Expired", 0, 0, 0);

        StopTxTimer();

        ImsList<ISipSocketListener*> objTmpListeners = m_objListeners;

        if (GetState() == STATE_CONNECTING)
        {
            for (IMS_UINT32 i = 0; i < objTmpListeners.GetSize(); ++i)
            {
                ISipSocketListener* piListener = objTmpListeners.GetAt(i);

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
                ISipSocketListener* piListener = objTmpListeners.GetAt(i);

                if (piListener != IMS_NULL)
                {
                    piListener->Socket_NotifyError(this, ERROR_WOULDBLOCK_TIMEDOUT);
                }
            }
        }
    }
    else if ((m_piKeepAliveTimer != IMS_NULL) && (m_piKeepAliveTimer == piTimer))
    {
        IMS_TRACE_D("KeepAliveTimer Expired", 0, 0, 0);

#ifdef __IMS_SIP_DEBUG__
        IMS_TRACE_D("KeepAliveTimer - ends (%s)", IMS_SYS_GetTimeString().GetStr(), 0, 0);
#endif

        IMS_UINT32 nInactiveInterval = IMS_SYS_GetTimeInSeconds() - m_nLastAliveTime;

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
                (nInactiveInterval < static_cast<IMS_UINT32>(m_objTcpTimerValues.m_nTvKeepAlive)))
        {
            // Re-start the KeepAlive timer
            StartKeepAliveTimer(nInactiveInterval);
        }
        else
        {
            // SipSocket::Socket_OnClosed(piSocket);

            StopKeepAliveTimer();

            if ((m_piListener != IMS_NULL) && (m_objTcpTimerValues.m_nTvKeepAlive > 0))
            {
                m_piListener->StreamSocket_KeepAliveExpired(this);
            }
        }
    }
}

PROTECTED VIRTUAL void SipStreamSocket::Socket_OnDataReceived(IN ISocket* piSocket)
{
    IMS_SINT32 nReadBytes;
    RcPtr<SipMessageBuffer> pMessageBuffer = SipMessageBuffer::GetInstance();
    IMS_BYTE* pRecvBuffer = pMessageBuffer->GetBuffer(GetSlotId());

    IMS_MEM_Memset(pRecvBuffer, 0x00, pMessageBuffer->GetLength());

    // Update the last active time for KeepAlive timer
    UpdateLastAliveTime();

    while (IMS_TRUE)
    {
        nReadBytes = piSocket->Receive(pRecvBuffer, pMessageBuffer->GetLength());

        IMS_TRACE_I("StreamSocket(%p): read-bytes=%d", piSocket, nReadBytes, 0);

        if (nReadBytes == ISocket::RESULT_ERROR || nReadBytes == ISocket::RESULT_WOULDBLOCK)
        {
            break;
        }
        else
        {
            pRecvBuffer[nReadBytes] = '\0';
            m_objMsgFraming.AppendPacket(pRecvBuffer, nReadBytes);

            if (nReadBytes < pMessageBuffer->GetLength())
            {
                // All data in the socket buffer is read
                break;
            }
        }
    }

    if ((nReadBytes == ISocket::RESULT_ERROR) ||
            ((nReadBytes == ISocket::RESULT_WOULDBLOCK) && (m_objMsgFraming.IsEmpty())))
    {
        return;
    }

    // Ignore any CRLF appearing before the start-line
    if (m_objMsgFraming.IgnoreCrlf())
    {
        if (m_objMsgFraming.IsEmpty())
        {
            // Notifies application to receive "pong" (server-to-client message)
            // to "ping" (keep-alive)
            if (m_bSipKeepAliveConfigured)
            {
                NotifyPongReceived();
            }
            return;
        }
    }

    if (m_objMsgFraming.IsEmpty())
    {
        return;
    }

    if (m_piListener != IMS_NULL)
    {
        ByteArray objRecvBuffer;

        while (m_objMsgFraming.CheckCompleteMessage())
        {
            if (!m_objMsgFraming.GetCompleteMessage(objRecvBuffer))
            {
                break;
            }

            m_piListener->StreamSocket_DataReceived(this, objRecvBuffer);

            m_objMsgFraming.UpdateState();
        }
    }

    SipSocket::Socket_OnDataReceived(piSocket);
}

PROTECTED VIRTUAL void SipStreamSocket::Socket_OnSendEnabled(IN ISocket* piSocket)
{
    if (GetState() == STATE_CONNECTING)
    {
        SetState(STATE_CONNECTED);
    }

    StopTxTimer();

    SipSocket::Socket_OnSendEnabled(piSocket);
}

PROTECTED VIRTUAL void SipStreamSocket::Socket_OnConnected(IN ISocket* piSocket)
{
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

    SipSocket::Socket_OnConnected(piSocket);
}

PROTECTED VIRTUAL void SipStreamSocket::Socket_OnClosed(
        IN ISocket* piSocket, IN IMS_SINT32 nReason /*= ISocket::CLOSE_REASON_UNKNOWN*/)
{
    StopTxTimer();
    StopKeepAliveTimer();

    if (m_objListeners.IsEmpty())
    {
        SipStreamSocket* pTmpSocket = this;
        ISipStreamSocketListener* piTmpListener = m_piListener;
        IMS_BOOL bPassiveClosedNotification = IMS_FALSE;

        if ((m_piListener != IMS_NULL) &&
                (IsKeepAlivePermanent() || (m_objTcpTimerValues.m_nTvKeepAlive > 0)))
        {
            bPassiveClosedNotification = IMS_TRUE;
        }

        SipSocket::Socket_OnClosed(piSocket, nReason);

        if (bPassiveClosedNotification)
        {
            piTmpListener->StreamSocket_PassiveClosed(pTmpSocket);
        }

        return;
    }

    SipSocket::Socket_OnClosed(piSocket, nReason);
}

PRIVATE
IMS_RESULT SipStreamSocket::StartTxTimer(IN IMS_SINT32 nDuration)
{
    if (nDuration <= 0)
    {
        IMS_TRACE_D("Do not start the transmission timer(%d)", nDuration, 0, 0);
        return IMS_SUCCESS;
    }

    if (m_piTxTimer == IMS_NULL)
    {
        m_piTxTimer = TimerService::GetTimerService()->CreateTimer();

        if (m_piTxTimer == IMS_NULL)
        {
            return IMS_FAILURE;
        }

        m_piTxTimer->SetTimer(nDuration, this);

        IMS_TRACE_D("Transmission Timer Started", 0, 0, 0);
    }

    return IMS_SUCCESS;
}

PRIVATE
void SipStreamSocket::StopTxTimer()
{
    if (m_piTxTimer == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("Transmission Timer Stopped", 0, 0, 0);

    m_piTxTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTxTimer);
    m_piTxTimer = IMS_NULL;
}

PRIVATE
IMS_RESULT SipStreamSocket::StartKeepAliveTimer(IN IMS_SINT32 nInactiveInterval)
{
    if (m_objTcpTimerValues.m_nTvKeepAlive <= 0)
    {
        IMS_TRACE_D("Do not start keep-alive timer(%d)", m_objTcpTimerValues.m_nTvKeepAlive, 0, 0);
        return IMS_SUCCESS;
    }

    if (nInactiveInterval >= m_objTcpTimerValues.m_nTvKeepAlive)
    {
        // Run the KeepAlive timer during one second.
        nInactiveInterval = m_objTcpTimerValues.m_nTvKeepAlive - 1000;
    }

    // If KEEP_ALIVE timer is present, kill it.
    StopKeepAliveTimer();

#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("KeepAliveTimer - starts (%s)", IMS_SYS_GetTimeString().GetStr(), 0, 0);
#endif

    m_piKeepAliveTimer = TimerService::GetTimerService()->CreateTimer();

    if (m_piKeepAliveTimer == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a KeepAlive timer failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    m_piKeepAliveTimer->SetTimer(m_objTcpTimerValues.m_nTvKeepAlive - nInactiveInterval, this);

    // In case of Accept & Connect, the last active duration will be a zero.
    if (nInactiveInterval == 0)
    {
        UpdateLastAliveTime();
    }

    return IMS_SUCCESS;
}

PRIVATE
void SipStreamSocket::StopKeepAliveTimer()
{
    if (m_piKeepAliveTimer == IMS_NULL)
    {
        return;
    }

    m_piKeepAliveTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piKeepAliveTimer);
    m_piKeepAliveTimer = IMS_NULL;
}

PRIVATE
void SipStreamSocket::UpdateLastAliveTime()
{
    if (m_piKeepAliveTimer == IMS_NULL)
    {
        return;
    }

    m_nLastAliveTime = IMS_SYS_GetTimeInSeconds();
}
