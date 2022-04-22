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
#include "INetworkPingListener.h"
#include "ImsNetworkPing.h"
#include "ServiceMemory.h"
#include "ServiceNetwork.h"
#include "ServiceSystemTime.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_ADAPT__;

PUBLIC
ImsNetworkPing::ImsNetworkPing()
    : m_piListener(IMS_NULL)
    , m_nState(STATE_IDLE)
    , m_piTimer(IMS_NULL)
    , m_piSocket(IMS_NULL)
{
}

PUBLIC VIRTUAL
ImsNetworkPing::~ImsNetworkPing()
{
    ClearResources();
}

PUBLIC VIRTUAL
void ImsNetworkPing::Destroy()
{
    ClearResources();

    delete this;
}

PUBLIC VIRTUAL
IMS_SINT32 ImsNetworkPing::Ping(IN const IPAddress& objSrcIp,
        IN const IPAddress& objDstIp, IN IMS_SINT32 nDstPort,
        IN IMS_SINT32 nWaitTime)
{
    if (GetState() != STATE_IDLE)
    {
        IMS_TRACE_D("NetPing :: Request pending...", 0, 0, 0);
        return PING_STATUS_PENDING;
    }

    if (!PrepareResources(objSrcIp, (nWaitTime > 0)))
    {
        IMS_TRACE_E(0, "PrepareResources failed", 0, 0, 0);
        return PING_STATUS_NOK;
    }

    IMS_SINT32 nResult = m_piSocket->Connect(objDstIp, nDstPort);

    if (nResult == ISocket::RESULT_SUCCESS)
    {
        ClearResources();
        return PING_STATUS_OK;
    }
    else if (nResult == ISocket::RESULT_WOULDBLOCK)
    {
        if (m_piTimer != IMS_NULL)
        {
            m_piTimer->SetTimer(nWaitTime, this);
        }
    }
    else
    {
        ClearResources();
        IMS_TRACE_D("NetPing :: Connect failed", 0, 0, 0);
        return PING_STATUS_NOK;
    }

    SetState(STATE_PENDING);

    return PING_STATUS_PENDING;
}

PUBLIC VIRTUAL
void ImsNetworkPing::SetListener(IN INetworkPingListener* piListener)
{
    m_piListener = piListener;
}

PROTECTED VIRTUAL
void ImsNetworkPing::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (m_piTimer != piTimer)
    {
        return;
    }

    if (GetState() == STATE_PENDING)
    {
        ClearAndNotifyResult(PING_STATUS_TIMEDOUT);
    }
    else
    {
        m_piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer);
        m_piTimer = IMS_NULL;
    }
}

PROTECTED VIRTUAL
void ImsNetworkPing::Socket_OnDataReceived(IN ISocket* /*piSocket*/)
{
    // no-op
}

PROTECTED VIRTUAL
void ImsNetworkPing::Socket_OnSendEnabled(IN ISocket* piSocket)
{
    if (m_piSocket != piSocket)
    {
        return;
    }

    if (GetState() == STATE_IDLE)
    {
        return;
    }

    ClearAndNotifyResult(PING_STATUS_OK);
}

PROTECTED VIRTUAL
void ImsNetworkPing::Socket_OnConnectionReceived(IN ISocket* /*piSocket*/)
{
    // no-op
}

PROTECTED VIRTUAL
void ImsNetworkPing::Socket_OnConnected(IN ISocket* piSocket)
{
    if (m_piSocket != piSocket)
    {
        return;
    }

    if (GetState() == STATE_IDLE)
    {
        return;
    }

    ClearAndNotifyResult(PING_STATUS_OK);
}

PROTECTED VIRTUAL
void ImsNetworkPing::Socket_OnClosed(IN ISocket* piSocket,
        IN IMS_SINT32 /*nReason = ISocket::CLOSE_REASON_UNKNOWN*/)
{
    if (m_piSocket != piSocket)
    {
        return;
    }

    if (GetState() == STATE_IDLE)
    {
        return;
    }

    ClearAndNotifyResult(PING_STATUS_DEAD_PEER);
}

