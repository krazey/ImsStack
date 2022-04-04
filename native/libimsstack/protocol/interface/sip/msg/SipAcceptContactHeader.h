/******************************************************************************
 * Project Name   : SIP_RTP
 * Group    : IP-CS [MSG-2]
 * Security   : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename      : SipAcceptContactHeader.h
 * Purpose     :
 * Platform      : Windows OR Android
 * Author(s)     :
 * E-mail id.    : giridhar.a@
 * Creation date   : July. 27,2010
 *
 * Edit HisAlertry   Modification description(s)
 * Date      Name    Version    Bug-ID    Description
 * ----------    ----------    -------    ------    -------------
 * Month. Date,10    Giridhar     0.0a    Initial creation
 *****************************************************************************/

#ifndef  __SIP_ACCEPT_CONTACT_HEADER_H__
#define  __SIP_ACCEPT_CONTACT_HEADER_H__

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

class SipAcceptContactHeader : public SipHeaderBase
{
    public:
        /*constructor*/
        SipAcceptContactHeader();

        SipAcceptContactHeader(const SipAcceptContactHeader& objHeader);

        /*destructor*/
        ~SipAcceptContactHeader();
        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

        /*virtual methods*/
        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

        SIP_BOOL SetValue(const SIP_CHAR* pszContact);

};
#endif //__SIP_ACCEPT_CONTACT_HEADER_H__
