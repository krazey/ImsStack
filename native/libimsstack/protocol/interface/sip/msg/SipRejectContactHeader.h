#ifndef __SIP_REJECT_CONTACT_HEADER_H__
#define __SIP_REJECT_CONTACT_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipRejectContactHeader : public SipHeaderBase
{
public:
    /*constructor*/
    SipRejectContactHeader();

    SipRejectContactHeader(const SipRejectContactHeader& objHeader);

    /*destructor*/
    ~SipRejectContactHeader();

    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    /*virtual methods*/
    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    SIP_BOOL SetValue(const SIP_CHAR* pszContact);
};
#endif  //__SIP_REJECT_CONTACT_HEADER_H__
