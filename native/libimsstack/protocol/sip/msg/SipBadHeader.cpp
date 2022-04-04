/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipBadHeader.cpp
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)           :
 * E-mail id.            : kishorekumar.v@
 * Creation date       : Nov. 13, 2017
 *
 * Edit History             Modification                         Description(s)
 *
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * Month. Date,10        Name                 0.0a            Initial creation
 *****************************************************************************/


/******************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "msg/SipBadHeader.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"

/******************************************************************************
  Macro Definitions
 *****************************************************************************/

/******************************************************************************
  Class Member Function Implementations
 *****************************************************************************/

/******************************************************************************
 * Function name      : SipBadHeader::SipBadHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipBadHeader::SipBadHeader()
    : SipHeaderBase(TYPE_INVALID)
    , m_pszHdrName(SIP_NULL)
{
}



/******************************************************************************
 * Function name      : SipBadHeader::SipBadHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipBadHeader::SipBadHeader(const SipBadHeader& objHeader)
    : SipHeaderBase(objHeader)
    , m_pszHdrName(SipPf_Strdup(objHeader.m_pszHdrName))
{
}

/******************************************************************************
 * Function name      : SipUnknownHeader::~SipUnknownHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipBadHeader::~SipBadHeader()
{
    if (m_pszHdrName != SIP_NULL)
    {
        delete[] m_pszHdrName;
    }
}

SIP_BOOL SipBadHeader::SetHeaderName(const SIP_CHAR* pszHdrName)
{
    return SetCharVar(pszHdrName, m_pszHdrName);
}

/******************************************************************************
 * Function name      : SipUnknownHeader::EncodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipBadHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    (void)ppCurrPos;
    (void) bParams;
    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipUnknownHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipBadHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    (void)pStartPt;
    (void)nDecLen;

    return SIP_TRUE;
}
