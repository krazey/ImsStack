#ifndef __SIP_WARNING_HEADER_H__
#define __SIP_WARNING_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipWarningHeader : public SipHeaderBase
{
private:
    /*warn-code*/
    SIP_UINT32 m_nWarnCode;

    /*Protocol Version*/
    SIP_CHAR* m_pszWarnAgent;

    /*Transport*/
    SIP_CHAR* m_pszWarnText;

public:
    /*constructor*/
    SipWarningHeader();

    SipWarningHeader(const SipWarningHeader& objHeader);

    /*destructor*/
    ~SipWarningHeader();

    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
    /*virtual methods*/
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    /*Sets the visited nw name*/
    inline SIP_VOID SetWarnCode(SIP_UINT32 nWarnCode) { m_nWarnCode = nWarnCode; }

    /*Gets the visited nw name*/
    inline SIP_UINT32 GetWarnCode() const { return m_nWarnCode; }

    /*Sets the visited nw name*/
    SIP_BOOL SetWarnAgent(const SIP_CHAR* pszWarnAgent);

    /*Gets the visited nw name*/
    inline SIP_CHAR* GetWarnAgent() const { return m_pszWarnAgent; }

    /*Sets the visited nw name*/
    SIP_BOOL SetWarnText(const SIP_CHAR* pszWarnText);

    /*Gets the visited nw name*/
    inline const SIP_CHAR* GetWarnText() const { return m_pszWarnText; }

    SIP_BOOL IsValidHeader() const;
};
#endif  //__SIP_WARNING_HEADER_H__
