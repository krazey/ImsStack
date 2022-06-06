#ifndef __SIP_IDENTITY_HEADER_H__
#define __SIP_IDENTITY_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipIdentityHeader : public SipHeaderBase
{
public:
    /*constructor*/
    SipIdentityHeader();
    SipIdentityHeader(const SipIdentityHeader& objHeader);

    /*destructor*/
    ~SipIdentityHeader();

    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    /*virtual methods*/
    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    SIP_BOOL SetSignedIdentityDigest(const SIP_CHAR* pszSignedIdentiDigest);
    SIP_BOOL SetInfo(const SIP_CHAR* pszInfo);

    inline const SIP_CHAR* GetSignedIdentityDigest() const { return m_pSignedIdentityDigest; }
    inline const SIP_CHAR* GetInfo() const { return m_pInfo; }

    inline SIP_BOOL IsValidHeader() const
    {
        return ((m_pSignedIdentityDigest == SIP_NULL) || (m_pInfo == SIP_NULL)) ? SIP_FALSE
                                                                                : SIP_TRUE;
    }

private:
    SIP_CHAR* m_pSignedIdentityDigest;
    SIP_CHAR* m_pInfo;
};
#endif  //__SIP_IDENTITY_HEADER_H__
