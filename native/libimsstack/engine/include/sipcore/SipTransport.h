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
#ifndef SIP_TRANSPORT_H_
#define SIP_TRANSPORT_H_

#include "ImsSlot.h"

#include "ISipSocketListener.h"
#include "SipPrivate.h"
#include "SipSocket.h"
#include "SipTransportAddress.h"
#include "msg/SipAddrSpec.h"
#include "msg/SipMessage.h"

class ISipTransportListener;
class SipProfile;
class SipTransportHelper;

class SipTransport : public ImsSlot, public ISipSocketListener
{
public:
    SipTransport(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nType);
    virtual ~SipTransport();

    SipTransport(IN const SipTransport&) = delete;
    SipTransport& operator=(IN const SipTransport&) = delete;

public:
    inline IMS_SINT32 GetType() const { return m_nType; }

    SipSocket* CreateTcpClientSocket();
    IMS_BOOL EncodeMessage(IN_OUT ::SipMessage*& pSipMsg, OUT ByteArray& objBuffer);
    const SipTransportAddress& GetAddress(IN IMS_SINT32 nTaType = TA_NEAR) const;
    inline IMS_SINT32 GetPortC() const { return m_nNearEndPortC; }
    inline IMS_SINT32 GetPortFlowControl() const { return m_nPortFlowControl; }
    const IpAddress& GetIpAddress(IN IMS_SINT32 nTaType = TA_NEAR);
    IMS_SINT32 GetPort(IN IMS_SINT32 nTaType = TA_NEAR) const;
    IMS_SINT32 GetProtocol(IN IMS_SINT32 nTaType = TA_NEAR) const;
    inline IMS_SINT32 GetTransportExt() const { return m_nTransportExt; }
    inline IMS_BOOL HasPendingMessage() const { return m_pSendBuffer != IMS_NULL; }
    inline void InitRetransmissionFlag() { m_bIsRetransmission = IMS_FALSE; }
    IMS_BOOL InitTransportDetails(
            IN const SipTransport* pTransport, IN const SipProfile* pProfile = IMS_NULL);
    IMS_BOOL InitTransportOnMessageReceived(IN ::SipMessage* pSipMsg);
    inline IMS_BOOL IsIpSecRequired() const
    {
        return (((m_nTransportExt & Sip::TRANSPORT_EXT_IPSEC) != 0) ||
                ((m_nTransportExt & Sip::TRANSPORT_EXT_IPSEC_UDP_ENC) != 0));
    }
    inline IMS_BOOL IsTcpConnectionOnlyRequired() const
    {
        return ((m_nTransportExt & Sip::TRANSPORT_EXT_TCP_ONLY) != 0);
    }
    inline IMS_BOOL IsTransactionFlowControlRequired() const { return m_bTxnFlowControlRequired; }
    IMS_BOOL SendToNetwork(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen,
            IN const SipProfile* pProfile, IN IMS_BOOL bNotifyError = IMS_TRUE);
    void SetAddress(IN const SipTransportAddress& objTAddr, IN IMS_SINT32 nTaType = TA_NEAR);
    inline void SetExplicitTargetProtocol(IN IMS_BOOL bExplicitTargetProtocol)
    {
        m_bExplicitTargetProtocol = bExplicitTargetProtocol;
    }
    void SetIpAddress(IN const IpAddress& objIp, IN IMS_SINT32 nTaType = TA_NEAR);
    inline void SetListener(IN ISipTransportListener* piListener) { m_piListener = piListener; }
    void SetPort(IN IMS_SINT32 nPort, IN IMS_SINT32 nTaType = TA_NEAR);
    void SetProtocol(IN IMS_SINT32 nProtocol, IN IMS_SINT32 nTaType = TA_NEAR);
    inline void SetTransactionFlowControlRequired(IN IMS_BOOL bFlowControlRequired)
    {
        m_bTxnFlowControlRequired = bFlowControlRequired;
    }
    void SetTransportTuple(IN const IpAddress& objIp, IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC,
            IN IMS_SINT32 nPortFc = Sip::PORT_UNSPECIFIED,
            IN IMS_SINT32 nTransportExt = Sip::TRANSPORT_EXT_ANY);
    void StoreResource();

