#ifndef __SIP_USER_AGENT_HEADER_H__
#define __SIP_USER_AGENT_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipUserAgentHeader : public SipHeaderBase
{
private:
    SipVector<SIP_CHAR*> m_objProductList;

public:
    /*constructor*/
    SipUserAgentHeader(SIP_INT32 eHdrType);
    SipUserAgentHeader(const SipUserAgentHeader& objHeader);

    /*destructor*/
    virtual ~SipUserAgentHeader();
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    /*virtual methods*/
    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    inline SIP_BOOL IsValidHeader() const
    {
        return (m_objProductList.IsEmpty() == SIP_FALSE) ? SIP_TRUE : SIP_FALSE;
    }
};
#endif  //__SIP_USER_AGENT_HEADER_H__
