/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipGeolocationRoutingHeader.h
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)           : sravanthi.p
 * E-mail id.            : sravanthi.panditi@
 * Creation date       : Feb. 05, 2013
 *
 * Edit HisAcceptry         Modification description(s)
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * Month. Date,10        Name                 0.0a            Initial creation
 *****************************************************************************/

#ifndef __SIP_GEOLOCATION_ROUTING_HEADER_H__
#define __SIP_GEOLOCATION_ROUTING_HEADER_H__

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

class SipGeolocationRoutingHeader : public SipHeaderBase
{
    private:
        SipNameValue* m_pGeoLocationRoutingList;

    public:
        /*constructor*/
        SipGeolocationRoutingHeader();

        /*Copy constructor*/
        SipGeolocationRoutingHeader(const SipGeolocationRoutingHeader& objHeader);

        /*destructor*/
        ~SipGeolocationRoutingHeader();
        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
        /*virtual methods*/
        /*Function for encoding of headers*/
        SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

        inline SIP_BOOL IsValidHeader() const
        {
            return (m_pGeoLocationRoutingList == SIP_NULL) ? SIP_FALSE : SIP_TRUE;
        }


};
#endif //__SIP_GEOLOCATION_ROUTING_HEADER_H__
