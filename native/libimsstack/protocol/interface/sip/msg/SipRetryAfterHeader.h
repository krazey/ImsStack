#ifndef __SIP_RETRY_AFTER_HEADER_H__
#define __SIP_RETRY_AFTER_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipRetryAfterHeader : public SipHeaderBase
{
private:
    /*Delta seconds*/
    SIP_UINT32 m_nDeltaSec;

    /*Comment*/
    SIP_CHAR* m_pszComment;

public:
    /*constructor*/
    SipRetryAfterHeader();

    SipRetryAfterHeader(const SipRetryAfterHeader& objHeader);

    /*destructor*/
    ~SipRetryAfterHeader();

    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    /*virtual methods*/
    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    /*Sets */
    inline SIP_VOID SetDeltaSec(SIP_UINT32 nDeltaSec) { m_nDeltaSec = nDeltaSec; }
    /*Gets */
    inline SIP_UINT32 GetDeltaSec() const { return m_nDeltaSec; }

    /*Sets */
    SIP_BOOL SetComment(const SIP_CHAR* pszComment);
    /*Gets */
    inline const SIP_CHAR* GetComment() const { return m_pszComment; }

    inline SIP_BOOL IsValidHeader() const { return SIP_TRUE; }
};
#endif  //__SIP_RETRY_AFTER_HEADER_H__
