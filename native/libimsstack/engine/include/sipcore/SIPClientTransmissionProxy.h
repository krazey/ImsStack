/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20160215  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SIP_CLIENT_TRANSMISSION_PROXY_H_
#define _SIP_CLIENT_TRANSMISSION_PROXY_H_

// TCP active connection MUST be created at the start time of raw SIP message transmission.
#include "EngineActivity.h"
#include "ISIPSocketListener.h"

class ISIPClientTransmissionListener;
class SipTimerValues;

class SIPClientTransmissionProxy : public EngineActivity, public ISIPSocketListener
{
public:
    SIPClientTransmissionProxy();
    virtual ~SIPClientTransmissionProxy();

private:
    SIPClientTransmissionProxy(IN CONST SIPClientTransmissionProxy& objRHS);
    SIPClientTransmissionProxy& operator=(IN CONST SIPClientTransmissionProxy& objRHS);

public:
    // EngineActivity class
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMSG);

    void Abort();
    IMS_RESULT Send();
    IMS_RESULT SendWithCredentials();
    void SetListener(IN ISIPClientTransmissionListener* piListener);
    void SetTimerValues(IN SipTimerValues* pTV);
    void SetTransactionState(IN SIPClientTransactionState* pCTState);

private:
    // ISIPSocketListener class
    virtual void Socket_NotifyError(IN SIPSocket* pSocket, IN IMS_SINT32 nErrorCode);
    virtual void Socket_SendEnabled(IN SIPSocket* pSocket);

    void DestroyStreamSocket();
    IMS_BOOL IsUDPFallbackRequired() const;
    IMS_BOOL IsUDPFallbackSupported() const;
    void NotifyTransportError(IN IMS_SINT32 nErrorCode);
    IMS_RESULT PrepareStreamSocket();
    IMS_RESULT SendMessage();
    void SendPendingMessage();

public:
    enum
    {
        RESULT_NOK = (-1),
        RESULT_OK = 0,
        RESULT_PENDING = 1
    };

private:
    enum
    {
        AMSG_SEND_MESSAGE = AMSG_USER,
        AMSG_NOTIFY_TRANSPORT_ERROR
    };

    SipTimerValues* pTV;
    SIPClientTransactionState* pCTState;
    ISIPClientTransmissionListener* piListener;

    IMS_BOOL bIsResubmittedRequest;
    // TCP socket
    SIPSocket* pSocket;
};

#endif  // _SIP_CLIENT_TRANSMISSION_PROXY_H_
