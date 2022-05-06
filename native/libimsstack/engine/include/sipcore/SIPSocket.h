/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_SOCKET_H_
#define _SIP_SOCKET_H_

#include "ImsSlot.h"
#include "ISocket.h"
#include "SIPSocketAddress.h"

class INetworkConnection;
class ISIPSocketListener;
class ISIPKeepAliveListener;

class SIPSocket : public ImsSlot, public ISocketListener
{
public:
    explicit SIPSocket(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nType_ = SIPSocketAddress::SOCKET_UDP);
    virtual ~SIPSocket();

private:
    SIPSocket(IN CONST SIPSocket& objRHS);
    SIPSocket& operator=(IN CONST SIPSocket& objRHS);

public:
    virtual SIPSocket* Accept();
    virtual void ApplyIpSec(IN ISocket* piAcceptedSocket = IMS_NULL);
    virtual IMS_BOOL Connect();
    virtual IMS_BOOL Create(
            IN CONST IPAddress& objIPA, IN IMS_UINT32 nPort = 0, IN IMS_BOOL bSecure = IMS_FALSE);
    virtual IMS_BOOL Equals(IN CONST SIPSocketAddress& objSA);
    virtual IMS_BOOL Equals(IN CONST SIPSocket& objSocket);
    virtual void GetSockName(OUT IPAddress& objIPA, OUT IMS_UINT32& nPort);
    virtual IMS_SINT32 Send(IN CONST IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen,
            IN IMS_UINT32 nPort = 0, IN CONST IPAddress& objIPA = IPAddress::NONE);

    virtual void NotifyForceClosed();

    void GetPeerName(OUT IPAddress& objIPA, OUT IMS_UINT32& nPort);
    IMS_SINT32 RemoveListener(IN ISIPSocketListener* piListener_);
    void SetKeepAliveListener(IN ISIPKeepAliveListener* piKeepAliveListener);
    void SetListener(IN ISIPSocketListener* piListener_);
    /** Same as ISocket::SetOption(...) */
    void SetOption(IN IMS_SINT32 nOption, IN IMS_SINT32 nOptionValue);

    inline IMS_SINT32 GetState() const { return nState; }
    inline IMS_SINT32 GetType() const { return objSA.GetType(); }
    inline IMS_BOOL IsSocketForcinlyClosed() const { return bForcinglyClosed; }

protected:
    // ISocketListener interface
    virtual void Socket_OnDataReceived(IN ISocket* piSocket);
    virtual void Socket_OnSendEnabled(IN ISocket* piSocket);
    virtual void Socket_OnConnectionReceived(IN ISocket* piSocket);
    virtual void Socket_OnConnected(IN ISocket* piSocket);
    virtual void Socket_OnClosed(
            IN ISocket* piSocket, IN IMS_SINT32 nReason = ISocket::CLOSE_REASON_UNKNOWN);

    void ApplyIpSecInternal(IN const SocketAddress& objLocal,
            IN const SocketAddress* pRemote = IMS_NULL, IN ISocket* piAcceptedSocket = IMS_NULL);
    void CloseSocket();
    void NotifyPongReceived();
    void SetForcinglyClosed(IN IMS_BOOL bClosed);
    void SetSocketOptionForTcpMaxSeg(
            IN INetworkConnection* piConnection, IN CONST IPAddress& objLocalIP);
    void SetSocketOptions(IN CONST IPAddress& objLocalIP, IN IMS_UINT32 nLocalPort);
    void SetState(IN IMS_SINT32 nState);

    static void SetSocketOption(IN IMS_SINT32 nSlotId, IN ISocket* piSocket,
            IN CONST IPAddress& objLocalIP, IN IMS_UINT32 nLocalPort, IN IMS_SINT32 nConfigItem,
            IN IMS_SINT32 nSocketOption, IN CONST IMS_CHAR* pszOptionName);
    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    enum
    {
        TYPE_NONE = 0,
        TYPE_UDP,
        TYPE_TCP,
        TYPE_TCP_CLIENT,
        TYPE_TCP_CLIENT_OTHER
    };

    enum
    {
        STATE_CREATED = 0,
        STATE_INITIALIZED,
        STATE_CONNECTING,
        STATE_CONNECTED,
        STATE_CLOSING,
        STATE_TERMINATED
    };

    enum
    {
        ERROR_NO_ERROR = 0,
        ERROR_WOULDBLOCK_TIMEDOUT,
        ERROR_CONNECTION_TIMEDOUT,
        ERROR_SEND_FAILED,
        ERROR_CLOSED,
        ERROR_CONNECT_FAILED,
        ERROR_DATA_CONNECTION_LOST
    };

protected:
    // Index to the destination transport tuple
    // In case of UDP/TCP server socket, this information is set to the local socket information.
    // In case of TCP client socket, this information is set to the peer socket information.
    //    1) Connected by UA : Set at the calling time of Connect(...).
    //    2) Accepted by UA : Set at the calling time of Accept(...) using GetPeerName(...).
    SIPSocketAddress objSA;

    ISocket* piSocket;
    IMSList<ISIPSocketListener*> objListeners;
    ISIPKeepAliveListener* piKeepAliveListener;

private:
    IMS_SINT32 nState;
    IMS_BOOL bForcinglyClosed;
};

#endif  // _SIP_SOCKET_H_
