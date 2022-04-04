/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipEventHeader.cpp
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
#include "msg/SipEventHeader.h"
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
 * Function name      : SipEventHeader::SipEventHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipEventHeader::SipEventHeader()
    : SipHeaderBase(SipHeaderBase::EVENT)
    , m_pEventTemplateList(SIP_NULL)
{
}


/******************************************************************************
 * Function name      : SipEventHeader::SipEventHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipEventHeader::SipEventHeader(const SipEventHeader& objHeader)
    : SipHeaderBase(objHeader)
    , m_pEventTemplateList(SIP_NULL)
{
    if (objHeader.m_pEventTemplateList != SIP_NULL)
    {
        m_pEventTemplateList = new SipParameterList(*(objHeader.m_pEventTemplateList));
    }
}


/******************************************************************************
 * Function name      : SipEventHeader::~SipEventHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipEventHeader::~SipEventHeader()
{
    if (m_pEventTemplateList != SIP_NULL)
    {
        m_pEventTemplateList->SipDelete();
    }
}

/******************************************************************************
 * Function name      : SipEventHeader::EncodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipEventHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    const SIP_CHAR* pszValue = GetValue();
    if (pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "No Evt package", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipPf_Strcpy(*ppCurrPos, pszValue);
    SipEnc_UpdateCurrPos(ppCurrPos);
    if (m_pEventTemplateList != SIP_NULL)
    {
        SipVector<SipNameValue*> &sipList = m_pEventTemplateList->GetList();
        SIP_UINT32 nSize = sipList.GetSize();
        for (SIP_UINT32 nIndex = SIP_ZERO; nIndex < nSize; nIndex++)
        {
            SipNameValue* pNmVl = sipList.GetAt(nIndex);
            if (pNmVl != SIP_NULL)
            {
                SIP_ENC_DOT(*ppCurrPos);

                SipPf_Strcpy(*ppCurrPos, pNmVl->m_pszName);
                SipEnc_UpdateCurrPos(ppCurrPos);
            }
        }
    }

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

/******************************************************************************
 * Function name      : SipEventHeader::SetEventPkg
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipEventHeader::AddEventTemplate(const SIP_CHAR* pszEvntTmpl)
{
    if (pszEvntTmpl == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODACCESSOR, "Empty value", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pEventTemplateList ==  SIP_NULL)
    {
        m_pEventTemplateList = new SipParameterList();
    }

    if (m_pEventTemplateList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODACCESSOR, "Memory Allocation fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return m_pEventTemplateList->Add(pszEvntTmpl);
}


/******************************************************************************
 * Function name      : SipEventHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipEventHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPre = SIP_NULL;
    SIP_CHAR* pTempNext = SIP_NULL;

    if (sipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SIP_SEMI) == SIP_TRUE)
    {
        if (DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI) == SIP_FALSE)
        {
            return SIP_FALSE;
        }
        pEndPt = pTempPre;
    }

    SIP_CHAR* pTempPos = SIP_NULL;
    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SIP_DOT) == SIP_FALSE)
    {
        pTempPos = pEndPt;
    }

    SIP_CHAR* pszEvent = sipCreateString(pStartPt, pTempPos);
    if (SetValue(pszEvent) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
        if (pszEvent != SIP_NULL)
        {
            delete[] pszEvent;
        }
        return SIP_FALSE;
    }
    delete[] pszEvent;

    if (pTempPos != pEndPt)
    {
        m_pEventTemplateList = new SipParameterList();
        if (m_pEventTemplateList == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        pTempPos = pTempPos + SIP_TWO;

        if (m_pEventTemplateList->DecUriSipParameterList(pTempPos,
                pEndPt, SIP_DOT) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Event Package Decoding Failed",
                SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    return SIP_TRUE;
}

SipHeaderBase* SipEventHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipEventHeader(*reinterpret_cast<SipEventHeader*>(pHeader));
    }
    return new SipEventHeader();
}
