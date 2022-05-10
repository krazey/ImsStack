#ifndef __SIP_FEATURE_CAPS_HEADER_H__
#define __SIP_FEATURE_CAPS_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipFeatureCapsHeader : public SipHeaderBase
{
public:
    /*constructor*/
    SipFeatureCapsHeader();

    /*Copy constructor*/
    SipFeatureCapsHeader(const SipFeatureCapsHeader& objHeader);

    /*destructor*/
    ~SipFeatureCapsHeader();
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
};
#endif  //__SIP_FEATURE_CAPS_HEADER_H__
