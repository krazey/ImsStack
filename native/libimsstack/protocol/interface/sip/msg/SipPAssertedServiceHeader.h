#ifndef __SIP_P_ASSERTED_SERVICE_HEADER_H__
#define __SIP_P_ASSERTED_SERVICE_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipPAssertedServiceHeader : public SipHeaderBase
{
public:
    /*constructor*/
    SipPAssertedServiceHeader();
    SipPAssertedServiceHeader(const SipPAssertedServiceHeader& objHeader);

    /*destructor*/
    ~SipPAssertedServiceHeader();
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    /*virtual methods*/
    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
};
#endif  //__SIP_P_ASSERTED_SERVICE_HEADER_H__
