#include "msg/SipPVisitedNetworkIdHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "msg/sip_msgutil.h"

/******************************************************************************
 * Function name      : SipPVisitedNetworkIdHeader::SipPVisitedNetworkIdHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipPVisitedNetworkIdHeader::SipPVisitedNetworkIdHeader() :
        SipHeaderBase(SipHeaderBase::P_VISITED_NETWORK_ID)
{
}

/******************************************************************************
 * Function name      : SipPVisitedNetworkIdHeader::SipPVisitedNetworkIdHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipPVisitedNetworkIdHeader::SipPVisitedNetworkIdHeader(
        const SipPVisitedNetworkIdHeader& objHeader) :
        SipHeaderBase(objHeader)
{
}

/******************************************************************************
 * Function name      : SipPVisitedNetworkIdHeader::~SipPVisitedNetworkIdHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipPVisitedNetworkIdHeader::~SipPVisitedNetworkIdHeader() {}

/******************************************************************************
 * Function name      : SipPVisitedNetworkIdHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipPVisitedNetworkIdHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    /*Case of nothing is present*/
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*First Check the presence of Header Prm i.e. ";"
      And decode if present*/
    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPre = SIP_NULL;
    SIP_CHAR* pTempNext = SIP_NULL;
    if (sipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SIP_SEMI) == SIP_TRUE)
    {
        if (DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "DecodeHdr: Hdr Prm Decoding Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        /*Update the End point for Previous Decoding*/
        pEndPt = pTempPre;
    }

    /* value can be token or quoted string
       P-Visited-Network-ID   = "P-Visited-Network-ID" HCOLON vnetwork-spec *(COMMA vnetwork-spec)
       vnetwork-spec  = (token / quoted-string) *(SEMI vnetwork-param)
       vnetwork-param = generic-param
     */
    //--------------------------------------------------------------------------------
    if ((DQUOTE == *pStartPt) &&
            (pEndPt != SIP_NULL && (DQUOTE == *pEndPt) && BACKSLASH != *(pEndPt - 1)))
    {
        pStartPt = pStartPt + SIP_ONE;
        pEndPt = pEndPt - SIP_ONE;
    }
    //-----------------------------------------------------------------------------
    SIP_CHAR* pszValue = sipCreateString(pStartPt, pEndPt);
    if (SetValue(pszValue) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr:Memory Allocation failed", SIP_ZERO, SIP_ZERO);
        if (pszValue != SIP_NULL)
        {
            delete[] pszValue;
        }
        return SIP_FALSE;
    }
    delete[] pszValue;

    return SIP_TRUE;
}

SipHeaderBase* SipPVisitedNetworkIdHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipPVisitedNetworkIdHeader(
                *reinterpret_cast<SipPVisitedNetworkIdHeader*>(pHeader));
    }
    return new SipPVisitedNetworkIdHeader();
}
