#ifndef __SIP_AUTH_BASE_H__
#define __SIP_AUTH_BASE_H__

#include "msg/SipHeaderBase.h"
#include "msg/sip_comdef.h"

class SipAuthBase : public SipHeaderBase
{
protected:
    /*Credential*/
    SipVector<SipNameValue*> m_objAuthList;

    SIP_BOOL EncodeAuthList(SIP_CHAR** ppCurrPos, SIP_CHAR cDelimiter);

public:
    /*constructor*/
    SipAuthBase(SIP_INT32 eHdrType);
    SipAuthBase(const SipAuthBase& objHeader);

    /*destructor*/
    ~SipAuthBase();

    /*virtual methods*/
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    SIP_BOOL SetParams(const SIP_CHAR* pszName, const SIP_CHAR* pszVal, SIP_BOOL bIsFeatureParam);

    SIP_BOOL FindElement(const SIP_CHAR* pszName, SipNameValue*& pNmvl, SIP_UINT32& nPos);

    SIP_CHAR* GetAuthValue(const SIP_CHAR* pszName);

    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
};
#endif  //__SIP_AUTH_BASE_H__
