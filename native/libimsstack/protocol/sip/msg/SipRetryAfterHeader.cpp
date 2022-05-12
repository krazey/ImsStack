#include "msg/SipRetryAfterHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"

#define MAX_RETRY_AFTER_LEN 12

/*constructor*/
SipRetryAfterHeader::SipRetryAfterHeader() :
        SipHeaderBase(SipHeaderBase::RETRY_AFTER_SEC),
        m_nDeltaSec(SIP_ZERO),
        m_pszComment(SIP_NULL)
{
}

/*Copy constructor*/
SipRetryAfterHeader::SipRetryAfterHeader(const SipRetryAfterHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_nDeltaSec(objHeader.m_nDeltaSec),
        m_pszComment(SipPf_Strdup(objHeader.m_pszComment))
{
}

/*destructor*/
SipRetryAfterHeader::~SipRetryAfterHeader()
{
    if (m_pszComment != SIP_NULL)
    {
        delete[] m_pszComment;
    }
}

/*virtual methods*/
SIP_BOOL SipRetryAfterHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    objBuffer += m_nDeltaSec;

    if (m_pszComment != SIP_NULL)
    {
        objBuffer += LPARAN;
        objBuffer += m_pszComment;
        objBuffer += RPARAN;
    }

    return (bParams == SIP_TRUE) ? EncodeParameters(objBuffer) : SIP_TRUE;
}

/*Function for encoding of headers*/
SIP_BOOL SipRetryAfterHeader::EncodeHdr(
        SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    SIP_CHAR szLen[MAX_RETRY_AFTER_LEN];
    SipPf_Sprintf(szLen, (SIP_CHAR*)"%u", m_nDeltaSec);

    SipPf_Strcpy(*ppCurrPos, szLen);
    SipEnc_UpdateCurrPos(ppCurrPos);

    if (m_pszComment != SIP_NULL)
    {
        SIP_ENC_LPAREN(*ppCurrPos);
        SipPf_Strcpy(*ppCurrPos, m_pszComment);
        SipEnc_UpdateCurrPos(ppCurrPos);
        SIP_ENC_RPAREN(*ppCurrPos);
    }

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

/*Sets */
SIP_BOOL SipRetryAfterHeader::SetComment(const SIP_CHAR* pszComment)
{
    return SetCharVar(pszComment, m_pszComment);
}

/******************************************************************************
 * Function name      : SipRetryAfterHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipRetryAfterHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pCommentStart = SIP_NULL;
    SIP_CHAR* pCommentEnd = SIP_NULL;
    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPre = SIP_NULL;
    SIP_CHAR* pTempNext = SIP_NULL;

    SIP_BOOL bStatus = FindComment(pStartPt, pEndPt, pCommentStart, pCommentEnd);

    if (bStatus == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid comment", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (sipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SIP_SEMI) == SIP_TRUE)
    {
        if ((pCommentEnd == SIP_NULL) || ((pTempPre + 1) > pCommentEnd))
        {
            if (DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI) == SIP_FALSE)
            {
                return SIP_FALSE;
            }
            pEndPt = pTempPre;
            pTempPre = SIP_NULL;
            pTempNext = SIP_NULL;
        }
    }
    else
    {
        // if there is some extra string after comment ends
        if ((pCommentEnd != SIP_NULL) && (pCommentEnd != pEndPt))
        {
            return SIP_FALSE;
        }
    }

    // if comment exists
    if (pCommentStart != SIP_NULL)
    {
        if ((pCommentStart + SIP_ONE) == pCommentEnd)
        {
            SetCharVar("", m_pszComment);
        }
        else
        {
            m_pszComment = sipCreateString(pCommentStart + SIP_ONE, pCommentEnd - SIP_ONE);
        }

        if (m_pszComment == SIP_NULL)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "DecodeHdr:Memory Allocation failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pEndPt = pCommentStart - 1;
    }

    pEndPt = sipSkipRwLWS(pStartPt, pEndPt);
    /*Now decode the delta sec value*/
    SIP_CHAR* pszValue = sipCreateString(pStartPt, pEndPt);
    if (pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr:Memory Allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (SipPf_Atoi_IsZero(pszValue) == SIP_FALSE)
    {
        m_nDeltaSec = SipPf_Atoi_Unsigned(pszValue);
        if ((m_nDeltaSec > MAX_CSEQ) || (m_nDeltaSec == SIP_ZERO))
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeHdr:Retry After value is not valid",
                    SIP_ZERO, SIP_ZERO);
            delete[] pszValue;
            return SIP_FALSE;
        }
    }
    else
    {
        m_nDeltaSec = SIP_ZERO;
    }
    delete[] pszValue;

    return SIP_TRUE;
}

SipHeaderBase* SipRetryAfterHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipRetryAfterHeader(*reinterpret_cast<SipRetryAfterHeader*>(pHeader));
    }
    return new SipRetryAfterHeader();
}
