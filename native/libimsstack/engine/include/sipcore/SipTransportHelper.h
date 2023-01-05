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
#ifndef SIP_TRANSPORT_HELPER_H_
#define SIP_TRANSPORT_HELPER_H_

#include "ImsMap.h"

#include "EngineActivity.h"
#include "ISipDatagramSocketListener.h"
#include "ISipStreamSocketListener.h"
#include "ISipTransportHelper.h"
#include "SipSocketAddress.h"
#include "SipTransportAddress.h"

class ISipLocalDnsQueryListener;
class ISipSocketListener;
class ISipTransportMessageListener;

/**
 * TCP active connection MUST be created at the start time of raw SIP message transmission.
 */
class SipTransportHelper :
        public EngineActivity,
        public ISipTransportHelper,
        public ISipStreamSocketListener,
        public ISipDatagramSocketListener
{
public:
    SipTransportHelper();
    virtual ~SipTransportHelper();

    SipTransportHelper(IN const SipTransportHelper&) = delete;
    SipTransportHelper& operator=(IN const SipTransportHelper&) = delete;

public:
    // EngineActivity class
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

    // Only SipConnectionNotifier
    void Clear();
    SipSocket* Create(IN const SipSocketAddress& objSockAddr);
    SipSocket* CreateStreamSocket(
            IN const SipSocketAddress& objSockAddr, IN const SipSocketAddress& objFarEnd);
    void Destroy(IN SipSocket*& pSocket, IN ISipSocketListener* piListener);
    void DestroyStreamSocket(
            IN const SipSocketAddress& objSockAddr, IN const SipSocketAddress& objFarEnd);
    SipSocket* Open(IN const SipSocketAddress& objSockAddr);
    SipSocket* OpenStreamSocket(
            IN const SipSocketAddress& objSockAddr, IN const SipSocketAddress& objFarEnd);
    inline void SetMessageListener(IN ISipTransportMessageListener* piListener)
    {
        m_piMessageListener = piListener;
    }

    // MULTI_REG_TRANSPORT
    IMS_SINT32 AttachClientInitiatedConnection(IN SipSocket* pSocket);
    void DetachClientInitiatedConnection(IN SipSocket* pSocket);
    IMS_BOOL IsClientInitiatedConnection(IN SipSocket* pSocket) const;

    // LOCAL_DNS_QUERY
    IMS_BOOL GetHostByName(IN const IPAddress& objLocalIp, IN const AString& strHostname,
            OUT IPAddress& objHostIp);

private:
    // ISipTransportHelper
    void ApplyIpSecForServerSockets() override;
    void DestroyAllSockets(
            IN IMS_SINT32 nMethod = 0, IN const IPAddress& objLocalIp = IPAddress::NONE) override;
    void DestroyTcpSocket(IN const IPAddress& objSrcIp, IN IMS_UINT32 nSrcPort,
            IN const IPAddress& objDstIp, IN IMS_UINT32 nDstPort,
            IN IMS_BOOL bIsConnectionByPeer = IMS_FALSE) override;
    void SetIpQos(IN SipRtConfig::IpQos* pIpQos) override;
    void SetKeepAlivePolicy(IN const IPAddress& objSrcIp, IN IMS_UINT32 nSrcPort,
            IN const IPAddress& objDstIp, IN IMS_UINT32 nDstPort,
            IN IMS_SINT32 nPolicy = (-1) /* default */) override;
    // LOCAL_DNS_QUERY
    void SetLocalDnsQueryListener(IN ISipLocalDnsQueryListener* piListener) override;

    // ISipDatagramSocketListener
    void DatagramSocket_DataReceived(IN SipSocket* pSocket, IN const ByteArray& objBuffer,
            IN const IPAddress& objIp, IN IMS_SINT32 nPort) override;
    // ISipStreamSocketListener
    void StreamSocket_ConnectionReceived(IN SipSocket* pSocket) override;
    void StreamSocket_DataReceived(IN SipSocket* pSocket, IN_OUT ByteArray& objBuffer) override;
    void StreamSocket_KeepAliveExpired(IN SipSocket* pSocket) override;
    void StreamSocket_PassiveClosed(IN SipSocket* pSocket) override;

    IMS_BOOL AttachSocket(IN SipSocket* pSocket);
    IMS_BOOL IsSocketPresent(IN const SipSocket* pSocket) const;
    SipSocket* LookupSocket(
            IN const SipSocketAddress& objSockAddr, IN IMS_BOOL bDetach = IMS_FALSE);
    SipSocket* LookupSocket(IN const SipSocket& objSocket, IN IMS_BOOL bDetach = IMS_FALSE);
    SipSocket* LookupStreamSocket(IN const SipSocketAddress& objSockAddr);
    SipSocket* LookupStreamSocket(
            IN const SipSocketAddress& objSockAddr, IN const SipSocketAddress& objFarEnd);

private:
    /// Event for message processing
    enum
    {
        AMSG_PROCESS_MESSAGE = AMSG_USER,
        AMSG_DESTROY_ALL_SOCKETS
    };

    /// Result values for the completeness of a raw SIP message (TCP only)
    enum
    {
        MESSAGE_ERROR = (-1),
        MESSAGE_COMPLETE = 0,
        MESSAGE_INCOMPLETE
    };

    class TransportBuffer
    {
        friend class SipTransportHelper;

        ByteArray m_objData;
        // Destination information of a message (on the basis of sender)
        SipTransportAddress m_objNearEnd;
        // Source information of a message (on the basis of sender)
        SipTransportAddress m_objFarEnd;

        inline TransportBuffer() {}
        inline TransportBuffer(IN const TransportBuffer& other) :
                m_objData(other.m_objData),
                m_objNearEnd(other.m_objNearEnd),
                m_objFarEnd(other.m_objFarEnd)
        {
        }
        inline ~TransportBuffer() {}
        inline TransportBuffer& operator=(IN const TransportBuffer& other)
        {
            if (this != &other)
            {
                m_objData = other.m_objData;
                m_objNearEnd = other.m_objNearEnd;
                m_objFarEnd = other.m_objFarEnd;
            }

            return (*this);
        }

        // DEBUG message
        void DisplayMessage(IN IMS_SINT32 nSlotId);
    };

    IMSList<SipSocket*> m_objSockets;
    IMSList<TransportBuffer*> m_objBuffers;
    ISipTransportMessageListener* m_piMessageListener;
    // MULTI_REG_TRANSPORT :: <Socket object's pointer, count>
    IMSMap<IMS_UINTP, IMS_SINT32> m_objClientInitiatedConnections;
    // LOCAL_DNS_QUERY
    // 1) Test purpose
    // 2) Using the application layer's DNS query result
    ISipLocalDnsQueryListener* m_piDnsQueryListener;
};

#endif
