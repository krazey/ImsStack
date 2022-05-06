/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_CONNECTION_NOTIFIER_H_
#define _SIP_CONNECTION_NOTIFIER_H_

#include "Connection.h"
#include "ISIPServerTransactionStateListener.h"
#include "ISIPSocketListener.h"
#include "SipProfile.h"
#include "SIPTransportAddress.h"

class SIPTransportHelper;
class SipParameter;
class SIPSocket;
class SIPSocketAddress;
class SIPServerTransactionState;
class ISipDialog;
class ISipServerConnection;
class IOnSIPServerConnectionListener;
class IOnSIPConnectionNotifierErrorListener;

class SIPConnectionNotifier :
        public Connection,
        public ISIPServerTransactionStateListener,
        public ISIPSocketListener
{
public:
    SIPConnectionNotifier(IN IMS_SINT32 nScheme_, IN IMS_SINT32 nPort_,
            IN CONST AString& strParams_, IN IMS_BOOL bSharedMode_ = IMS_FALSE);
    virtual ~SIPConnectionNotifier();

private:
    SIPConnectionNotifier(IN CONST SIPConnectionNotifier& objRHS);
    SIPConnectionNotifier& operator=(IN CONST SIPConnectionNotifier& objRHS);

public:
    // IConnection interface
    virtual void Close();

    // ISipConnectionNotifier interface
    ISipServerConnection* AcceptAndOpen();
    const IPAddress& GetLocalAddress() const;
    IMS_SINT32 GetLocalPort() const;
    void SetListener(IN IOnSIPServerConnectionListener* piListener);

    //// IMS extensions
    ISipServerConnection* AcceptAndOpen(OUT ISipDialog*& piOrigDialog);
    AString GetContactAddress() const;
    // MULTI_REG_SIP_PROFILE
    SipProfile* GetSIPProfile() const;
    IMS_BOOL IsTransportResourceReserved(IN IMS_SINT32 nType = TRANSPORT_ALL) const;
    IMS_RESULT ReserveTransportResource(IN CONST IPAddress& objIPA, IN IMS_SINT32 nPortS,
            IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFlowControl);
    IMS_RESULT RestoreTransportResource(
            IN IMS_SINT32 nType, IN CONST IPAddress& objPeerIP, IN IMS_SINT32 nPeerPort);
    void SetErrorListener(IN IOnSIPConnectionNotifierErrorListener* piListener);
    void SetFilter(IN CONST AString& strFilter);
    void SetFromAndContact(IN CONST AString& strFrom, IN CONST AString& strDisplayName,
            IN CONST AString& strUserInfo);
    // MULTI_REG_SIP_PROFILE
    void SetSIPProfile(IN SipProfile* pProfile);
    void UpdatePortFlowControl(IN IMS_SINT32 nPort);
    void UpdatePortUC(IN IMS_SINT32 nPort);

    // Extension methods
    AString GetUserIdentity() const;
    IMS_BOOL IsSameConnectionNotifier(IN CONST SIPTransportAddress& objTA) const;
    inline IMS_BOOL IsSharedMode() const { return (nMode == SHARED) ? IMS_TRUE : IMS_FALSE; }

protected:
    // ISIPServerTransactionStateListener interface
    virtual void ServerTransactionState_ForkedRequestReceived(
            IN SIPServerTransactionState* pSTState, IN SIPDialogEx* pOrigDialogEx);
    virtual void ServerTransactionState_RequestCreated(IN SIPServerTransactionState* pSTState);
    virtual void ServerTransactionState_RequestReceived(IN SIPServerTransactionState* pSTState);

    // ISIPSocketListener interface
    virtual void Socket_NotifyError(IN SIPSocket* pSocket, IN IMS_SINT32 nErrorCode);
    virtual void Socket_SendEnabled(IN SIPSocket* pSocket);

private:
    void ClearTransportResource();
    // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
    void ControlUDPClientReference(IN IMS_SINT32 nControl);
    IMS_BOOL CreateClientInitiatedConnection(IN IMS_SINT32 nPort, IN SIPSocketAddress* pFarEnd);
    IMS_BOOL ConnectClientInitiatedConnection();
    void DestroyClientInitiatedConnection(IN IMS_SINT32 nPort);
    void ExtractProperties(IN CONST AString& strParams);
    SIPTransportHelper* GetTransportHelper() const;
    IMS_BOOL IsClientInitiatedConnectionRequired() const;
    IMS_BOOL IsIPSecRequired() const;
    IMS_BOOL IsTCPConnectionOnlyRequired() const;

    IMS_RESULT RestoreTransportResourceForClientInitiatedConnection(
            IN CONST IPAddress& objPeerIP, IN IMS_SINT32 nPeerPort);
    IMS_RESULT RestoreTransportResourceForServerConnection();

private:
    enum
    {
        SHARED = 0,
        DEDICATED
    };

    // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
    enum
    {
        CTRL_CREATE = 1,
        CTRL_DESTROY = 2
    };

    // Error codes
    enum
    {
        TRANSPORT_ERROR_TCP_CLIENT = 1,
        TRANSPORT_ERROR_TCP_SERVER = 2,
        TRANSPORT_ERROR_UDP_SERVER = 3
    };

    // Types of transport resource
    enum
    {
        TRANSPORT_CLIENT_INITIATED_CONNECTION = 0x01,
        TRANSPORT_SERVER_CONNECTION = 0x02,
        TRANSPORT_ALL = (TRANSPORT_CLIENT_INITIATED_CONNECTION | TRANSPORT_SERVER_CONNECTION)
    };

#ifdef __JSR180_ONLY__
    struct UserProfile
    {
        // Active SIP user identity : From header
        AString strFrom;

        // Terminal SIP settings : Contact header
        AString strDisplayName;
        AString strUserInfo;
    };
#endif

    class ForkedTxnState
    {
    public:
        ForkedTxnState(IN SIPDialogEx* pDialogEx_, IN SIPServerTransactionState* pSTState_) :
                pDialogEx(pDialogEx_),
                pSTState(pSTState_)
        {
        }

        ~ForkedTxnState() {}

    public:
        RCPtr<SIPDialogEx> pDialogEx;
        RCPtr<SIPServerTransactionState> pSTState;
    };

    static IMS_SINT32* pGlobalSystemPort;

    IMS_SINT32 nMode;  // Shared mode or Dedicated mode

    // Connetor.open() info.
    IMS_SINT32 nScheme;
    IMS_SINT32 nPort;
    IMS_SINT32 nTransportProtocol;  // UDP only / TCP only / Both (udp/tcp)
    // MULTI_REG_TRANSPORT
    IMS_SINT32 nTransportExt;
    AString strType;
    AString strFilter;  // For PushRegistry
    IMSList<SipParameter*> objParameters;

#ifdef __JSR180_ONLY__
    UserProfile stUserProfile;
#endif

    // Identifier for IP connectivity
    IPAddress objIPA;
    IMS_SINT32 nPortC;
    SIPSocket* pSocket_UDP;
    SIPSocket* pSocket_TCP;
    // RFC5626_FLOW_CONTROL
    IMS_SINT32 nPortFlowControl;

    // FIX_MESSAGE_ORDER_ON_MIXED_TRANSPORT_USE
    // The patch grabs the reference of the datagram socket (UDP client socket)
    // to prevent misordering SIP messages when REG or re-REG is done.
    // It's only applied if IPSec is turned on and this assumes that
    // UDP transport protocol may be used in the near future.
    SIPSocket* pSocket_UDPClient;
    // For client initiated connection (TCP_ONLY / FLOW_CONTROL)
    SIPSocket* pSocket_TCPClient;
    SIPSocketAddress* pSA_FarEnd;

    // MULTI_REG_SIP_PROFILE
    RCPtr<SipProfile> pSIPProfile;
    // Queue for incoming request messages
    IMSList<RCPtr<SIPServerTransactionState>> objTxnStates;
    IMSList<ForkedTxnState*> objForkedTxnStates;
    IOnSIPServerConnectionListener* piListener;
    IOnSIPConnectionNotifierErrorListener* piErrorListener;
};

#endif  // _SIP_CONNECTION_NOTIFIER_H_
