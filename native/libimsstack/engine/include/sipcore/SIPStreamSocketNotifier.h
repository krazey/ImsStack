/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_STREAM_SOCKET_NOTIFIER_H_
#define _SIP_STREAM_SOCKET_NOTIFIER_H_

#include "private/SipConfig.h"
#include "SIPSocket.h"

class ISIPStreamSocketListener;



class SIPStreamSocketNotifier
    : public SIPSocket
{
public:
    explicit SIPStreamSocketNotifier(IN IMS_SINT32 nSlotId);
    virtual ~SIPStreamSocketNotifier();

public:
    virtual SIPSocket* Accept();
    virtual IMS_BOOL Create(IN CONST IPAddress &objIPA, IN IMS_UINT32 nPort = 0,
            IN IMS_BOOL bSecure = IMS_FALSE);
    void SetListener(IN ISIPStreamSocketListener *piListener);

public:
    virtual void Socket_OnConnectionReceived(IN ISocket *piSocket);
    virtual void Socket_OnClosed(IN ISocket *piSocket,
            IN IMS_SINT32 nReason = ISocket::CLOSE_REASON_UNKNOWN);

private:
    ISIPStreamSocketListener *piListener;
};

#endif // _SIP_STREAM_SOCKET_NOTIFIER_H_
