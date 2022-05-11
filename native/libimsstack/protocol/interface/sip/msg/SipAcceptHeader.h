#ifndef __SIP_ACCEPT_HEADER_H__
#define __SIP_ACCEPT_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipAcceptHeader : public SipHeaderBase
{
private:
    /*Media range*/
    SIP_CHAR* m_pszMType;
    SIP_CHAR* m_pszMSubType;

public:
    /*constructor*/
    SipAcceptHeader();

    /*Copy constructor*/
    SipAcceptHeader(const SipAcceptHeader& objHeader);

    /*destructor*/
    ~SipAcceptHeader();
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    /*virtual methods*/
    /*Function for encoding of headers*/
    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
    SIP_BOOL IsValidHeader() const;
    /*set methods*/
    SIP_BOOL SetMediaType(const SIP_CHAR* pszMType);

    SIP_BOOL SetMediaSubType(const SIP_CHAR* pszMSubType);

    /*Get methods*/
    inline const SIP_CHAR* GetMType() const { return m_pszMType; }

    inline const SIP_CHAR* GetMSubType() const { return m_pszMSubType; }
};
#endif  //__SIP_ACCEPT_HEADER_H__
