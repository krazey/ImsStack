#ifndef __SIP_INFO_BASE_H__
#define __SIP_INFO_BASE_H__

#include "msg/SipHeaderBase.h"

class SipInfoBase : public SipHeaderBase
{
public:
    /*constructor*/
    SipInfoBase(SIP_INT32 eHdrType);
    SipInfoBase(const SipInfoBase& objHeader);

    /*destructor*/
    ~SipInfoBase();

    /*virtual methods*/
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
};
#endif  //__SIP_INFO_BASE_H__
