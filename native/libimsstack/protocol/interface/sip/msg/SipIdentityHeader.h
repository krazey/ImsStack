#ifndef __SIP_IDENTITY_HEADER_H__
#define __SIP_IDENTITY_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipIdentityHeader : public SipHeaderBase
{
public:
    /*constructor*/
    SipIdentityHeader();
    SipIdentityHeader(const SipIdentityHeader& objHeader);

    /*destructor*/
    ~SipIdentityHeader();

    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    /*virtual methods*/
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
};
#endif  //__SIP_IDENTITY_HEADER_H__
