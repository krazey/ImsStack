#ifndef __SIP_TO_HEADER_H__
#define __SIP_TO_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipToHeader : public SipNameAddrHeader
{
public:
    /*constructor*/
    SipToHeader();

    /*destructor*/
    ~SipToHeader();
    SipToHeader(const SipToHeader& objHeader);

    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    SIP_CHAR* GetTag();

    SIP_BOOL SetTag(SIP_CHAR* pszToTag);
};
#endif  //__SIP_TO_HEADER_H__
