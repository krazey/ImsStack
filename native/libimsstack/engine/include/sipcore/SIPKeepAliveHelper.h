/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110528  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SIP_KEEP_ALIVE_HELPER_H_
#define _SIP_KEEP_ALIVE_HELPER_H_

#include "ImsSlot.h"
#include "ISipKeepAliveHelper.h"
#include "ISIPKeepAliveListener.h"
#include "SIPSocketAddress.h"

class SIPKeepAliveHelper : public ImsSlot, public ISipKeepAliveHelper, public ISIPKeepAliveListener
{
public:
    explicit SIPKeepAliveHelper(IN IMS_SINT32 nSlotId);
    virtual ~SIPKeepAliveHelper();

private:
    // ISipObject class
    virtual void Destroy();

    // ISipKeepAliveHelper class
    virtual IMS_RESULT SendPacket(IN CONST ByteArray& objPacket);
    virtual void SetListener(IN ISipKeepAliveHelperListener* piListener);
    virtual void SetTransportTupleD(IN CONST IPAddress& objIP, IN IMS_SINT32 nPort);
    virtual void SetTransportTupleS(IN CONST IPAddress& objIP, IN IMS_SINT32 nPort,
            IN IMS_SINT32 nProtocol = Sip::TRANSPORT_UDP);

    // ISIPKeepAliveListener class
    virtual void KeepAlive_PongReceived();

private:
    ISipKeepAliveHelperListener* piListener;
    SIPSocketAddress objSA_NearEnd;
    SIPSocketAddress objSA_FarEnd;
};

#endif  // _SIP_KEEP_ALIVE_HELPER_H_