    inline virtual IMS_BOOL FormViaHeader(
            IN_OUT ::SipMessage*& /*pSipMsg*/, IN const SipProfile* pProfile = IMS_NULL)
    {
        (void)pProfile;
        return IMS_TRUE;
    }
    virtual IMS_BOOL ReserveResource(IN const SipProfile* pProfile = IMS_NULL);
    inline virtual IMS_BOOL UpdateDestinationInfo(IN ::SipMessage* /*pSipMsg*/,
            IN IMS_BOOL bRoutingLr = IMS_TRUE, IN SipAddrSpec* pImplicitRoute = IMS_NULL)
    {
        (void)bRoutingLr;
        (void)pImplicitRoute;
        return IMS_TRUE;
    }
    inline virtual IMS_SINT32 ValidateViaHeader(IN ::SipMessage* /*pSipMsg*/)
    {
        return SipPrivate::MESSAGE_VALID;
    }

    static AString CreateSocketErrorMessage(
            IN IMS_SINT32 nErrorCode, IN IMS_SINT32 nSocketType = SipSocketAddress::SOCKET_NONE);
    static IMS_BOOL GetHostNPortFromViaHeader(
            IN ::SipMessage* pSipMsg, OUT AString& strHost, OUT IMS_SINT32& nPort);
    static void ParseHostNPort(
            IN const AString& strHostNPort, OUT AString& strHost, OUT IMS_SINT32& nPort);
    // To display an SIP Protocol Message
    static void PrintMessage(IN IMS_SINT32 nSlotId, IN IMS_BOOL bSend,
            IN const SipTransportAddress& objFarEnd, IN const IMS_CHAR* pszMessage,
            IN IMS_SINT32 nLength);

protected:
    // ISipSocketListener interface
    void Socket_NotifyError(IN SipSocket* pSocket, IN IMS_SINT32 nErrorCode) override;
    void Socket_SendEnabled(IN SipSocket* pSocket) override;

    void CorrectNearEndAddress();
    SipTransportHelper* GetTransportHelper() const;
    inline IMS_BOOL IsExplicitTargetProtocolSelected() const { return m_bExplicitTargetProtocol; }
    inline IMS_BOOL IsFlowControlPortConfigured() const
    {
        return Sip::IsPortSpecified(m_nPortFlowControl);
    }
    inline IMS_BOOL IsIpSecUdpEncRequired() const
    {
        return ((m_nTransportExt & Sip::TRANSPORT_EXT_IPSEC_UDP_ENC) != 0);
    }
    inline IMS_BOOL IsTargetProtocolConfigured() const
    {
        return (((m_nTransportExt & Sip::TRANSPORT_EXT_UDP) != 0) ||
                ((m_nTransportExt & Sip::TRANSPORT_EXT_TCP) != 0) ||
                ((m_nTransportExt & Sip::TRANSPORT_EXT_TLS) != 0));
    }
    void NotifyTransportError(IN IMS_SINT32 nErrorCode);

private:
    IMS_BOOL IsNetworkConnectionAvailable() const;
    SipSocket* LookupSocket() const;
    void ReleaseSocket();
    IMS_BOOL ReserveSocket(IN const SipProfile* pProfile = IMS_NULL);
    IMS_BOOL TransmitMessage(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen);

public:
    /// Type of transport layer
    enum
    {
        TYPE_CLIENT = 0,
        TYPE_SERVER
    };

    /// Type of transport address
    enum
    {
        /// UE
        TA_NEAR = 0,
        /// Proxy
        TA_FAR
    };

    // A default threshold value when MTU size is unknown; The default MTU size will be 1500.
    // 200 bytes is a buffer for collecting the Record-Route.
    // enum { MSG_THRESHOLD = 1300 };

private:
    // Type of transport: client, server
    IMS_SINT32 m_nType;
    // Transport protocol set by the application
    IMS_BOOL m_bExplicitTargetProtocol;
    // Checks if the message is a retransmission or not
    IMS_BOOL m_bIsRetransmission;
    // Indicates that transaction flow control is required or not.
    //    REGISTER transaction - flow control not required
    //    non-REGISTER transaction - flow control required
    // By using this information, the transport layer can make a decision
    // whether TCP connection SHOULD be created or opened(just refers).
    IMS_BOOL m_bTxnFlowControlRequired;
    // Device's client port number to send an outgoing request
    IMS_SINT32 m_nNearEndPortC;
    SipTransportAddress m_objNearEnd;
    SipTransportAddress m_objFarEnd;
    // TCP only maybe
    ByteArray* m_pSendBuffer;
    // MULTI_REG_TRANSPORT
    IMS_SINT32 m_nTransportExt;
    // RFC5626_FLOW_CONTROL
    IMS_SINT32 m_nPortFlowControl;
    // Socket for message transmission
    SipSocket* m_pSocket;
    ISipTransportListener* m_piListener;
};

#endif
