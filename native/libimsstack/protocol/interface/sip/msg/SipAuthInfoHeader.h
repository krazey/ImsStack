#ifndef __SIP_AUTH_INFO_HEADER_H__
#define __SIP_AUTH_INFO_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipAuthInfoHeader : public SipHeaderBase
{
private:
    /*Media Disp*/
    SipNameValue* m_pAiInfo;

public:
    /*constructor*/
    SipAuthInfoHeader();
    SipAuthInfoHeader(const SipAuthInfoHeader& objHeader);

    /*destructor*/
    ~SipAuthInfoHeader();
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
    /*virtual methods*/
    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    const SIP_CHAR* GetAiInfoVal(SIP_UINT32 nPos = SIP_ZERO);
    inline SIP_BOOL IsValidHeader() const { return (m_pAiInfo != SIP_NULL) ? SIP_TRUE : SIP_FALSE; }
};
#endif  //__SIP_AUTH_INFO_HEADER_H__
