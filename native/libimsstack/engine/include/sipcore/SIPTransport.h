/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_TRANSPORT_H_
#define _SIP_TRANSPORT_H_

#include "ImsSlot.h"
#include "SIPPrivate.h"
#include "SIPStackHeaders.h"
#include "SIPTransportAddress.h"
#include "ISIPSocketListener.h"
#include "SIPSocket.h"

class SipProfile;
class ISIPTransportErrorListener;
class SIPTransportHelper;

class SIPTransport : public ImsSlot, public ISIPSocketListener
{
public:
    SIPTransport(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nType_);
    virtual ~SIPTransport();

private:
    SIPTransport(IN const SIPTransport& objRHS);
    SIPTransport& operator=(IN const SIPTransport& objRHS);

public:
    inline IMS_SINT32 GetType() const { return nType; }

    SIPSocket* CreateTCPClientSocket();
    IMS_BOOL EncodeMessage(IN_OUT SipMessage*& pstMessage, OUT ByteArray& objBuffer);
    const SIPTransportAddress& GetAddress(IN IMS_SINT32 nTA = TA_NEAR) const;
    IMS_SINT32 GetPortC() const;
    IMS_SINT32 GetPortFlowControl() const;
    const IPAddress& GetIPAddress(IN IMS_SINT32 nTA = TA_NEAR);
    IMS_SINT32 GetPort(IN IMS_SINT32 nTA = TA_NEAR) const;
    IMS_SINT32 GetProtocol(IN IMS_SINT32 nTA = TA_NEAR) const;
    IMS_SINT32 GetTransportExt() const;
    void InitRetransmissionFlag();
    // MULTI_REG_SIP_PROFILE
    IMS_BOOL InitTransportDetails(
            IN CONST SIPTransport* pTransport, IN CONST SipProfile* pSIPProfile = IMS_NULL);
    IMS_BOOL InitTransportOnMessageReceived(IN SipMessage* pstMessage);
    IMS_BOOL IsIPSecRequired() const;
    IMS_BOOL IsTCPConnectionOnlyRequired() const;
    IMS_BOOL IsTransactionFlowControlRequired() const;
    IMS_BOOL SendToNetwork(IN CONST IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen,
            IN IMS_BOOL bNotifyError = IMS_TRUE);
    void SetAddress(IN CONST SIPTransportAddress& objTA, IN IMS_SINT32 nTA = TA_NEAR);
    void SetExplicitTargetProtocol(IN IMS_BOOL bExplicitTargetProtocol);
    void SetIPAddress(IN CONST IPAddress& objIPAddress, IN IMS_SINT32 nTA = TA_NEAR);
    void SetListener(IN ISIPTransportErrorListener* piListener);
    void SetPort(IN IMS_SINT32 nPort, IN IMS_SINT32 nTA = TA_NEAR);
    void SetProtocol(IN IMS_SINT32 nProtocol, IN IMS_SINT32 nTA = TA_NEAR);
    void SetTransactionFlowControlRequired(IN IMS_BOOL bFlowControlRequired);
    void SetTransportTuple(IN CONST IPAddress& objIPA, IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC,
            IN IMS_SINT32 nPortFC = Sip::PORT_UNSPECIFIED,
            IN IMS_SINT32 nTransportExt = 0 /* ANY */);
    void StoreResource();

    // MULTI_REG_SIP_PROFILE
    virtual IMS_BOOL FormViaHeader(
            IN_OUT SipMessage*& pstMessage, IN CONST SipProfile* pSIPProfile = IMS_NULL);
    // MULTI_REG_SIP_PROFILE
    virtual IMS_BOOL ReserveResource(IN CONST SipProfile* pSIPProfile = IMS_NULL);
    virtual IMS_BOOL UpdateDestinationInfo(IN SipMessage* pstMessage,
            IN IMS_BOOL bRoutingLR = IMS_TRUE, IN SipAddrSpec* pstImplicitRoute = IMS_NULL);
    virtual IMS_SINT32 ValidateViaHeader(IN SipMessage* pstMessage);

    static AString CreateSocketErrorMessage(
            IN IMS_SINT32 nErrorCode, IN IMS_SINT32 nSocketType = SIPSocketAddress::SOCKET_NONE);
    static IMS_BOOL GetHostNPortFromViaHeader(
            IN SipMessage* pstMessage, OUT AString& strHost, OUT IMS_SINT32& nPort);
    static void ParseHostNPort(
            IN CONST AString& strHostNPort, OUT AString& strHost, OUT IMS_SINT32& nPort);
    // To display an SIP Protocol Message
    static void PrintMessage(IN IMS_SINT32 nSlotId, IN IMS_BOOL bSEND,
            IN CONST SIPTransportAddress& objTA_FarEnd, IN CONST IMS_CHAR* pszMessage,
            IN IMS_SINT32 nLength);

protected:
    // ISIPSocketListener interface
    virtual void Socket_NotifyError(IN SIPSocket* pSocket, IN IMS_SINT32 nErrorCode);
    virtual void Socket_SendEnabled(IN SIPSocket* pSocket);

    void CorrectNearEndAddress();
    SIPTransportHelper* GetTransportHelper() const;
    IMS_BOOL IsExplicitTargetProtocolSelected() const;
    IMS_BOOL IsFlowControlPortConfigured() const;
    IMS_BOOL IsIPSecUDPEncRequired() const;
    IMS_BOOL IsTargetProtocolConfigured() const;
    void NotifyTransportError(IN IMS_SINT32 nErrorCode);

private:
    IMS_BOOL IsNetworkConnectionAvailable() const;
    SIPSocket* LookupSocket() const;
    void ReleaseSocket();
    // MULTI_REG_SIP_PROFILE
    IMS_BOOL ReserveSocket(IN CONST SipProfile* pSIPProfile = IMS_NULL);
    IMS_BOOL TransmitMessage(IN CONST IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen);

public:
    // Type of transport layer
    enum
    {
        TYPE_CLIENT = 0,
        TYPE_SERVER
    };

    // Type of transport address
    enum
    {
        // UE
        TA_NEAR = 0,
        // Proxy
        TA_FAR
    };

    // A default threshold value when MTU size is unknown; The default MTU size will be 1500.
    // 200 bytes is a buffer for collecting the Record-Route.
    // enum { MSG_THRESHOLD = 1300 };

private:
    // Type of transport: client, server
    IMS_SINT32 nType;
    // Transport protocol set by the application
    IMS_BOOL bExplicitTargetProtocol;
    // Checks if the message is a retransmission or not
    IMS_BOOL bIsRetransmission;
    // Indicates that transaction flow control is required or not.
    //    REGISTER transaction - flow control not required
    //    non-REGISTER transaction - flow control required
    // By using this information, the transport layer can make a decision
    // whether TCP connection SHOULD be created or opened(just refers).
    IMS_BOOL bTxnFlowControlRequired;

    // Device's client port number to send an outgoing request
    IMS_SINT32 nNearEnd_PortC;
    SIPTransportAddress objTA_NearEnd;
    SIPTransportAddress objTA_FarEnd;
    // TCP only maybe
    ByteArray* pSendBuffer;
    // MULTI_REG_TRANSPORT
    IMS_SINT32 nTransportExt;
    // RFC5626_FLOW_CONTROL
    IMS_SINT32 nPortFlowControl;

    // Socket for message transmission
    SIPSocket* pSocket;
    ISIPTransportErrorListener* piErrorListener;
};

#endif  // _SIP_TRANSPORT_H_
