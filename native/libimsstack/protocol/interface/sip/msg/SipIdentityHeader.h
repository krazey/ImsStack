/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipIdentityHeader.h
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)           : Saurabh Srivastava
 * E-mail id.            : saurabh31.srivastava@
 * Creation date       : April 19 2011
 *
 * Edit History         Modification description(s)
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * Month. Date,10        Name                 0.0a            Initial creation
 *****************************************************************************/

#ifndef __SIP_IDENTITY_HEADER_H__
#define __SIP_IDENTITY_HEADER_H__

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


class SipIdentityHeader : public SipHeaderBase
{
    public:
        /*constructor*/
        SipIdentityHeader();
        SipIdentityHeader(const SipIdentityHeader& objHeader);

        /*destructor*/
        ~SipIdentityHeader();

        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

        /*virtual methods*/
        /*Function for encoding of headers*/
        SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
};
#endif //__SIP_IDENTITY_HEADER_H__
