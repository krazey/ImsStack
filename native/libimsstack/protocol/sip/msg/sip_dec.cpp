/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : sip_dec.cpp
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)           : Saurabh Srivastava
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

#include "msg/sip_msgutil.h"
#include "sip_pf_datatypes.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"
#include "msg/SipHeaders.h"
#include "msg/SipMessage.h"
#include "msg/SipHeaderBase.h"
/****************************************************************************
  Macro Definitions
 *****************************************************************************/
#define MAX__CONTACT_EXPIRES 4294967295

#define SIP_MAX_HDR_LEN 32


extern SIPHdrAccess* gpHdrAccess;


/******************************************************************************
 * Function name      : sipSkipRwWSP
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_CHAR* sipSkipRwWSP(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt)
{
    /*NULL validation*/
    if ((pStartPt == SIP_NULL) || (pEndPt == SIP_NULL))
    {
        return SIP_NULL;
    }
    while ((pEndPt > pStartPt) && IS_WSP(*pEndPt))
    {
        pEndPt--;
    }
    return pEndPt;
}




/******************************************************************************
 * Function name      : sipFindPostDelimiter
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL sipFindPostDelimiter(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR** ppTempLoc,
        SIP_CHAR cDelimiter)
{
    while (pStartPt <= pEndPt)
    {
        if (*pStartPt == cDelimiter)
        {
            *ppTempLoc = pStartPt + SIP_ONE;
            return SIP_TRUE;
        }
        pStartPt = pStartPt + SIP_ONE;
    }
    return SIP_FALSE;
}




/*****************************************************************************
 * Function name      : sipGetUriType
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_INT32 sipGetUriType(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt)
{
    SIP_UINT32 nSize = (pEndPt - pStartPt) + SIP_ONE;
    if (SipPf_Memcmp(pStartPt, SIP_SIP, nSize) == 0)
    {
        return SipUri::SCHEME_SIP;
    }
    else if (SipPf_Memcmp(pStartPt, SIP_SIPS, nSize) == 0)
    {
        return SipUri::SCHEME_SIPS;
    }
    return SipUri::SCHEME_ABS;
}


/*****************************************************************************
 * Function name      : sipGetHdrType
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_INT32 sipGetHdrType(const SIP_CHAR* pszHdrName)
{
    gpHdrAccess = SIPHdrAccess::GetInstance();
    return gpHdrAccess->GetHdrType(pszHdrName);
}

/*****************************************************************************
 * Function name      : CheckAndGetHdrEnumType
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_INT32 CheckAndGetHdrEnumType(SIP_INT32 nType)
{
    //support EXPIRES_ANY & EXPIRES_DATE
    if ((nType == SipHeaderBase::EXPIRES_ANY) || (nType == SipHeaderBase::EXPIRES_DATE))
    {
        nType = SipHeaderBase::EXPIRES_SEC;
    }//support CONTACT_ANY & CONTACT_WILD
    else if ((nType == SipHeaderBase::CONTACT_ANY) || (nType == SipHeaderBase::CONTACT_WILD))
    {
        nType = SipHeaderBase::CONTACT;
    }//Support for Retry-After Any & Sec header
    else if ((nType == SipHeaderBase::RETRY_AFTER_ANY) ||
             (nType == SipHeaderBase::RETRY_AFTER_DATE))
    {
        nType = SipHeaderBase::RETRY_AFTER_SEC;
    }

    return nType;
}

/****************************************************************************
  Class Member Function Implementations
 *****************************************************************************/
SIP_BOOL IsValidAddress(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    SIP_CHAR* pTempLoc = SIP_NULL;
    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;

    /*Find the Start of header parm*/
    if (sipFindPostDelimiter(pStartPt, pEndPt, &pTempLoc, QMARK) == SIP_FALSE)
    {
        return SIP_TRUE;
    }

    pStartPt = pTempLoc;
    pTempLoc = SIP_NULL;

    if (sipFindPostDelimiter(pStartPt, pEndPt, &pTempLoc, PERCENT) == SIP_FALSE)
    {
        return SIP_TRUE;
    }
    return SIP_FALSE;
}
