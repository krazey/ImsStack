#include "msg/SipResourcePriorityHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "msg/sip_msgutil.h"

/******************************************************************************
 * Function name      : SipResourcePriorityHeader::SipResourcePriorityHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipResourcePriorityHeader::SipResourcePriorityHeader() :
        SipHeaderBase(SipHeaderBase::RESOURCE_PRIORITY),
        m_pszNameSpace(SIP_NULL),
        m_pszRPriority(SIP_NULL)
{
}

/******************************************************************************
 * Function name      : SipResourcePriorityHeader::SipResourcePriorityHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipResourcePriorityHeader::SipResourcePriorityHeader(const SipResourcePriorityHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pszNameSpace(SipPf_Strdup(objHeader.m_pszNameSpace)),
        m_pszRPriority(SipPf_Strdup(objHeader.m_pszRPriority))
{
}
/******************************************************************************
 * Function name      : SipResourcePriorityHeader::~SipResourcePriorityHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipResourcePriorityHeader::~SipResourcePriorityHeader()
{
    if (m_pszNameSpace != SIP_NULL)
    {
        delete[] m_pszNameSpace;
    }

    if (m_pszRPriority != SIP_NULL)
    {
        delete[] m_pszRPriority;
    }
}

/******************************************************************************
 * Function name      : SipResourcePriorityHeader::EncodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipResourcePriorityHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if ((m_pszNameSpace == SIP_NULL) || (m_pszRPriority == SIP_NULL))
    {
        return SIP_FALSE;
    }

    SipPf_Strcpy(*ppCurrPos, m_pszNameSpace);
    SipEnc_UpdateCurrPos(ppCurrPos);

    SIP_ENC_DOT(*ppCurrPos);

    SipPf_Strcpy(*ppCurrPos, m_pszRPriority);
    SipEnc_UpdateCurrPos(ppCurrPos);

    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipResourcePriorityHeader::SetNameSpace
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipResourcePriorityHeader::SetNameSpace(const SIP_CHAR* pszNameSpace)
{
    return SetCharVar(pszNameSpace, m_pszNameSpace);
}

/******************************************************************************
 * Function name      : SipResourcePriorityHeader::SetSeq
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipResourcePriorityHeader::SetRPriority(const SIP_CHAR* pszRPriority)
{
    return SetCharVar(pszRPriority, m_pszRPriority);
}

/******************************************************************************
 * Function name      : SipResourcePriorityHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipResourcePriorityHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPre = SIP_NULL;

    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPre, SIP_DOT) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipResourcePriorityHeader::DecodeHdr: Dot missing in ResourcePriority", SIP_ZERO,
                SIP_ZERO);
        return SIP_FALSE;
    }

    m_pszNameSpace = sipCreateString(pStartPt, pTempPre);
    if (m_pszNameSpace == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr:Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    pStartPt = pTempPre + SIP_TWO;
    m_pszRPriority = sipCreateString(pStartPt, pEndPt);
    if (m_pszRPriority == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr:Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipResourcePriorityHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipResourcePriorityHeader(
                *reinterpret_cast<SipResourcePriorityHeader*>(pHeader));
    }
    return new SipResourcePriorityHeader();
}
