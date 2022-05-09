#ifndef __SIP_P_VISITED_NETWORK_ID_HEADER_H__
#define __SIP_P_VISITED_NETWORK_ID_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipPVisitedNetworkIdHeader : public SipHeaderBase
{
public:
    /*constructor*/
    SipPVisitedNetworkIdHeader();

    /*Copy constructor*/
    SipPVisitedNetworkIdHeader(const SipPVisitedNetworkIdHeader& objHeader);

    /*destructor*/
    ~SipPVisitedNetworkIdHeader();

    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    /*virtual methods*/
    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
};
#endif  //__SIP_P_VISITED_NETWORK_ID_HEADER_H__
