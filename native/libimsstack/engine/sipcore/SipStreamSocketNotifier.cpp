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
#include "ServiceNetwork.h"
#include "ServiceTrace.h"

#include "ISipStreamSocketListener.h"
#include "SipDebug.h"
#include "SipPrivate.h"
#include "SipRtConfigUtils.h"
#include "SipStreamSocket.h"
#include "SipStreamSocketNotifier.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipStreamSocketNotifier::SipStreamSocketNotifier(IN IMS_SINT32 nSlotId) :
        SipSocket(nSlotId, SipSocketAddress::SOCKET_TCP),
        m_piListener(IMS_NULL)
{
}

PUBLIC VIRTUAL SipStreamSocketNotifier::~SipStreamSocketNotifier()
{
    IMS_TRACE_D("StreamSocketNotifier(D) (%s|%d)", SipDebug::GetIp(m_objSockAddr.GetIpAddress()),
            m_objSockAddr.GetPort(), 0);
}

PUBLIC VIRTUAL SipSocket* SipStreamSocketNotifier::Accept()
{
    if (GetState() != STATE_CONNECTED)
    {
        return IMS_NULL;
    }

    ISocket* piNewSocket = m_piSocket->Accept();

    if (piNewSocket != IMS_NULL)
    {
        IMS_TRACE_I("StreamSocket being created by remote end", 0, 0, 0);

        SipStreamSocket* pStreamSocket = new SipStreamSocket(GetSlotId(), piNewSocket);

        if (pStreamSocket == IMS_NULL)
        {
            piNewSocket->SetListener(IMS_NULL);
            piNewSocket->Close();
            NetworkService::GetNetworkService()->DestroySocket(piNewSocket);
            return IMS_NULL;
        }

        ApplyIpSec(pStreamSocket);

        // Inherits the server's listener
        pStreamSocket->SetListener(m_piListener);

        return pStreamSocket;
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL IMS_BOOL SipStreamSocketNotifier::Create(
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

    if (m_piSocket->Listen() == ISocket::RESULT_ERROR)
    {
        return IMS_FALSE;
    }

    m_objSockAddr.SetPort(nPort);
    m_objSockAddr.SetIpAddress(objIp);

    SetState(STATE_CONNECTED);

    IMS_TRACE_I("StreamSocketNotifier(C) (%s|%d)",
            SipRtConfigUtils::IsRoutingInfoHiddenInLog(GetSlotId()) ? "***"
                                                                    : SipDebug::GetIp(objIp),
            nPort, 0);

    ApplyIpSec();

    return IMS_TRUE;
}

PROTECTED VIRTUAL void SipStreamSocketNotifier::Socket_OnConnectionReceived(IN ISocket* piSocket)
{
    SipSocket::Socket_OnConnectionReceived(piSocket);

    if (m_piListener != IMS_NULL)
    {
        m_piListener->StreamSocket_ConnectionReceived(this);

        // Accept & Close : A new connection MUST be extracted from the pending connection queue.
    }
}

PROTECTED VIRTUAL void SipStreamSocketNotifier::Socket_OnClosed(
        IN ISocket* piSocket, IN IMS_SINT32 nReason /*= ISocket::CLOSE_REASON_UNKNOWN*/)
{
    SipSocket::Socket_OnClosed(piSocket, nReason);
}
