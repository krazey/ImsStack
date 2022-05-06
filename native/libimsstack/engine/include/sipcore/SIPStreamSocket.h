/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_STREAM_SOCKET_H_
#define _SIP_STREAM_SOCKET_H_

#include "ITimer.h"
#include "private/SipConfig.h"
#include "SIPSocket.h"
#include "SIPMessageFraming.h"

class ISIPStreamSocketListener;

class SIPStreamSocket : public SIPSocket, public ITimerListener
{
public:
    explicit SIPStreamSocket(IN IMS_SINT32 nSlotId);
    SIPStreamSocket(IN IMS_SINT32 nSlotId, IN ISocket* piSocket_);
    virtual ~SIPStreamSocket();

private:
    SIPStreamSocket(IN CONST SIPStreamSocket& objRHS);
    SIPStreamSocket& operator=(IN CONST SIPStreamSocket& objRHS);

public:
    virtual void ApplyIpSec(IN ISocket* piAcceptedSocket = IMS_NULL);
    virtual IMS_BOOL Connect();
    virtual IMS_BOOL Create(
            IN CONST IPAddress& objIPA, IN IMS_UINT32 nPort = 0, IN IMS_BOOL bSecure = IMS_FALSE);
    virtual void GetSockName(OUT IPAddress& objIPA, OUT IMS_UINT32& nPort);
    virtual IMS_SINT32 Send(IN CONST IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen,
            IN IMS_UINT32 nPort = 0, IN CONST IPAddress& objIPA = IPAddress::NONE);
    virtual void NotifyForceClosed();

    void DisableKeepAlive();
    IMS_BOOL IsKeepAliveTimerActive() const;
    IMS_BOOL IsKeepAlivePermanent() const;
    IMS_BOOL IsSecureSocket() const;
    void ReuseSocket();
    void SetConfigForSIPKeepAlive(IN IMS_BOOL bSIPKeepAlive);
    void SetFarEnd(IN CONST IPAddress& objIPA, IN IMS_UINT32 nPort);
    void SetKeepAlivePolicy(IN IMS_SINT32 nPolicy);
    void SetListener(IN ISIPStreamSocketListener* piListener);

protected:
    // ITimerListener interface
    virtual void Timer_TimerExpired(IN ITimer* piTimer);

    virtual void Socket_OnDataReceived(IN ISocket* piSocket);
    virtual void Socket_OnSendEnabled(IN ISocket* piSocket);
    virtual void Socket_OnConnected(IN ISocket* piSocket);
    virtual void Socket_OnClosed(
            IN ISocket* piSocket, IN IMS_SINT32 nReason = ISocket::CLOSE_REASON_UNKNOWN);

private:
    IMS_RESULT StartTxTimer(IN IMS_SINT32 nDuration);
    void StopTxTimer();
    IMS_RESULT StartKeepAliveTimer(IN IMS_SINT32 nInactiveInterval);
    void StopKeepAliveTimer();
    void UpdateLastAliveTime();

private:
    enum
    {
        CONNECTION_TIMER_VALUE = 10000
    };
    enum
    {
        WOULDBLOCK_TIMER_VALUE = 33000
    };
    enum
    {
        KEEPALIVE_TIMER_VALUE = 60000
    };

    SipConfig::TcpTimerValues objTV_TCP;
    SIPMessageFraming objMFraming;

    IMS_BOOL bSecure;
    IMS_BOOL bSIPKeepAliveConfigured;

    ITimer* piTxTimer;
    IMS_UINT32 nLastAliveTime;
    ITimer* piKeepAliveTimer;
    ISIPStreamSocketListener* piListener;
};

#endif  // _SIP_STREAM_SOCKET_H_
