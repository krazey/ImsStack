#include "msg/SipStatusLine.h"
#include "sip_pf_datatypes.h"
#include "SipTrace.h"
#include "platform/sip_pf_memory.h"
#include "sip_debug.h"
#include "sip_error.h"
#include "platform/sip_pf_string.h"
#include "msg/SipMessage.h"
#include "msg/sip_msgutil.h"

SipStatusLine::SipStatusLine(const SIP_CHAR* pszStatusCode, const SIP_CHAR* pszRsnPhrase)
    : m_pszSipVersion(SipPf_Strdup(SIP_SIPVER))
    , m_pszStatusCode(SipPf_Strdup(pszStatusCode))
    , m_pszRsnPhrase(SipPf_Strdup(pszRsnPhrase))
{
}

SipStatusLine::SipStatusLine(const SIP_CHAR* pszSipVersion, const SIP_CHAR* pszStatusCode,
        const SIP_CHAR* pszRsnPhrase)
    : m_pszSipVersion(SipPf_Strdup(pszSipVersion))
    , m_pszStatusCode(SipPf_Strdup(pszStatusCode))
    , m_pszRsnPhrase(SipPf_Strdup(pszRsnPhrase))
{
}

SipStatusLine::SipStatusLine(const SipStatusLine& objHeader)
    : m_pszSipVersion(SipPf_Strdup(objHeader.m_pszSipVersion))
    , m_pszStatusCode(SipPf_Strdup(objHeader.m_pszStatusCode))
    , m_pszRsnPhrase(SipPf_Strdup(objHeader.m_pszRsnPhrase))
{
}

SipStatusLine::~SipStatusLine()
{
    if (m_pszSipVersion != SIP_NULL)
    {
        delete[] m_pszSipVersion;
    }
    if (m_pszStatusCode != SIP_NULL)
    {
        delete[] m_pszStatusCode;
    }
    if (m_pszRsnPhrase != SIP_NULL)
    {
        delete[] m_pszRsnPhrase;
    }

}

SIP_BOOL SipStatusLine::EncodeStatusLine(SIP_CHAR** ppCurrPos)
{
    /*check for existence of version, status code and reason phrase*/
    if (m_pszStatusCode == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "Status code missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    if (m_pszRsnPhrase == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "Reason Phrase missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    if (m_pszSipVersion == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "Sip Version missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /* Encode Sip Version*/
    SipPf_Strcpy(*ppCurrPos, m_pszSipVersion);
    /*Update the Msg Buffer's current position*/
    SipEnc_UpdateCurrPos(ppCurrPos);

    /* Put a space */
    SIP_ENC_SP(*ppCurrPos);

    /*Encode Status Code*/
    SipPf_Strcpy(*ppCurrPos, m_pszStatusCode);
    /*Update the Msg Buffer's current position*/
    SipEnc_UpdateCurrPos(ppCurrPos);

    /* Put a space */
    SIP_ENC_SP(*ppCurrPos);

    SipPf_Strcpy(*ppCurrPos, m_pszRsnPhrase);
    /*Update the Msg Buffer's current position*/
    SipEnc_UpdateCurrPos(ppCurrPos);

    return SIP_TRUE;
}

SIP_BOOL SipStatusLine::SetStatusCode(const SIP_CHAR* pszStatusCode)
{
    return SetCharVar(pszStatusCode, m_pszStatusCode);
}

SIP_BOOL SipStatusLine::SetSipVersion(const SIP_CHAR* pszVer)
{
    return SetCharVar(pszVer, m_pszSipVersion);
}

SIP_BOOL SipStatusLine::SetRsnPhrase(const SIP_CHAR* pszRsnPhrase)
{
    return SetCharVar(pszRsnPhrase, m_pszRsnPhrase);
}

SIP_BOOL SipStatusLine::GetStatusCode(SIP_INT16* pnStatusCode) const
{
    if (m_pszStatusCode != SIP_NULL)
    {
        *pnStatusCode = SipPf_Atoi(m_pszStatusCode);
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SIP_UINT16 SipStatusLine::GetStatusCodeAsInt() const
{
    return (m_pszStatusCode != SIP_NULL) ? SipPf_Atoi(m_pszStatusCode) : SIP_SC_INVALID;
}

/******************************************************************************
 * Function name      : SipStatusLine::DecodeStatusLine
 *
 * Description     :
 *
 * Preconditions          SIP_CHAR   * pStartPt,
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipStatusLine::DecodeStatusLine(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempLoc = SIP_NULL;

    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempLoc, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipStatusLine::DecodeStatusLine: Space Not Found",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pszSipVersion = sipCreateString(pStartPt, pTempLoc);
    if (m_pszSipVersion == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipStatusLine::DecodeStatusLine: Memory Allocation Failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*update the start point to the start of Status Code*/
    pStartPt = pTempLoc + SIP_TWO;
    pTempLoc = SIP_NULL;
    /*Find the endpoint of status code*/
    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempLoc, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipStatusLine::DecodeStatusLine: Space Not Found",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pszStatusCode = sipCreateString(pStartPt, pTempLoc);
    if (m_pszStatusCode == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipStatusLine::DecodeStatusLine: Memory Allocation Failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Update the start point to the start of reason phrase*/
    pStartPt =  pTempLoc + SIP_TWO;
    m_pszRsnPhrase = sipCreateString(pStartPt, pEndPt);
    if (m_pszRsnPhrase == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipStatusLine::DecodeStatusLine: No Reason phrase present in response line",
                SIP_ZERO, SIP_ZERO);
    }

    return SIP_TRUE;
}
