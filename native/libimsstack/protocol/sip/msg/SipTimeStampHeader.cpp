#include "msg/SipTimeStampHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"

/*constructor*/
SipTimeStampHeader::SipTimeStampHeader() :
        SipHeaderBase(SipHeaderBase::TIMESTAMP),
        m_pszTimeVal(SIP_NULL),
        m_pszDelay(SIP_NULL)
{
}
/*Copy Constructor*/
SipTimeStampHeader::SipTimeStampHeader(const SipTimeStampHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pszTimeVal(SipPf_Strdup(objHeader.m_pszTimeVal)),
        m_pszDelay(SipPf_Strdup(objHeader.m_pszDelay))
{
}
/*destructor*/
SipTimeStampHeader::~SipTimeStampHeader()
{
    if (m_pszTimeVal != SIP_NULL)
    {
        delete[] m_pszTimeVal;
        m_pszTimeVal = SIP_NULL;
    }
    if (m_pszDelay != SIP_NULL)
    {
        delete[] m_pszDelay;
        m_pszDelay = SIP_NULL;
    }
}

/*virtual methods*/
SIP_BOOL SipTimeStampHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL /*bParams*/) const
{
    if (m_pszTimeVal == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode: Missing timestamp", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    objBuffer += m_pszTimeVal;

    if (m_pszDelay != SIP_NULL)
    {
        objBuffer += SPACE;
        objBuffer += m_pszDelay;
    }

    return SIP_TRUE;
}

/*Function for encoding of headers*/
/******************************************************************************
 * Function name      : SipTimeStampHeader::EncodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipTimeStampHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    /*Encoding of header Value  i.e.
      "Timestamp" HCOLON 1*(DIGIT) [ "." *(DIGIT) ] [ LWS delay ]   */
    /*Encoding of DIGIT*/
    if (m_pszTimeVal == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "EncodeHdr: Missing TimeStamp ", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipPf_Strcpy(*ppCurrPos, m_pszTimeVal);
    SipEnc_UpdateCurrPos(ppCurrPos);

    /*Encoding of Delay*/
    if (m_pszDelay != SIP_NULL)
    {
        SIP_ENC_SP(*ppCurrPos);

        SipPf_Strcpy(*ppCurrPos, m_pszDelay);
        SipEnc_UpdateCurrPos(ppCurrPos);
    }

    return SIP_TRUE;
}

/*Sets */
SIP_BOOL SipTimeStampHeader::SetTimeVal(const SIP_CHAR* pszTimeVal)
{
    return SetCharVar(pszTimeVal, m_pszTimeVal);
}

/*Sets */
SIP_BOOL SipTimeStampHeader::SetDelay(const SIP_CHAR* pszDelay)
{
    return SetCharVar(pszDelay, m_pszDelay);
}

/******************************************************************************
 * Function name      : SipTimeStampHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipTimeStampHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPre = SIP_NULL;
    /*Find the LWS i.e. End of Transport*/
    if (sipFindLWS(pStartPt, pEndPt, &pTempPre) == SIP_FALSE)
    {
        pTempPre = pEndPt;
    }

    m_pszTimeVal = sipCreateString(pStartPt, pTempPre);
    if (m_pszTimeVal == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr:Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (pTempPre != pEndPt)
    {
        /*point to the start of the LWS*/
        pTempPre = pTempPre + SIP_ONE;
        pStartPt = sipSkipFwLWS(pTempPre, pEndPt);
        m_pszDelay = sipCreateString(pStartPt, pEndPt);
        if (m_pszDelay == SIP_NULL)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "DecodeHdr:Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }

    return SIP_TRUE;
}

SipHeaderBase* SipTimeStampHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipTimeStampHeader(*reinterpret_cast<SipTimeStampHeader*>(pHeader));
    }
    return new SipTimeStampHeader();
}