PROTECTED
void ImsNetworkPing::ClearResources()
{
    if (m_piTimer != IMS_NULL)
    {
        m_piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer);
        m_piTimer = IMS_NULL;
    }

    if (m_piSocket != IMS_NULL)
    {
        m_piSocket->Close();
        NetworkService::GetNetworkService()->DestroySocket(m_piSocket);
        m_piSocket = IMS_NULL;
    }
}

PROTECTED
IMS_BOOL ImsNetworkPing::PrepareResources(IN const IPAddress& objIp,
        IN IMS_BOOL bTimerRequired)
{
    ClearResources();

    // Prepare timer if required
    if (bTimerRequired)
    {
        m_piTimer = TimerService::GetTimerService()->CreateTimer();

        if (m_piTimer == IMS_NULL)
        {
            IMS_TRACE_E(0, "NetPing :: Timer creation failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    // Prepare socket
    INetworkConnection* piConnection = NetworkService::GetNetworkService()->FindConnection(objIp);

    m_piSocket = NetworkService::GetNetworkService()->CreateSocket(piConnection);

    if (m_piSocket == IMS_NULL)
    {
        ClearResources();
        IMS_TRACE_E(0, "NetPing :: Socket creation failed", 0, 0, 0);
        return IMS_FALSE;
    }

    m_piSocket->SetListener(this);

    ISocket::ADDRESS_FAMILY_ENTYPE enAddressFamily = ISocket::ADDRESS_FAMILY_INET;

    if (!objIp.IsIPv4Address())
    {
        enAddressFamily = ISocket::ADDRESS_FAMILY_INET6;
    }

    ISocket::SOCKET_RESULT enResult = m_piSocket->Open(
            ISocket::TYPE_STREAM, enAddressFamily);

    if (enResult != ISocket::RESULT_SUCCESS)
    {
        ClearResources();
        return IMS_FALSE;
    }

    IMS_SINT32 nOptVal = 1;

    m_piSocket->SetOption(ISocket::OPT_REUSEADDR, nOptVal);
    m_piSocket->SetOption(ISocket::OPT_LINGER, nOptVal);
    // TOS : required ?

    if (m_piSocket->Bind(objIp, GetRandomPort(objIp)) == ISocket::RESULT_ERROR)
    {
        ClearResources();
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED
void ImsNetworkPing::ClearAndNotifyResult(IN IMS_SINT32 nResult)
{
    ClearResources();
    SetState(STATE_IDLE);
    NotifyResult(nResult);
}

PROTECTED
void ImsNetworkPing::NotifyResult(IN IMS_SINT32 nResult)
{
    IMS_TRACE_I("NetPing :: Result=%d", nResult, 0, 0);

    if (m_piListener != IMS_NULL)
    {
        m_piListener->NetworkPing_NotifyResult(this, nResult);
    }
}

PROTECTED
IMS_SINT32 ImsNetworkPing::GetState() const
{
    return m_nState;
}

PROTECTED
void ImsNetworkPing::SetState(IN IMS_SINT32 nState)
{
    if (m_nState != nState)
    {
        IMS_TRACE_I("NetPing :: %d >> %d", m_nState, nState, 0);
        m_nState = nState;
    }
}

PROTECTED GLOBAL
IMS_SINT32 ImsNetworkPing::GetRandomPort(IN const IPAddress& objIp)
{
    const IMS_SINT32 MAX_COUNT = 50;
    NetworkService* pNetworkService = NetworkService::GetNetworkService();

    for (IMS_UINT32 i = 0; i < MAX_COUNT; i++)
    {
        // Generate random port number with values between 49152 and 65535
        IMS_SINT32 nPort = (IMS_SYS_GetRandom0() % (65535 - 49152 + 1)) + 49152;

        if (pNetworkService->CheckIpAndPortAvailability(objIp, nPort, ISocket::TYPE_STREAM))
        {
            IMS_TRACE_D("NetPing :: random-port=%d(%d)", nPort, i, 0);
            return nPort;
        }
    }

    return 0;
}
