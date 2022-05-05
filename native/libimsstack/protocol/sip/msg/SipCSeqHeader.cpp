#include "msg/SipCSeqHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "msg/sip_msgutil.h"

/****************************************************************************
  Macro Definitions
 *****************************************************************************/
#define MAX_CSEQ_LEN 12
/****************************************************************************
  Class Member Function Implementations
 *****************************************************************************/

/******************************************************************************
 * Function name  : SipCSeqHeader::SipCSeqHeader
 *
 * Description   :
 *
 * Preconditions  :
 *
 * Side Effects  : none
 *****************************************************************************/
SipCSeqHeader::SipCSeqHeader()
    : SipHeaderBase(SipHeaderBase::CSEQ)
    , m_pszMethod(SIP_NULL)
    , m_nSeq(SIP_ZERO)
{
}

/******************************************************************************
 * Function name  : SipCSeqHeader::SipCSeqHeader
 *
 * Description   :
 *
 * Preconditions  :
 *
 * Side Effects  : none
 *****************************************************************************/
SipCSeqHeader::SipCSeqHeader(const SipCSeqHeader& objHeader)
    : SipHeaderBase(objHeader)
    , m_pszMethod(SipPf_Strdup(objHeader.m_pszMethod))
    , m_nSeq(objHeader.m_nSeq)
{
}
/******************************************************************************
 * Function name  : SipCSeqHeader::~SipCSeqHeader
 *
 * Description   :
 *
 * Preconditions  :
 *
 * Side Effects  : none
 *****************************************************************************/
SipCSeqHeader::~SipCSeqHeader()
{
    if (m_pszMethod != SIP_NULL)
    {
        delete[] m_pszMethod;
    }
}

/******************************************************************************
 * Function name  : SipCSeqHeader::EncodeHdr
 *
 * Description   :
 *
 * Preconditions  :
 *
 * Side Effects  : none
 *****************************************************************************/
SIP_BOOL SipCSeqHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (m_pszMethod == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing CSeq Method", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR szBuf[MAX_CSEQ_LEN];
    SipPf_Sprintf(szBuf, (SIP_CHAR*)"%u", m_nSeq);

    SipPf_Strcpy(*ppCurrPos, szBuf);
    SipEnc_UpdateCurrPos(ppCurrPos);

    SIP_ENC_SP(*ppCurrPos);

    SipPf_Strcpy(*ppCurrPos, m_pszMethod);
    SipEnc_UpdateCurrPos(ppCurrPos);

    return SIP_TRUE;
}

/******************************************************************************
 * Function name  : SipCSeqHeader::SetMethod
 *
 * Description   :
 *
 * Preconditions  :
 *
 * Side Effects  : none
 *****************************************************************************/
SIP_BOOL SipCSeqHeader::SetMethod(const SIP_CHAR* pszMethod)
{
    return SetCharVar(pszMethod, m_pszMethod);
}

/******************************************************************************
 * Function name      : SipCSeqHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipCSeqHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPre = SIP_NULL;

    if (sipFindLWS(pStartPt, pEndPt, &pTempPre) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "LWS missing in Cseq", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pszSeq = sipCreateString(pStartPt, pTempPre);
    if (pszSeq == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_nSeq = SipPf_Atoi_Unsigned(pszSeq);
    if ((m_nSeq > MAX_CSEQ) || (m_nSeq == SIP_ZERO))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid CSeq Value", SIP_ZERO, SIP_ZERO);
        delete[] pszSeq;
        return SIP_FALSE;
    }

    delete[] pszSeq;

    pTempPre = pTempPre + SIP_ONE;
    pStartPt = sipSkipFwLWS(pTempPre, pEndPt);

    m_pszMethod = sipCreateString(pStartPt, pEndPt);
    if (m_pszMethod == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SIP_BOOL SipCSeqHeader::IsValidHeader() const
{
    if ((m_pszMethod == SIP_NULL) ||
        ((m_nSeq > MAX_CSEQ) || (m_nSeq == SIP_ZERO)))
    {
        return SIP_FALSE;
    }
    return SIP_TRUE;
}

SipHeaderBase* SipCSeqHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipCSeqHeader(*reinterpret_cast<SipCSeqHeader*>(pHeader));
    }
    return new SipCSeqHeader();
}
