/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipUserAgentHeader.h
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)           :
 * E-mail id.            : giridhar.a@
 * Creation date       : July. 27,2010
 *
 * Edit HisAlertry         Modification description(s)
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * Month. Date,10        Giridhar               0.0a            Initial creation
 *****************************************************************************/

/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "msg/SipUserAgentHeader.h"
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
  Enum Declaration
 *****************************************************************************/

/****************************************************************************
  Class Declaration Starts
 *****************************************************************************/

/*constructor*/
SipUserAgentHeader::SipUserAgentHeader(SIP_INT32 eHdrType)
    : SipHeaderBase(eHdrType)
    , m_objProductList(SipVector<SIP_CHAR*>())
{
}


/*constructor*/
SipUserAgentHeader::SipUserAgentHeader(const SipUserAgentHeader& objHeader)
    : SipHeaderBase(objHeader)
    , m_objProductList(SipVector<SIP_CHAR*>())
{
    SIP_UINT32 nSize = objHeader.m_objProductList.GetSize();
    for (SIP_UINT32 nCount = SIP_ZERO; nCount < nSize; nCount++)
    {
        SIP_CHAR* pszTempszVal = objHeader.m_objProductList.GetAt(nCount);
        if (pszTempszVal != SIP_NULL)
        {
            SIP_CHAR* pszVal = SipPf_Strdup(pszTempszVal);
            m_objProductList.Add(pszVal);
        }
    }
}


/*destructor*/
SipUserAgentHeader::~SipUserAgentHeader()
{
    while(m_objProductList.IsEmpty() != SIP_TRUE)
    {
        delete m_objProductList.Top();
        m_objProductList.Pop();
    }
}


/*virtual methods*/
/*Function for encoding of headers*/
SIP_BOOL SipUserAgentHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (m_objProductList.IsEmpty() == SIP_TRUE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "No header body", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nSize = m_objProductList.GetSize();
    for (SIP_UINT32 nIndex = SIP_ZERO; nIndex < nSize; nIndex++)
    {
        if (nIndex != SIP_ZERO)
        {
            SIP_ENC_SP(*ppCurrPos);
        }
        SIP_CHAR* pszVal = m_objProductList.GetAt(nIndex);
        SipPf_Strcpy(*ppCurrPos, pszVal);
        SipEnc_UpdateCurrPos(ppCurrPos);
    }

    return SIP_TRUE;
}


/*Sets */
SIP_BOOL SipUserAgentHeader::AddProductNameVer(const SIP_CHAR* pszProduct)
{
    if (pszProduct == SIP_NULL)
    {
        return SIP_FALSE;
    }

    SIP_CHAR* pszTempProduct = SipPf_Strdup(pszProduct);
    if (pszTempProduct == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "Malloc Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    m_objProductList.Add(pszTempProduct);

    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipUserAgentHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipUserAgentHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pTempPos = SIP_NULL;
    SIP_CHAR* pCommentStart = SIP_NULL;
    SIP_CHAR* pCommentEnd = SIP_NULL;
    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    /*"UserAgent" HCOLON UserAgent-val *(LWS UserAgent-val) */
    while (pStartPt < pEndPt)
    {
        SIP_BOOL bStatus = FindComment(pStartPt, pEndPt, pCommentStart, pCommentEnd);

        if (sipFindLWS(pStartPt, pEndPt, &pTempPos) == SIP_FALSE)
        {
            pTempPos = pEndPt;
        }

        //check LWS is between comment
        if (bStatus == SIP_TRUE &&
            (pCommentStart < (pTempPos) && (pTempPos) < pCommentEnd) == SIP_TRUE)
        {
            pStartPt = pCommentStart;
            pTempPos = pCommentEnd;
        }

        SIP_CHAR* pszUserAgent = sipCreateString(pStartPt, pTempPos);
        if (pszUserAgent == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "DecodeHdr:Memory Allocation failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        /*Put the value into list*/
        if (m_objProductList.Add(pszUserAgent) < SIP_ZERO)
        {
            delete[] pszUserAgent;
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "DecodeHdr:Adding in list failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (pTempPos == pEndPt)
        {
            pStartPt = pEndPt;
        }
        else
        {
            pTempPos = pTempPos + SIP_ONE;
            pStartPt = sipSkipFwLWS(pTempPos, pEndPt);
            pTempPos = SIP_NULL;
        }
    }

    return SIP_TRUE;
}

SipHeaderBase* SipUserAgentHeader::GetNewObj(SIP_INT32 eHdr, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipUserAgentHeader(*reinterpret_cast<SipUserAgentHeader*>(pHeader));
    }
    return new SipUserAgentHeader(eHdr);
}
