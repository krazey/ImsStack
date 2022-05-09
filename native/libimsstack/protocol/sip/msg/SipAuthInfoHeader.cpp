#include "msg/SipAuthInfoHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"

SipAuthInfoHeader::SipAuthInfoHeader() :
        SipHeaderBase(SipHeaderBase::AUTHENTICATION_INFO),
        m_pAiInfo(SIP_NULL)
{
}
SipAuthInfoHeader::SipAuthInfoHeader(const SipAuthInfoHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pAiInfo(SIP_NULL)
{
    m_pAiInfo = new SipNameValue(*(objHeader.m_pAiInfo));
}

SipAuthInfoHeader::~SipAuthInfoHeader()
{
    if (m_pAiInfo != SIP_NULL)
    {
        m_pAiInfo->SipDelete();
    }
}

SIP_BOOL SipAuthInfoHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (m_pAiInfo == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Auth info missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    SIP_CHAR* pMsgCurrPtr = *ppCurrPos;
    SIP_BOOL bStatus = m_pAiInfo->EncodeFromList(&pMsgCurrPtr);
    if (bStatus == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Name Value Encode fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    *ppCurrPos = pMsgCurrPtr;

    return bStatus;
}

const SIP_CHAR* SipAuthInfoHeader::GetAiInfoVal(SIP_UINT32 nPos /*default value is zero*/)
{
    if ((m_pAiInfo != SIP_NULL) && (nPos < m_pAiInfo->m_valueList.GetSize()))
    {
        return m_pAiInfo->m_valueList.GetAt(nPos);
    }
    return SIP_NULL;
}

/******************************************************************************
 * Function name      : SipAuthInfoHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipAuthInfoHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    m_pAiInfo = new SipNameValue(GetHdrType());
    if (m_pAiInfo == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    if (m_pAiInfo->DecHdrNameVal(pStartPt, pEndPt) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Name Value Decoding fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipAuthInfoHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipAuthInfoHeader(*reinterpret_cast<SipAuthInfoHeader*>(pHeader));
    }
    return new SipAuthInfoHeader();
}
