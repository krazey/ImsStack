#include "msg/SipIdentityHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"

/******************************************************************************
 * Function name      : SipIdentityHeader::SipIdentityHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipIdentityHeader::SipIdentityHeader() :
        SipHeaderBase(SipHeaderBase::IDENTITY)
{
}

/******************************************************************************
 * Function name      : SipIdentityHeader::SipIdentityHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipIdentityHeader::SipIdentityHeader(const SipIdentityHeader& objHeader) :
        SipHeaderBase(objHeader)
{
}

/******************************************************************************
 * Function name      : SipIdentityHeader::~SipIdentityHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipIdentityHeader::~SipIdentityHeader() {}

/******************************************************************************
 * Function name      : SipIdentityHeader::EncodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipIdentityHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    const SIP_CHAR* pszValue = GetValue();
    if (pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Empty value", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Encode the Left DQoute*/
    SIP_ENC_LDQUOT(*ppCurrPos);

    /*Encoding of header Value*/
    SipPf_Strcpy(*ppCurrPos, pszValue);
    SipEnc_UpdateCurrPos(ppCurrPos);

    /*Encode the right DQoute*/
    SIP_ENC_RDQUOT(*ppCurrPos);

    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipIdentityHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipIdentityHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Find the position of First DQUOTE*/
    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTemp = SIP_NULL;

    if (sipFindPostDelimiter(pStartPt, pEndPt, &pTemp, DQUOTE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "left DQUOTE not found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    pStartPt = pTemp;
    pTemp = SIP_NULL;

    /*Find the position of Second DQUOTE*/
    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTemp, DQUOTE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "right DQUOTE not found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pszValue = sipCreateString(pStartPt, pTemp);
    if (SIP_FALSE == SetValue(pszValue))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
        if (pszValue != SIP_NULL)
        {
            delete[] pszValue;
        }
        return SIP_FALSE;
    }
    delete[] pszValue;

    return SIP_TRUE;
}

SipHeaderBase* SipIdentityHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipIdentityHeader(*reinterpret_cast<SipIdentityHeader*>(pHeader));
    }
    return new SipIdentityHeader();
}
