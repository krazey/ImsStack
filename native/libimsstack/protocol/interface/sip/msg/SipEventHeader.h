#ifndef __SIP_EVENT_HEADER_H__
#define __SIP_EVENT_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipEventHeader : public SipHeaderBase
{
private:
    /*This is a part of header value but kept as prm*/
    SipParameterList* m_pEventTemplateList;

public:
    /*constructor*/
    SipEventHeader();
    SipEventHeader(const SipEventHeader& objHeader);
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
    /*destructor*/
    ~SipEventHeader();

    /*virtual methods*/
    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
};
#endif  //__SIP_EVENT_HEADER_H__
