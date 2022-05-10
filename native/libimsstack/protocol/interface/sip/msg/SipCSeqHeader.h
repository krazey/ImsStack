#ifndef __SIP_CSEQ_HEADER_H__
#define __SIP_CSEQ_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipCSeqHeader : public SipHeaderBase
{
private:
    SIP_CHAR* m_pszMethod;
    SIP_UINT32 m_nSeq;

public:
    /*constructor*/
    SipCSeqHeader();
    SipCSeqHeader(const SipCSeqHeader& objHeader);

    /*destructor*/
    ~SipCSeqHeader();
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
    /*virtual methods*/
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    /*set methods*/
    SIP_BOOL SetMethod(const SIP_CHAR* pszMethod);

    /*set Seq*/
    inline SIP_VOID SetSeq(SIP_UINT32 nSeq) { m_nSeq = nSeq; }
    /*Get methods*/
    inline const SIP_CHAR* GetMethod() const { return m_pszMethod; }

    inline SIP_UINT32 GetCSeq() const { return m_nSeq; }

    SIP_BOOL IsValidHeader() const;
};
#endif  //__SIP_CSEQ_HEADER_H__
