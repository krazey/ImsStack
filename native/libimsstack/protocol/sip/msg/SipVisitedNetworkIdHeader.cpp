/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipVisitedNetworkIdHeader.h
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
#include "msg/SipVisitedNetworkIdHeader.h"
#include "platform/sip_pf_string.h"


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
SipVisitedNetworkIdHeader::SipVisitedNetworkIdHeader()
    : SipParameters()
    , m_pszVisitedNetwork(SIP_NULL)
{
}

/*destructor*/
SipVisitedNetworkIdHeader::~SipVisitedNetworkIdHeader()
{
}

/*virtual methods*/
/*Function for encoding of headers*/
SIP_BOOL SipVisitedNetworkIdHeader::EncodeHdr(SIP_CHAR** ppCurrPos,
        SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    (void)ppCurrPos;
    (void)bParams;
    return SIP_TRUE;
}

/*Function for decoding of headers*/
SIP_BOOL SipVisitedNetworkIdHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    (void)pStartPt;
    (void)nDecLen;
    return SIP_TRUE;
}
