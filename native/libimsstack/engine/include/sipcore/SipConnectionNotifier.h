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
#ifndef SIP_CONNECTION_NOTIFIER_H_
#define SIP_CONNECTION_NOTIFIER_H_

#include "Connection.h"
#include "ISipServerTransactionStateListener.h"
#include "ISipSocketListener.h"
#include "Sip.h"
#include "SipProfile.h"
#include "SipServerTransactionState.h"
#include "SipTransportAddress.h"

class IOnSipConnectionNotifierErrorListener;
class IOnSipServerConnectionListener;
class ISipDialog;
class ISipServerConnection;
class SipParameter;
class SipSocket;
class SipSocketAddress;
class SipTransportHelper;

class SipConnectionNotifier :
        public Connection,
        public ISipServerTransactionStateListener,
        public ISipSocketListener
{
public:
    SipConnectionNotifier(IN IMS_SINT32 nScheme, IN IMS_SINT32 nPort, IN const AString& strParams,
            IN IMS_BOOL bSharedMode = IMS_FALSE);
    virtual ~SipConnectionNotifier();

    SipConnectionNotifier(IN const SipConnectionNotifier&) = delete;
    SipConnectionNotifier& operator=(IN const SipConnectionNotifier&) = delete;

public:
    // IConnection interface
    void Close() override;

    // ISipConnectionNotifier interface
    ISipServerConnection* AcceptAndOpen();
    inline const IpAddress& GetLocalAddress() const { return m_objIpAddr; }
    inline IMS_SINT32 GetLocalPort() const { return m_nPort; }
    inline void SetListener(IN IOnSipServerConnectionListener* piListener)
    {
        m_piListener = piListener;
    }

    ISipServerConnection* AcceptAndOpen(OUT ISipDialog*& piOrigDialog);
    AString GetContactAddress() const;
    inline SipProfile* GetSipProfile() const { return m_pSipProfile.Get(); }
    IMS_BOOL IsTransportResourceReserved(IN IMS_SINT32 nType = TRANSPORT_ALL) const;
    IMS_RESULT ReserveTransportResource(IN const IpAddress& objIp, IN IMS_SINT32 nPortS,
            IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFlowControl);
    IMS_RESULT RestoreTransportResource(
            IN IMS_SINT32 nType, IN const IpAddress& objPeerIp, IN IMS_SINT32 nPeerPort);
    inline void SetErrorListener(IN IOnSipConnectionNotifierErrorListener* piListener)
    {
        m_piErrorListener = piListener;
    }
    inline void SetFilter(IN const AString& strFilter) { m_strFilter = strFilter; }
    void SetFromAndContact(IN const AString& strFrom, IN const AString& strDisplayName,
            IN const AString& strUserInfo);
    inline void SetSipProfile(IN SipProfile* pProfile) { m_pSipProfile = pProfile; }
    void UpdatePortFlowControl(IN IMS_SINT32 nPort);
    void UpdatePortUc(IN IMS_SINT32 nPort);

    // Extension methods
    inline AString GetUserIdentity() const
    {
        return AString("\"Anonymous\" <sip:anonymous@anonymous.invalid>");
    }
    IMS_BOOL IsSameConnectionNotifier(IN const SipTransportAddress& objTAddr) const;
    inline IMS_BOOL IsSharedMode() const { return (m_nMode == SHARED) ? IMS_TRUE : IMS_FALSE; }

protected:
    // ISipServerTransactionStateListener interface
    void ServerTransactionState_ForkedRequestReceived(
            IN SipServerTransactionState* pStState, IN SipDialogEx* pOrigDialogEx) override;
    void ServerTransactionState_RequestCreated(IN SipServerTransactionState* pStState) override;
    void ServerTransactionState_RequestReceived(IN SipServerTransactionState* pStState) override;

    // ISipSocketListener interface
    void Socket_NotifyError(IN SipSocket* pSocket, IN IMS_SINT32 nErrorCode) override;
    void Socket_SendEnabled(IN SipSocket* pSocket) override;

private:
    void ClearTransportResource();
    // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
    void ControlUdpClientReference(IN IMS_SINT32 nControl);
    IMS_BOOL CreateClientInitiatedConnection(IN IMS_SINT32 nPort, IN SipSocketAddress* pFarEnd);
    IMS_BOOL ConnectClientInitiatedConnection();
    void DestroyClientInitiatedConnection(IN IMS_SINT32 nPort);
    void ExtractProperties(IN const AString& strParams);
    SipTransportHelper* GetTransportHelper() const;
    inline IMS_BOOL IsClientInitiatedConnectionRequired() const
    {
        return Sip::IsPortSpecified(m_nPortFlowControl);
    }
    inline IMS_BOOL IsIpSecRequired() const
    {
        return ((m_nTransportExt & Sip::TRANSPORT_EXT_IPSEC) != 0);
    }
    inline IMS_BOOL IsTcpConnectionOnlyRequired() const
    {
        return ((m_nTransportExt & Sip::TRANSPORT_EXT_TCP_ONLY) != 0);
    }

    IMS_RESULT RestoreTransportResourceForClientInitiatedConnection(
            IN const IpAddress& objPeerIp, IN IMS_SINT32 nPeerPort);
    IMS_RESULT RestoreTransportResourceForServerConnection();

private:
    enum
    {
        SHARED = 0,
        DEDICATED
    };

    /// FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
    enum
    {
        CTRL_CREATE = 1,
        CTRL_DESTROY = 2
    };

    /// Error codes
    enum
    {
        TRANSPORT_ERROR_TCP_CLIENT = 1,
        TRANSPORT_ERROR_TCP_SERVER = 2,
        TRANSPORT_ERROR_UDP_SERVER = 3
    };

    /// Types of transport resource
    enum
    {
        TRANSPORT_CLIENT_INITIATED_CONNECTION = 0x01,
        TRANSPORT_SERVER_CONNECTION = 0x02,
        TRANSPORT_ALL = (TRANSPORT_CLIENT_INITIATED_CONNECTION | TRANSPORT_SERVER_CONNECTION)
    };

    class ForkedTxnState
    {
    public:
        ForkedTxnState(IN SipDialogEx* pDialogEx_, IN SipServerTransactionState* pStState_) :
                pDialogEx(pDialogEx_),
                pStState(pStState_)
        {
        }

        ~ForkedTxnState() {}

    public:
        RcPtr<SipDialogEx> pDialogEx;
        RcPtr<SipServerTransactionState> pStState;
    };

    static IMS_SINT32* s_pGlobalSystemPort;

    IMS_SINT32 m_nMode;  // Shared mode or Dedicated mode
    // Connetor.open() info.
    IMS_SINT32 m_nScheme;
    IMS_SINT32 m_nPort;
    IMS_SINT32 m_nTransportProtocol;  // UDP only / TCP only / Both (udp/tcp)
    // MULTI_REG_TRANSPORT
    IMS_SINT32 m_nTransportExt;
    AString m_strType;
    AString m_strFilter;  // For PushRegistry
    ImsList<SipParameter*> m_objParameters;

    // Identifier for IP connectivity
    IpAddress m_objIpAddr;
    IMS_SINT32 m_nPortC;
    SipSocket* m_pSocketUdp;
    SipSocket* m_pSocketTcp;
    // RFC5626_FLOW_CONTROL
    IMS_SINT32 m_nPortFlowControl;
    // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
    // The patch grabs the reference of the datagram socket (UDP client socket)
    // to prevent misordering SIP messages when REG or re-REG is done.
    // It's only applied if IPSec is turned on and this assumes that
    // UDP transport protocol may be used in the near future.
    SipSocket* m_pSocketUdpClient;
    // For client initiated connection (TCP_ONLY / FLOW_CONTROL)
    SipSocket* m_pSocketTcpClient;
    SipSocketAddress* m_pSockAddrFarEnd;
    RcPtr<SipProfile> m_pSipProfile;
    // Queue for incoming request messages
    ImsList<RcPtr<SipServerTransactionState>> m_objTxnStates;
    ImsList<ForkedTxnState*> m_objForkedTxnStates;
    IOnSipServerConnectionListener* m_piListener;
    IOnSipConnectionNotifierErrorListener* m_piErrorListener;
};

#endif
