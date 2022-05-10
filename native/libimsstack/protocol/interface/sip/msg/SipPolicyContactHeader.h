#ifndef __SIP_POLICY_CONTACT_HEADER_H__
#define __SIP_POLICY_CONTACT_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipPolicyContactHeader : public SipNameAddrHeader
{
public:
    /*constructor*/
    SipPolicyContactHeader();
    SipPolicyContactHeader(const SipPolicyContactHeader& objHeader);

    /*destructor*/
    ~SipPolicyContactHeader();

    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
    /*virtual methods*/
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
};

#endif  //__SIP_POLICY_CONTACT_HEADER_H__
