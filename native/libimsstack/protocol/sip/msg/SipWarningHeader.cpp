#include "msg/SipWarningHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"

#define MAX_WARN_LEN 12

/*constructor*/
SipWarningHeader::SipWarningHeader() :
        SipHeaderBase(SipHeaderBase::WARNING),
        m_nWarnCode(SIP_ZERO),
        m_pszWarnAgent(SIP_NULL),
        m_pszWarnText(SIP_NULL)
{
}

SipWarningHeader::SipWarningHeader(const SipWarningHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_nWarnCode(objHeader.m_nWarnCode),
        m_pszWarnAgent(SipPf_Strdup(objHeader.m_pszWarnAgent)),
        m_pszWarnText(SipPf_Strdup(objHeader.m_pszWarnText))
{
}

/*destructor*/
SipWarningHeader::~SipWarningHeader()
{
    if (m_pszWarnAgent != SIP_NULL)
    {
        delete[] m_pszWarnAgent;
        m_pszWarnAgent = SIP_NULL;
    }
    if (m_pszWarnText != SIP_NULL)
    {
        delete[] m_pszWarnText;
        m_pszWarnText = SIP_NULL;
    }
}

/*virtual methods*/
SIP_BOOL SipWarningHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL /*bParams*/) const
{
    if (IsValidHeader() == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "Missing warn-agent or warn-text", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    objBuffer += m_nWarnCode;
    objBuffer += SPACE;
    objBuffer += m_pszWarnAgent;
    objBuffer += SPACE;

    if (HasSpace(m_pszWarnText))
    {
        objBuffer += DQUOTE;
        objBuffer += m_pszWarnText;
        objBuffer += DQUOTE;
    }
    else
    {
        objBuffer += m_pszWarnText;
    }

    return SIP_TRUE;
}

/*Function for encoding of headers*/
SIP_BOOL SipWarningHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (IsValidHeader() == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "Missing Warn Agent or Warn Text", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR szLen[MAX_WARN_LEN];
    SipPf_Sprintf(szLen, (SIP_CHAR*)"%u", m_nWarnCode);

    SipPf_Strcpy(*ppCurrPos, szLen);
    SipEnc_UpdateCurrPos(ppCurrPos);

    SIP_ENC_SP(*ppCurrPos);

    SipPf_Strcpy(*ppCurrPos, m_pszWarnAgent);
    SipEnc_UpdateCurrPos(ppCurrPos);

    SIP_ENC_SP(*ppCurrPos);

    if (HasSpace(m_pszWarnText))
    {
        SIP_ENC_LDQUOT(*ppCurrPos);

        SipPf_Strcpy(*ppCurrPos, m_pszWarnText);
        SipEnc_UpdateCurrPos(ppCurrPos);

        SIP_ENC_RDQUOT(*ppCurrPos);
    }
    else
    {
        SipPf_Strcpy(*ppCurrPos, m_pszWarnText);
        SipEnc_UpdateCurrPos(ppCurrPos);
    }

    return SIP_TRUE;
}

/*Sets the visited nw name*/
SIP_BOOL SipWarningHeader::SetWarnAgent(const SIP_CHAR* pszWarnAgent)
{
    return SetCharVar(pszWarnAgent, m_pszWarnAgent);
}

/*Sets the visited nw name*/
SIP_BOOL SipWarningHeader::SetWarnText(const SIP_CHAR* pszWarnText)
{
    return SetCharVar(pszWarnText, m_pszWarnText);
}

/******************************************************************************
 * Function name      : SipWarningHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipWarningHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempLoc = SIP_NULL;

    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempLoc, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeHdr: Space Not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pszWarnCode = sipCreateString(pStartPt, pTempLoc);
    if (pszWarnCode == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_nWarnCode = SipPf_Atoi(pszWarnCode);
    delete[] pszWarnCode;
    if ((m_nWarnCode < MIN_WARNCODE) || (m_nWarnCode > MAX_WARNCODE))
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr:Warn code Value is not valid", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*update the start point to the start of Warn Agent*/
    pStartPt = pTempLoc + SIP_TWO;
    pTempLoc = SIP_NULL;
    /*Find the endpoint of Warn Agent*/
    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempLoc, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeHdr: Space Not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pszWarnAgent = sipCreateString(pStartPt, pTempLoc);
    if (m_pszWarnAgent == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Update the start point to the start of Warn text*/
    pStartPt = pTempLoc + SIP_TWO;
    m_pszWarnText = sipCreateString(pStartPt, pEndPt);
    if (m_pszWarnText == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SIP_BOOL SipWarningHeader::IsValidHeader() const
{
    if (((m_nWarnCode < MIN_WARNCODE) || (m_nWarnCode > MAX_WARNCODE)) ||
            (m_pszWarnAgent == SIP_NULL) || (m_pszWarnText == SIP_NULL))
    {
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipWarningHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipWarningHeader(*reinterpret_cast<SipWarningHeader*>(pHeader));
    }
    return new SipWarningHeader();
}
