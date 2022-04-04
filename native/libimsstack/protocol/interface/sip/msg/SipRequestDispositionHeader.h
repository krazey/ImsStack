/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipRequestDispositionHeader.h
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

#ifndef __SIP_REQUEST_DISPOSITION_HEADER_H__
#define __SIP_REQUEST_DISPOSITION_HEADER_H__

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

class SipRequestDispositionHeader : public SipHeaderBase
{
    public:
        /*constructor*/
        SipRequestDispositionHeader();
        SipRequestDispositionHeader(const SipRequestDispositionHeader& objHeader);
        /*destructor*/
        ~SipRequestDispositionHeader();
        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
        /*virtual methods*/
        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
};
#endif //__SIP_REQUEST_DISPOSITION_HEADER_H__
