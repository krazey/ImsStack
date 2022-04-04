/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipFeatureCapsHeader.cpp
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)           :    sravanthi panditi
 * E-mail id.            : sravanthi.panditi@
 * Creation date       : Feb. 05, 2013
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

#include "msg/SipFeatureCapsHeader.h"
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
 * Function name      : SipFeatureCapsHeader::SipFeatureCapsHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipFeatureCapsHeader::SipFeatureCapsHeader()
    : SipHeaderBase(SipHeaderBase::FEATURE_CAPS)
{
}

/************************************************ ******************************
 * Function name      : SipFeatureCapsHeader::SipFeatureCapsHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipFeatureCapsHeader::SipFeatureCapsHeader(const SipFeatureCapsHeader& objHeader)
    : SipHeaderBase(objHeader)
{
}

/******************************************************************************
 * Function name      : SipFeatureCapsHeader::~SipFeatureCapsHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipFeatureCapsHeader::~SipFeatureCapsHeader()
{
}

SIP_BOOL SipFeatureCapsHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (SipHeaderBase::DecodeHdr(pStartPt, nDecLen) == SIP_FALSE)
    {
      return SIP_FALSE;
    }

    const SIP_CHAR* pszValue = GetValue();

    if ((pszValue != SIP_NULL) && (SipPf_Strlen(pszValue) == SIP_ONE) && IS_ASTERISK(*pszValue))
    {
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipFeatureCapsHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipFeatureCapsHeader(*reinterpret_cast<SipFeatureCapsHeader*>(pHeader));
    }
    return new SipFeatureCapsHeader();
}
