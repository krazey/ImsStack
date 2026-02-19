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
#include "ServiceTrace.h"

#include "ISipDatagramSocketListener.h"
#include "SipDatagramSocket.h"
#include "SipDebug.h"
#include "SipMessageBuffer.h"
#include "SipPrivate.h"
#include "SipRtConfigUtils.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipDatagramSocket::SipDatagramSocket(IN IMS_SINT32 nSlotId) :
        SipSocket(nSlotId),
        m_piListener(IMS_NULL)
{
}

PUBLIC VIRTUAL SipDatagramSocket::~SipDatagramSocket()
{
    IMS_TRACE_D("DatagramSocket(D) (%s|%d)", SipDebug::GetIp(m_objSockAddr.GetIpAddress()),
            m_objSockAddr.GetPort(), 0);
}

PUBLIC VIRTUAL IMS_BOOL SipDatagramSocket::Create(
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

    m_objSockAddr.SetPort(nPort);
    m_objSockAddr.SetIpAddress(objIp);

    SetState(STATE_CONNECTING);

    IMS_TRACE_I("DatagramSocket(C) (%s|%d)",
            SipRtConfigUtils::IsRoutingInfoHiddenInLog(GetSlotId()) ? "***"
                                                                    : SipDebug::GetIp(objIp),
            nPort, 0);

    ApplyIpSec();

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_SINT32 SipDatagramSocket::Send(IN const IMS_BYTE* pBuffer,
        IN IMS_SINT32 nBuffLen, IN IMS_UINT32 nPort /*= 0*/,
        IN const IpAddress& objIp /*= IpAddress::NONE*/)
{
    if ((GetState() != STATE_CONNECTED) && (GetState() != STATE_CONNECTING))
    {
        return ISocket::RESULT_ERROR;
    }

    return m_piSocket->SendTo(pBuffer, nBuffLen, objIp, nPort);
}

PROTECTED VIRTUAL void SipDatagramSocket::Socket_OnDataReceived(IN ISocket* piSocket)
{
    RcPtr<SipMessageBuffer> pMessageBuffer = SipMessageBuffer::GetInstance();
    IMS_BYTE* pRecvBuffer = pMessageBuffer->GetBuffer(GetSlotId());

    IMS_MEM_Memset(pRecvBuffer, 0x00, pMessageBuffer->GetLength());

    IMS_UINT32 nPort = 0;
    IpAddress objIp;
    IMS_SINT32 nReadBytes =
            piSocket->ReceiveFrom(pRecvBuffer, pMessageBuffer->GetLength(), objIp, nPort);

    if (nReadBytes > 0)
    {
        // Ignore CR LF
        if ((nReadBytes == 2) && (pRecvBuffer[0] == 0x0D) && (pRecvBuffer[1] == 0x0A))
        {
            IMS_TRACE_I("CRLF is ignored by SIP transport layer", 0, 0, 0);
            return;
        }

        pRecvBuffer[nReadBytes] = '\0';

        ByteArray objRecvBuffer(pRecvBuffer, nReadBytes);

        if (m_piListener != IMS_NULL)
        {
            m_piListener->DatagramSocket_DataReceived(this, objRecvBuffer, objIp, nPort);
        }
    }

    SipSocket::Socket_OnDataReceived(piSocket);
}

PROTECTED VIRTUAL void SipDatagramSocket::Socket_OnSendEnabled(IN ISocket* piSocket)
{
    SetState(STATE_CONNECTED);
    SipSocket::Socket_OnSendEnabled(piSocket);
}
