#ifndef __ISIPNETWORKUTIL_H__
#define __ISIPNETWORKUTIL_H__

#include "sip_pf_datatypes.h"
#include "ISipUserData.h"
#include "transport/SipTransportBuffer.h"
#include "transport/SipTransportParameter.h"

class ISipNetworkUtil
{
    public:

        ISipNetworkUtil(){};
        virtual ~ISipNetworkUtil(){};

        virtual SIP_BOOL SendToNetwork(SipTransportBuffer* pTranspSipBuffer,
                SipTransportParameter* pFinalTranspParam, ISipUserData* pUserData) = 0;

        virtual SIP_BOOL CheckTCPConnection(SipTransportParameter* pTransportParam,
                ISipUserData* pUserData) = 0;

        virtual SIP_BOOL AbortTransmission(SipTransportParameter* pTranspParam,
                ISipUserData* pUserData) = 0;
};

#endif // __ISIPNETWORKUTIL_H__
