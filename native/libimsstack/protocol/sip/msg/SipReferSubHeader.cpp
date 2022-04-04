/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipReferSubHeader.cpp
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
#include "msg/SipReferSubHeader.h"
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

SipReferSubHeader::SipReferSubHeader()
    : SipHeaderBase(SipHeaderBase::REFER_SUB)
{
}

SipReferSubHeader::SipReferSubHeader(const SipReferSubHeader& objHeader)
    : SipHeaderBase(objHeader)
{
}

/*destructor*/
SipReferSubHeader::~SipReferSubHeader()
{
}

/******************************************************************************
 * Function name      : SipReferSubHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipReferSubHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (SipHeaderBase::DecodeHdr(pStartPt, nDecLen) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    const SIP_CHAR* pszValue = GetValue();

    if ((pszValue != SIP_NULL) &&
        (SipPf_Stricmp(pszValue, "true") != SIP_ZERO) &&
        (SipPf_Stricmp(pszValue, "false") != SIP_ZERO))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,"Invalid Only true or false allowed",
                          SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipReferSubHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipReferSubHeader(*reinterpret_cast<SipReferSubHeader*>(pHeader));
    }
    return new SipReferSubHeader();
}
