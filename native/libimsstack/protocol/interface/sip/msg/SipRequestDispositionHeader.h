#ifndef __SIP_REQUEST_DISPOSITION_HEADER_H__
#define __SIP_REQUEST_DISPOSITION_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipRequestDispositionHeader : public SipHeaderBase
{
public:
    /*constructor*/
    SipRequestDispositionHeader();
    SipRequestDispositionHeader(const SipRequestDispositionHeader& objHeader);
    /*destructor*/
    ~SipRequestDispositionHeader();
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
    /*virtual methods*/
    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
};
#endif  //__SIP_REQUEST_DISPOSITION_HEADER_H__
