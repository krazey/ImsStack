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
#include "ISIPKeepAliveHelper.h"
#include "ISIPKeepAliveListener.h"
#include "SIPSocketAddress.h"



class SIPKeepAliveHelper
    : public ImsSlot
    , public ISIPKeepAliveHelper
    , public ISIPKeepAliveListener
{
public:
    explicit SIPKeepAliveHelper(IN IMS_SINT32 nSlotId);
    virtual ~SIPKeepAliveHelper();

private:
    // ISIPObject class
    virtual void Destroy();

    // ISIPKeepAliveHelper class
    virtual IMS_RESULT SendPacket(IN CONST ByteArray &objPacket);
    virtual void SetListener(IN ISIPKeepAliveHelperListener *piListener);
    virtual void SetTransportTupleD(IN CONST IPAddress &objIP, IN IMS_SINT32 nPort);
    virtual void SetTransportTupleS(IN CONST IPAddress &objIP, IN IMS_SINT32 nPort,
            IN IMS_SINT32 nProtocol = SIP::TRANSPORT_UDP);

    // ISIPKeepAliveListener class
    virtual void KeepAlive_PongReceived();

private:
    ISIPKeepAliveHelperListener *piListener;
    SIPSocketAddress objSA_NearEnd;
    SIPSocketAddress objSA_FarEnd;
};

#endif // _SIP_KEEP_ALIVE_HELPER_H_
