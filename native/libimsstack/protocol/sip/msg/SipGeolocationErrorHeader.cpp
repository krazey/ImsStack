/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipGeolocationErrorHeader.cpp
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)           :    sravanthi.panditi
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

#include "msg/SipGeolocationErrorHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"

/****************************************************************************
  Macro Definitions
 *****************************************************************************/
#define MAX_ERR_LEN 4
/****************************************************************************
  Class Member Function Implementations
 *****************************************************************************/

/******************************************************************************
 * Function name      : SipGeolocationErrorHeader::SipGeolocationErrorHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipGeolocationErrorHeader::SipGeolocationErrorHeader()
    : SipHeaderBase(SipHeaderBase::GEOLOCATION_ERROR)
{
}

/******************************************************************************
 * Function name      : SipGeolocationErrorHeader::SipGeolocationErrorHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipGeolocationErrorHeader::SipGeolocationErrorHeader(const SipGeolocationErrorHeader& objHeader)
    : SipHeaderBase(objHeader)
{
}

/******************************************************************************
 * Function name      : SipGeolocationErrorHeader::~SipGeolocationErrorHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipGeolocationErrorHeader::~SipGeolocationErrorHeader()
{
}

/*Sets */
SIP_BOOL SipGeolocationErrorHeader::SetErrCode(SIP_UINT32 nErrCode)
{
    SIP_CHAR szLen[MAX_ERR_LEN];
    SipPf_Sprintf(szLen, (SIP_CHAR*)"%u", nErrCode);
    return SetValue(szLen);
}

/*Gets */
SIP_UINT32 SipGeolocationErrorHeader::GetErrCode() const
{
    return SipPf_Atoi(GetValue());
}

SipHeaderBase* SipGeolocationErrorHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipGeolocationErrorHeader(
            *reinterpret_cast<SipGeolocationErrorHeader*>(pHeader));
    }
    return new SipGeolocationErrorHeader();
}
