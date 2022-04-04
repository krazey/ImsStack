/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipPPreferredServiceHeader.h
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)           : sravanthi panditi
 * E-mail id.            : sravanthi.panditi@
 * Creation date       : Feb. 05, 2013
 *
 * Edit History         Modification description(s)
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * Month. Date,10        Name                 0.0a            Initial creation
 *****************************************************************************/

#ifndef __SIP_P_PREFEERED_SERVICE_HEADER_H__
#define __SIP_P_PREFEERED_SERVICE_HEADER_H__

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

class SipPPreferredServiceHeader : public SipHeaderBase
{
    public:
        /*constructor*/
        SipPPreferredServiceHeader();
        SipPPreferredServiceHeader(const SipPPreferredServiceHeader& objHeader);

        /*destructor*/
        ~SipPPreferredServiceHeader();
        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

        /*virtual methods*/
        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
};
#endif //__SIP_P_PREFEERED_SERVICE_HEADER_H__
