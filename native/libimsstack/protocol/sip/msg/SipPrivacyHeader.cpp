#include "msg/SipPrivacyHeader.h"
#include "SipTrace.h"
#include "sip_debug.h"
#include "msg/sip_msgutil.h"
#include "platform/sip_pf_string.h"

/******************************************************************************
 * Function name      : SipPrivacyHeader::SipPrivacyHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipPrivacyHeader::SipPrivacyHeader() :
        SipHeaderBase(SipHeaderBase::PRIVACY),
        m_objPrivacyList(SipVector<SIP_CHAR*>())
{
}

/******************************************************************************
 * Function name      : SipPrivacyHeader::SipPrivacyHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipPrivacyHeader::SipPrivacyHeader(const SipPrivacyHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_objPrivacyList(SipVector<SIP_CHAR*>())
{
    SIP_UINT32 nSize = objHeader.m_objPrivacyList.GetSize();
    for (SIP_UINT32 nCount = SIP_ZERO; nCount < nSize; nCount++)
    {
        SIP_CHAR* pszTempVal = objHeader.m_objPrivacyList.GetAt(nCount);
        if (pszTempVal != SIP_NULL)
        {
            SIP_CHAR* pszVal = SipPf_Strdup(pszTempVal);
            m_objPrivacyList.Add(pszVal);
        }
    }
}

/******************************************************************************
 * Function name      : SipPrivacyHeader::~SipPrivacyHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipPrivacyHeader::~SipPrivacyHeader()
{
    while (m_objPrivacyList.IsEmpty() != SIP_TRUE)
    {
        delete m_objPrivacyList.Top();
        m_objPrivacyList.Pop();
    }
}

SIP_BOOL SipPrivacyHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL /*bParams*/) const
{
    SIP_UINT32 nSize = m_objPrivacyList.GetSize();

    if (nSize == 0)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Empty privacy values", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    for (SIP_UINT32 i = SIP_ZERO; i < nSize; i++)
    {
        const SIP_CHAR* pszPrivacy = m_objPrivacyList.GetAt(i);

        if (i != SIP_ZERO)
        {
            objBuffer += SIP_SEMI;
        }

        objBuffer += pszPrivacy;
    }

    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipPrivacyHeader::EncodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipPrivacyHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    SIP_UINT32 nCount = m_objPrivacyList.GetSize();

    if (nCount == 0)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Empty privacy values", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    for (SIP_UINT32 nIndex = SIP_ZERO; nIndex < nCount; nIndex++)
    {
        SIP_CHAR* pszPrivacy = m_objPrivacyList.GetAt(nIndex);
        if (nIndex != SIP_ZERO)
        {
            SIP_ENC_SEMI(*ppCurrPos);
        }
        SipPf_Strcpy(*ppCurrPos, pszPrivacy);
        SipEnc_UpdateCurrPos(ppCurrPos);
    }
    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipPrivacyHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
/*SIP_BOOL SipPrivacyHeader::DecodeHdr
  (
  SIP_CHAR   * pStartPt,
  SIP_UINT32  nDecLen
  )
  {
  return SIP_TRUE;
  }*/

/******************************************************************************
 * Function name      : SipPrivacyHeader::SetPrivacy
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipPrivacyHeader::AddPrivacy(const SIP_CHAR* pszPrivacy)
{
    SIP_CHAR* pszTempPrivacy = SIP_NULL;
    if (SetCharVar(pszPrivacy, pszTempPrivacy) == SIP_TRUE)
    {
        return ((m_objPrivacyList.Add(pszTempPrivacy) >= 0) ? SIP_TRUE : SIP_FALSE);
    }
    return SIP_FALSE;
}

/******************************************************************************
 * Function name      : SipMinSEHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipPrivacyHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    /*"Privacy" HCOLON priv-value *(";" priv-value)*/
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "SipPrivacyHeader::DecodeHdr:Privacy Value Missing",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    while (pStartPt < pEndPt)
    {
        SIP_CHAR* pTempPos = SIP_NULL;

        if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SIP_SEMI) == SIP_FALSE)
        {
            pTempPos = pEndPt;
        }

        SIP_CHAR* pszPrivacy = sipCreateString(pStartPt, pTempPos);
        if (pszPrivacy == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipPrivacyHeader::DecodeHdr:Memory Allocation failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        /*Put the value into list*/
        if (m_objPrivacyList.Add(pszPrivacy) < SIP_ZERO)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipPrivacyHeader::DecodeHdr:Adding in list failed", SIP_ZERO, SIP_ZERO);
            delete[] pszPrivacy;
            return SIP_FALSE;
        }

        if (pTempPos == pEndPt)
        {
            break;
        }
        else
        {
            pStartPt = pTempPos + SIP_TWO;
            pStartPt = sipSkipFwLWS(pStartPt, pEndPt);
            if (pStartPt > pEndPt)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                        "SipParameterList::DecHdrSipParameterList: No Parameter Present", SIP_ZERO,
                        SIP_ZERO);
                return SIP_FALSE;
            }
        }
    }

    return SIP_TRUE;
}

SipHeaderBase* SipPrivacyHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipPrivacyHeader(*reinterpret_cast<SipPrivacyHeader*>(pHeader));
    }
    return new SipPrivacyHeader();
}
