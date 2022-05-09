#include "msg/SipUnknownHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"

/******************************************************************************
 * Function name      : SipUnknownHeader::SipUnknownHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipUnknownHeader::SipUnknownHeader() :
        SipHeaderBase(SipHeaderBase::UNKNOWN),
        m_pszHdrName(SIP_NULL),
        m_pszHdrValue(SIP_NULL)
{
}

/******************************************************************************
 * Function name      : SipUnknownHeader::SipUnknownHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipUnknownHeader::SipUnknownHeader(const SipUnknownHeader& objHeader) :
        SipHeaderBase(SipHeaderBase::UNKNOWN),
        m_pszHdrName(SipPf_Strdup(objHeader.m_pszHdrName)),
        m_pszHdrValue(SipPf_Strdup(objHeader.m_pszHdrValue))
{
}

/******************************************************************************
 * Function name      : SipUnknownHeader::~SipUnknownHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipUnknownHeader::~SipUnknownHeader()
{
    if (m_pszHdrName != SIP_NULL)
    {
        delete[] m_pszHdrName;
    }
    if (m_pszHdrValue != SIP_NULL)
    {
        delete[] m_pszHdrValue;
    }
}

/******************************************************************************
 * Function name      : SipUnknownHeader::EncodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipUnknownHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (m_pszHdrName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Header name not found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipPf_Strcpy(*ppCurrPos, m_pszHdrName);
    SipEnc_UpdateCurrPos(ppCurrPos);

    SIP_ENC_COLON(*ppCurrPos);

    SIP_ENC_SP(*ppCurrPos);

    SipPf_Strcpy(*ppCurrPos, m_pszHdrValue);
    SipEnc_UpdateCurrPos(ppCurrPos);

    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipUnknownHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipUnknownHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    (void)pStartPt;
    (void)nDecLen;

    return SIP_TRUE;
}

SIP_BOOL SipUnknownHeader::SetHeaderName(const SIP_CHAR* pszHdrName)
{
    return SetCharVar(pszHdrName, m_pszHdrName);
}

SIP_BOOL SipUnknownHeader::SetHeaderValue(const SIP_CHAR* pszHdrValue)
{
    return SetCharVar(pszHdrValue, m_pszHdrValue);
}

SipHeaderBase* SipUnknownHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipUnknownHeader(*reinterpret_cast<SipUnknownHeader*>(pHeader));
    }
    return new SipUnknownHeader();
}
