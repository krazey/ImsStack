/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipGeolocationErrorHeader.h
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

#ifndef __SIP_GEOLOCATION_ERROR_HEADER_H__
#define __SIP_GEOLOCATION_ERROR_HEADER_H__

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

class SipGeolocationErrorHeader : public SipHeaderBase
{
    public:
        /*constructor*/
        SipGeolocationErrorHeader();

        /*Copy constructor*/
        SipGeolocationErrorHeader(const SipGeolocationErrorHeader& objHeader);

        /*destructor*/
        ~SipGeolocationErrorHeader();
        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

        /*Sets */
        SIP_BOOL SetErrCode(SIP_UINT32 nErrCode);

        /*Gets */
        SIP_UINT32 GetErrCode() const;
};
#endif //__SIP_GEOLOCATION_ERROR_HEADER_H__
