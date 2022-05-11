#ifndef __SIP_RACK_HEADER_H__
#define __SIP_RACK_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipRAcKHeader : public SipHeaderBase
{
private:
    /*Response Num*/
    SIP_UINT32 m_nResponseNum;

    /*CSeq Num*/
    SIP_UINT32 m_nCSeqNum;

    /*Method*/
    SIP_CHAR* m_pszMethod;

public:
    /*constructor*/
    SipRAcKHeader();
    SipRAcKHeader(const SipRAcKHeader& objHeader);
    /*destructor*/
    ~SipRAcKHeader();
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    /*set methods*/
    SIP_BOOL SetMethod(const SIP_CHAR* pszMethod);
    inline SIP_VOID SetResponseNum(SIP_UINT32 nResponseNum) { m_nResponseNum = nResponseNum; }

    inline SIP_VOID SetCSeqNum(SIP_UINT32 nCSeqNum) { m_nCSeqNum = nCSeqNum; }
    /*Get methods*/

    inline const SIP_CHAR* GetMethod() const { return m_pszMethod; }

    inline SIP_UINT32 GetResponseNum() const { return m_nResponseNum; }

    inline SIP_UINT32 GetCSeqNum() const { return m_nCSeqNum; }
    inline SIP_BOOL IsValidHeader() const
    {
        return (m_pszMethod == SIP_NULL) ? SIP_FALSE : SIP_TRUE;
    }
};

#endif  //__SIP_RACK_HEADER_H__
