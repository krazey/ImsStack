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

#include "ISipKeepAliveHelperListener.h"
#include "SipDebug.h"
#include "SipFactoryProxy.h"
#include "SipKeepAliveHelper.h"
#include "SipRtConfigUtils.h"
#include "SipSocket.h"
#include "SipTransportHelper.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipKeepAliveHelper::SipKeepAliveHelper(IN IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        m_piListener(IMS_NULL)
{
    m_objNearEnd.SetType(SipSocketAddress::SOCKET_UDP);
    m_objFarEnd.SetType(SipSocketAddress::SOCKET_UDP);
}

PRIVATE VIRTUAL void SipKeepAliveHelper::Destroy()
{
    SipSocket* pSocket = IMS_NULL;
    SipTransportHelper* pTransportHelper =
            SipFactoryProxy::GetInstance()->GetTransportHelper(GetSlotId());

    if (m_objNearEnd.GetType() == SipSocketAddress::SOCKET_UDP)
    {
        pSocket = pTransportHelper->Open(m_objNearEnd);
    }
    else
    {
        pSocket = pTransportHelper->OpenStreamSocket(m_objNearEnd, m_objFarEnd);
    }

    if (pSocket != IMS_NULL)
    {
        pSocket->SetKeepAliveListener(IMS_NULL);
    }

    delete this;
}

PRIVATE VIRTUAL IMS_RESULT SipKeepAliveHelper::SendPacket(IN const ByteArray& objPacket)
{
    if (m_objFarEnd.GetIpAddress().Equals(IpAddress::NONE) || (m_objFarEnd.GetPort() <= 0))
    {
        IMS_TRACE_E(0, "IP address & port number MUST be specified to send the keep-alive packet",
                0, 0, 0);
        return IMS_FAILURE;
    }

    if (objPacket.GetLength() == 0)
    {
        IMS_TRACE_E(0, "No packet", 0, 0, 0);
        return IMS_FAILURE;
    }

    SipSocket* pSocket = IMS_NULL;
    SipTransportHelper* pTransportHelper =
            SipFactoryProxy::GetInstance()->GetTransportHelper(GetSlotId());

    if (m_objNearEnd.GetType() == SipSocketAddress::SOCKET_UDP)
    {
        pSocket = pTransportHelper->Open(m_objNearEnd);
    }
    else
    {
        pSocket = pTransportHelper->OpenStreamSocket(m_objNearEnd, m_objFarEnd);
    }

    if (pSocket == IMS_NULL)
    {
        IMS_TRACE_D("NearEnd: %s|%d|%d", SipDebug::GetIp(m_objNearEnd.GetIpAddress()),
                m_objNearEnd.GetPort(), m_objNearEnd.GetType());
        // LOG_EXCLUDING_SERVER_INFO
        IMS_TRACE_D("FarEnd: %s|%d|%d",
                SipRtConfigUtils::IsRoutingInfoHiddenInLog(GetSlotId())
                        ? "***"
                        : SipDebug::GetIp(m_objFarEnd.GetIpAddress()),
                m_objFarEnd.GetPort(), m_objFarEnd.GetType());
        IMS_TRACE_E(0, "Finding the socket failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    pSocket->SetKeepAliveListener(this);

    if (pSocket->Send(objPacket.GetData(), objPacket.GetLength(), m_objFarEnd.GetPort(),
                m_objFarEnd.GetIpAddress()) < 0)
    {
        IMS_TRACE_E(0, "Sending the keep-alive packet failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL void SipKeepAliveHelper::SetListener(IN ISipKeepAliveHelperListener* piListener)
{
    m_piListener = piListener;
}

PRIVATE VIRTUAL void SipKeepAliveHelper::SetTransportTupleD(
        IN const IpAddress& objIp, IN IMS_SINT32 nPort)
{
    m_objFarEnd.SetIpAddress(objIp);
    m_objFarEnd.SetPort(nPort);
}

PRIVATE VIRTUAL void SipKeepAliveHelper::SetTransportTupleS(IN const IpAddress& objIp,
        IN IMS_SINT32 nPort, IN IMS_SINT32 nProtocol /*= Sip::TRANSPORT_UDP*/)
{
    m_objNearEnd.SetIpAddress(objIp);
    m_objNearEnd.SetPort(nPort);

    if ((nProtocol == Sip::TRANSPORT_TCP) || (nProtocol == Sip::TRANSPORT_TLS))
    {
        m_objNearEnd.SetType(SipSocketAddress::SOCKET_TCP_CLIENT);
    }
    else
    {
        m_objNearEnd.SetType(SipSocketAddress::SOCKET_UDP);
    }

    m_objFarEnd.SetType(m_objNearEnd.GetType());
}

PRIVATE VIRTUAL void SipKeepAliveHelper::KeepAlive_PongReceived()
{
    if (m_piListener != IMS_NULL)
    {
        m_piListener->KeepAliveHelper_PongReceived();
    }
}
