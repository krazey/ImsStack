/******************************************************************************
 * Project Name   : SIP_RTP
 * Group    : IP-CS [MSG-2]
 * Security   : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename      : SipContactHeader.h
 * Purpose     :
 * Platform      : Windows OR Android
 * Author(s)     :
 * E-mail id.    : saurabh31.srivastava@
 * Creation date   : Month. Date,10
 *
 * Edit HisAcceptry   Modification description(s)
 * Date      Name    Version    Bug-ID    Description
 * ----------    ----------    -------    ------    -------------
 * Month. Date,10    Name       0.0a    Initial creation
 *****************************************************************************/

#ifndef __SIP_CONTACT_HEADER_H__
#define __SIP_CONTACT_HEADER_H__

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

class SipContactHeader: public SipNameAddrHeader
{
    public:
        /*constructor*/
        SipContactHeader();
        SipContactHeader(const SipContactHeader& objHeader);

        /*destructor*/
        ~SipContactHeader();
        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
        /*virtual methods*/
        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

        /*Function for decoding of headers*/
};



#endif //__SIP_CONTACT_HEADER_H__
