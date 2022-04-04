/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipFeatureCapsHeader.h
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)           : sravanthi panditi
 * E-mail id.            : sravanthi.panditi@
 * Creation date       : Feb. 05, 2013
 *
 * Edit HisAcceptry         Modification description(s)
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * Month. Date,10        Name                 0.0a            Initial creation
 *****************************************************************************/

#ifndef __SIP_FEATURE_CAPS_HEADER_H__
#define __SIP_FEATURE_CAPS_HEADER_H__

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

class SipFeatureCapsHeader : public SipHeaderBase
{
    public:
        /*constructor*/
        SipFeatureCapsHeader();

        /*Copy constructor*/
        SipFeatureCapsHeader(const SipFeatureCapsHeader& objHeader);

        /*destructor*/
        ~SipFeatureCapsHeader();
        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
};
#endif //__SIP_FEATURE_CAPS_HEADER_H__
