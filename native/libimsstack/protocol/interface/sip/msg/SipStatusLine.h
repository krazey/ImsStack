#ifndef __SIP_STATUS_LINE_H__
#define __SIP_STATUS_LINE_H__

#include "SipRefBase.h"
#include "msg/sip_comdef.h"

class SipStatusLine : public SipRefBase
{
private:
    SIP_CHAR* m_pszSipVersion;
    SIP_CHAR* m_pszStatusCode;
    SIP_CHAR* m_pszRsnPhrase;

public:
    /*constructor*/
    SipStatusLine() :
            m_pszSipVersion(SIP_NULL),
            m_pszStatusCode(SIP_NULL),
            m_pszRsnPhrase(SIP_NULL)
    {
    }

    SipStatusLine(const SIP_CHAR* pszStatusCode, const SIP_CHAR* pszRsnPhrase);

    SipStatusLine(const SIP_CHAR* pszSipVersion, const SIP_CHAR* pszStatusCode,
            const SIP_CHAR* pszRsnPhrase);

    SipStatusLine(const SipStatusLine& objHeader);
    /*destructor*/
    ~SipStatusLine();

    /*Function for encoding*/
    SIP_BOOL EncodeStatusLine(SIP_CHAR** ppCurrPos);

    /*Function for decoding*/
    SIP_BOOL DecodeStatusLine(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    /*Set Methods*/
    SIP_BOOL SetStatusCode(const SIP_CHAR* pszStatusCode);

    SIP_BOOL SetSipVersion(const SIP_CHAR* pszVer);

    SIP_BOOL SetRsnPhrase(const SIP_CHAR* pszRsnPhrase);

    /*Get methods*/
    inline const SIP_CHAR* GetStatusCode() const { return m_pszStatusCode; }

    SIP_BOOL GetStatusCode(SIP_INT16* pnStatusCode) const;

    SIP_UINT16 GetStatusCodeAsInt() const;

    inline const SIP_CHAR* GetSipVersion() const { return m_pszSipVersion; }

    inline const SIP_CHAR* GetRsnPhrase() const { return m_pszRsnPhrase; }
};
#endif  //__SIP_STATUS_LINE_H__
