/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipContentTypeHeader.cpp
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)           :
 * E-mail id.            : saurabh31.srivastava@
 * Creation date       : July. 27, 2010
 *
 * Edit History             Modification                         Description(s)
 *
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * Month. Date,10        Name                 0.0a            Initial creation
 *****************************************************************************/


/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "msg/SipContentTypeHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"

/****************************************************************************
  Macro Definitions
 *****************************************************************************/

/****************************************************************************
  Class Member Function Implementations
 *****************************************************************************/

/******************************************************************************
 * Function name      : SipContentTypeHeader::SipContentTypeHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipContentTypeHeader::SipContentTypeHeader()
    : SipHeaderBase(SipHeaderBase::CONTENT_TYPE)
    , m_pszMType(SIP_NULL)
    , m_pszMSubType(SIP_NULL)
{
}

/******************************************************************************
 * Function name      : SipContentTypeHeader::SipContentTypeHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipContentTypeHeader::SipContentTypeHeader(const SipContentTypeHeader& objHeader)
    : SipHeaderBase(objHeader)
    , m_pszMType(SipPf_Strdup(objHeader.m_pszMType))
    , m_pszMSubType(SipPf_Strdup(objHeader.m_pszMSubType))
{
}

/******************************************************************************
 * Function name      : SipContentTypeHeader::~SipContentTypeHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipContentTypeHeader::~SipContentTypeHeader()
{
    if (m_pszMType != SIP_NULL)
    {
        delete[] m_pszMType;
    }
    if (m_pszMSubType != SIP_NULL)
    {
        delete[] m_pszMSubType;
    }
}

/******************************************************************************
 * Function name      : SipContentTypeHeader::EncodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipContentTypeHeader::EncodeHdr(SIP_CHAR** ppCurrPos,
        SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    if ((m_pszMType == SIP_NULL) || (m_pszMSubType == SIP_NULL))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing MType or MSubType", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipPf_Strcpy(*ppCurrPos, m_pszMType);
    SipEnc_UpdateCurrPos(ppCurrPos);

    SIP_ENC_SLASH(*ppCurrPos);

    SipPf_Strcpy(*ppCurrPos, m_pszMSubType);
    SipEnc_UpdateCurrPos(ppCurrPos);

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

/******************************************************************************
 * Function name      : SipContentTypeHeader::SetMediaType
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipContentTypeHeader::SetMediaType(const SIP_CHAR* pszMtype)
{
    return SetCharVar(pszMtype, m_pszMType);
}
/******************************************************************************
 * Function name      : SipContentTypeHeader::SetMediaType
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipContentTypeHeader::SetSubMediaType(const SIP_CHAR* pszMSubtype)
{
    return SetCharVar(pszMSubtype, m_pszMSubType);
}

/******************************************************************************
 * Function name      : SipContentTypeHeader::GetBoundary
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_CHAR* SipContentTypeHeader::GetBoundary()
{
    SipParameters* pParameters = GetParameters();

    if (pParameters == SIP_NULL)
    {
        return SIP_NULL;
    }

    SipParameterList* pParameterList = pParameters->GetParameterList();

    if (pParameterList == SIP_NULL)
    {
        return SIP_NULL;
    }

    SIP_CHAR* pszVal = pParameterList->GetParamValue("boundary");
    SIP_CHAR* pszStripDquoteVal = StripDQUOTE(pszVal);
    if (pszStripDquoteVal == SIP_NULL)
    {
        return pszVal;
    }

    delete[] pszVal;
    return pszStripDquoteVal;
}


/******************************************************************************
 * Function name      :SipContentTypeHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/

SIP_BOOL SipContentTypeHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPre = SIP_NULL;
    SIP_CHAR* pTempNext = SIP_NULL;
    /*Find the SLASH*/
    if (sipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SLASH) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "SLASH missing in Accept", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pszMType = sipCreateString(pStartPt, pTempPre);
    if (m_pszMType == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    pStartPt = pTempNext;
    pTempNext = SIP_NULL;
    pTempPre = SIP_NULL;

    if (sipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SIP_SEMI) == SIP_FALSE)
    {
        pTempPre = pEndPt;
    }

    m_pszMSubType = sipCreateString(pStartPt, pTempPre);
    if (m_pszMSubType == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (pTempNext != SIP_NULL)
    {
        return DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI);
    }
    return SIP_TRUE;
}

SIP_BOOL SipContentTypeHeader::IsValidHeader() const
{
    if ((m_pszMType == SIP_NULL) || (m_pszMSubType == SIP_NULL))
    {
        return SIP_FALSE;
    }
    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipContentTypeHeader::GetNewObj
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/

SipHeaderBase* SipContentTypeHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipContentTypeHeader(*reinterpret_cast<SipContentTypeHeader*>(pHeader));
    }
    return new SipContentTypeHeader();
}

SIP_CHAR* SipContentTypeHeader::StripDQUOTE(const SIP_CHAR* pszStr)
{
    SIP_INT32 nStrLen = SipPf_Strlen(pszStr);
    if (nStrLen <= SIP_ONE)
    {
        return SIP_NULL;
    }
    const SIP_CHAR* pEndPtr = pszStr + nStrLen - SIP_ONE;
    if (IS_DQUOTE(*pszStr) && IS_DQUOTE(*pEndPtr))
    {
        return sipCreateString(pszStr + SIP_ONE, pEndPtr - SIP_ONE);
    }
    return SIP_NULL;
}
