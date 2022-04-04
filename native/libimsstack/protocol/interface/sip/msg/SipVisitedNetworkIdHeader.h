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

#ifndef __SIP_VISITED_NETWORK_ID_HEADER_H__
#define __SIP_VISITED_NETWORK_ID_HEADER_H__

/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "msg/SipHeaderBase.h"


/****************************************************************************
  Macro Definitions
 *****************************************************************************/


/****************************************************************************
  Enum Declaration
 *****************************************************************************/

/****************************************************************************
  Class Declaration Starts
 *****************************************************************************/

class SipVisitedNetworkIdHeader : public SipParameters
{
    private:
        SIP_CHAR* m_pszVisitedNetwork;

    public:
        /*constructor*/
        SipVisitedNetworkIdHeader();

        /*destructor*/
        ~SipVisitedNetworkIdHeader();

        /*virtual methods*/
        /*Function for encoding of headers*/
        SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

        /*Gets the visited nw name*/
        inline const SIP_CHAR* GetVisitedNetwork() const
        {
            return m_pszVisitedNetwork;
        }

};
#endif //__SIP_VISITED_NETWORK_ID_HEADER_H__
