#ifndef __SIP_P_PREFEERED_SERVICE_HEADER_H__
#define __SIP_P_PREFEERED_SERVICE_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipPPreferredServiceHeader : public SipHeaderBase
{
public:
    /*constructor*/
    SipPPreferredServiceHeader();
    SipPPreferredServiceHeader(const SipPPreferredServiceHeader& objHeader);

    /*destructor*/
    ~SipPPreferredServiceHeader();
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    /*virtual methods*/
    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
};
#endif  //__SIP_P_PREFEERED_SERVICE_HEADER_H__
