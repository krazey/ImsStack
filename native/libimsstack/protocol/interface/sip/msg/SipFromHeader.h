#ifndef __SIP_FROM_HEADER_H__
#define __SIP_FROM_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipFromHeader : public SipNameAddrHeader
{
public:
    /*constructor*/
    SipFromHeader();

    SipFromHeader(SIP_CHAR* pszDispName);
    SipFromHeader(const SipFromHeader& objHeader);
    /*destructor*/
    ~SipFromHeader();
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    SIP_CHAR* GetTag();

    SIP_BOOL SetTag(SIP_CHAR* pszFromTag);
};
#endif  //__SIP_FROM_HEADER_H__
