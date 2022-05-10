#ifndef __SIP_PRIVACY_HEADER_H__
#define __SIP_PRIVACY_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipPrivacyHeader : public SipHeaderBase
{
private:
    /*this will be stored as alist in sipparmlist*/
    SipVector<SIP_CHAR*> m_objPrivacyList;

public:
    /*constructor*/
    SipPrivacyHeader();
    SipPrivacyHeader(const SipPrivacyHeader& objHeader);

    /*destructor*/
    ~SipPrivacyHeader();
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
    /*virtual methods*/
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    SIP_BOOL AddPrivacy(const SIP_CHAR* pszPrivacy);

    inline SIP_BOOL IsValidHeader() const
    {
        return (m_objPrivacyList.GetSize() > 0) ? SIP_TRUE : SIP_FALSE;
    }
};
#endif  //__SIP_PRIVACY_HEADER_H__
