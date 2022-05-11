#ifndef __SIP_BAD_HEADER_H__
#define __SIP_BAD_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipBadHeader : public SipHeaderBase
{
private:
    SIP_CHAR* m_pszHdrName;

public:
    /*constructor*/
    SipBadHeader();

    /*destructor*/
    ~SipBadHeader();

    SipBadHeader(const SipBadHeader& objHeader);

    /*virtual methods*/
    inline SIP_BOOL Encode(AStringBuffer& /*objBuffer*/, SIP_BOOL /*bParams*/) const override
    {
        return SIP_TRUE;
    }

    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    /*set methods*/
    SIP_BOOL SetHeaderName(const SIP_CHAR* pszHdrName);

    /*Get methods*/
    inline const SIP_CHAR* GetHeaderName() const { return m_pszHdrName; }

    inline SIP_BOOL IsValidHeader() const { return SIP_TRUE; }
};
#endif  //__SIP_BAD_HEADER_H__
