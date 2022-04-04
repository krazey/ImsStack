#ifndef __ISIP_DEFNETWORKUTIL_H__
#define __ISIP_DEFNETWORKUTIL_H__

#include "sip_pf_datatypes.h"
#include "ISipNetworkUtil.h"

class SipDefNetworkUtil: public ISipNetworkUtil
{
    public:
        SipDefNetworkUtil();
        ~SipDefNetworkUtil();
    public:
        SIP_BOOL SendToNetwork(SipTransportBuffer* pTranspSipBuffer,
                SipTransportParameter* pFinalTranspParam, ISipUserData* pUserData);

        SIP_BOOL CheckTCPConnection(SipTransportParameter* pTransportParam,
                ISipUserData* pUserData);

        SIP_BOOL AbortTransmission(SipTransportParameter* pTranspParam, ISipUserData* pUserData);
};

#endif // __ISIP_DEFNETWORKUTIL_H__
