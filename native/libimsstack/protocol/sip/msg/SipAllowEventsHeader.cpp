/******************************************************************************
 * Project Name   : SIP_RTP
 * Group    : IP-CS [MSG-2]
 * Security   : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename      : SipAllowEventsHeader.cpp
 * Purpose     :
 * Platform      : Windows OR Android
 * Author(s)     :
 * E-mail id.    : saurabh31.srivastava@
 * Creation date   : July. 27, 2010
 *
 * Edit History     Modification         Description(s)
 *
 * Date      Name    Version    Bug-ID    Description
 * ----------    ----------    -------    ------    -------------
 * Month. Date,10    Name       0.0a    Initial creation
 *****************************************************************************/


/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "msg/SipAllowEventsHeader.h"
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
 * Function name  : SipAllowEventsHeader::SipAllowEventsHeader
 *
 * Description   :
 *
 * Preconditions  :
 *
 * Side Effects  : none
 *****************************************************************************/
SipAllowEventsHeader::SipAllowEventsHeader()
    : SipHeaderBase(SipHeaderBase::ALLOW_EVENTS)
    , m_pEventTemplateList(SIP_NULL)
{
}


/******************************************************************************
 * Function name  : SipAllowEventsHeader::SipAllowEventsHeader
 *
 * Description   :
 *
 * Preconditions  :
 *
 * Side Effects  : none
 *****************************************************************************/
SipAllowEventsHeader::SipAllowEventsHeader(const SipAllowEventsHeader& objHeader)
    : SipHeaderBase(objHeader)
    , m_pEventTemplateList(SIP_NULL)
{
    if (objHeader.m_pEventTemplateList != SIP_NULL)
    {
        m_pEventTemplateList = new SipParameterList(*(objHeader.m_pEventTemplateList));
    }
}


/******************************************************************************
 * Function name  : SipAllowEventsHeader::~SipAllowEventsHeader
 *
 * Description   :
 *
 * Preconditions  :
 *
 * Side Effects  : none
 *****************************************************************************/
SipAllowEventsHeader::~SipAllowEventsHeader()
{
    if (m_pEventTemplateList != SIP_NULL)
    {
        m_pEventTemplateList->SipDelete();
    }
}

/******************************************************************************
 * Function name  : SipAllowEventsHeader::EncodeHdr
 *
 * Description   :
 *
 * Preconditions  :
 *
 * Side Effects  : none
 *****************************************************************************/
SIP_BOOL SipAllowEventsHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    const SIP_CHAR* pszValue = GetValue();
    if (pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing Event package", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipPf_Strcpy(*ppCurrPos, pszValue);
    SipEnc_UpdateCurrPos(ppCurrPos);

    return (m_pEventTemplateList != SIP_NULL) ?
        m_pEventTemplateList->EncodeList(ppCurrPos,SIP_DOT) : SIP_TRUE;
}


SIP_BOOL SipAllowEventsHeader::AddEvtTemplate(const SIP_CHAR* pszEvntTmpl)
{
    if (m_pEventTemplateList == SIP_NULL)
    {
        m_pEventTemplateList = new SipParameterList();
    }

    if (m_pEventTemplateList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Memory allocation fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pszTempEvTmpl = SIP_NULL;
    if (SetCharVar(pszEvntTmpl, pszTempEvTmpl) == SIP_TRUE)
    {
        SIP_BOOL bResult = m_pEventTemplateList->Add(pszTempEvTmpl);
        delete[] pszTempEvTmpl;
        return bResult;
    }

    return SIP_FALSE;

}

/******************************************************************************
 * Function name      : SipAllowEventsHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipAllowEventsHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPos = SIP_NULL;

    /*Case of having event template*/
    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SIP_DOT) == SIP_FALSE)
    {
        pTempPos = pEndPt;
    }

    SIP_CHAR* pszValue = sipCreateString(pStartPt, pTempPos);
    if (SetValue(pszValue) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
        if (pszValue != SIP_NULL)
        {
            delete[] pszValue;
        }
        return SIP_FALSE;
    }
    delete[] pszValue;

    if (pTempPos != pEndPt)
    {
        m_pEventTemplateList = new SipParameterList(GetHdrType());
        if (m_pEventTemplateList == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        /*Update the tempPos to the start of eventTamplate*/
        pTempPos = pTempPos + SIP_TWO;
        if (m_pEventTemplateList->DecUriSipParameterList(pTempPos,
                pEndPt, SIP_DOT) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Hdr Prm Decoding Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    return SIP_TRUE;
}

SipHeaderBase* SipAllowEventsHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipAllowEventsHeader(*reinterpret_cast<SipAllowEventsHeader*>(pHeader));
    }
    return new SipAllowEventsHeader();
}
