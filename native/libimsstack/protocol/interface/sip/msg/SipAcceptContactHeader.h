#ifndef __SIP_ACCEPT_CONTACT_HEADER_H__
#define __SIP_ACCEPT_CONTACT_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipAcceptContactHeader : public SipHeaderBase
{
public:
    /*constructor*/
    SipAcceptContactHeader();

    SipAcceptContactHeader(const SipAcceptContactHeader& objHeader);

    /*destructor*/
    ~SipAcceptContactHeader();
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    /*virtual methods*/
    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    SIP_BOOL SetValue(const SIP_CHAR* pszContact);
};
#endif  //__SIP_ACCEPT_CONTACT_HEADER_H__
