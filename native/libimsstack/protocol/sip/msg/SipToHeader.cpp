/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipToHeader.cpp
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
#include "sip_pf_datatypes.h"
#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"
#include "sip_error.h"
#include "SipTrace.h"
#include "sip_debug.h"
#include "msg/SipAddrSpec.h"
#include "msg/SipToHeader.h"
#include "msg/sip_msgutil.h"

/****************************************************************************
  Macro Definitions
 *****************************************************************************/

/****************************************************************************
  Class Member Function Implementations
 *****************************************************************************/

/******************************************************************************
 * Function name      : SipToHeader::SipToHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipToHeader::SipToHeader()
    : SipNameAddrHeader(SipHeaderBase::TO)
{
}

/******************************************************************************
 * Function name      : SipToHeader::SipToHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipToHeader::SipToHeader(const SipToHeader& objHeader)
    : SipNameAddrHeader(objHeader)
{
}
/******************************************************************************
 * Function name      : SipToHeader::~SipToHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipToHeader::~SipToHeader()
{
}

/******************************************************************************
 * Function name      : SipToHeader::GetTag
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_CHAR* SipToHeader::GetTag()
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
    return pParameterList->GetParamValue("tag");
}

/******************************************************************************
 * Function name      : SipToHeader::SetTag
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipToHeader::SetTag(SIP_CHAR* pszToTag)
{
    SipParameters* pParameters = GetParameters();

    if (pParameters == SIP_NULL)
    {
        InitParameters(SIP_NULL);
        pParameters = GetParameters();
    }

    if (pParameters == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SetTag: Memory Allocation Failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_BOOL bStatus = pParameters->AddParam("tag", pszToTag);
    if (bStatus == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "SetTag: Set Tag failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    return SIP_TRUE;
}

SipHeaderBase* SipToHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipToHeader(*reinterpret_cast<SipToHeader*>(pHeader));
    }
    return new SipToHeader();
}
