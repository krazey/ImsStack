/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipPolicyContactHeader.h
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)           : sravanthi.panditi
 * E-mail id.            : sravanthi.panditi@
 * Creation date       : Feb . 05,2013
 *
 * Edit HisAcceptry         Modification description(s)
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * Month. Date,10        Name                 0.0a            Initial creation
 *****************************************************************************/

#ifndef __SIP_POLICY_CONTACT_HEADER_H__
#define __SIP_POLICY_CONTACT_HEADER_H__

/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "msg/SipHeaderBase.h"

class SipPolicyContactHeader: public SipNameAddrHeader
{
    public:
        /*constructor*/
        SipPolicyContactHeader();
        SipPolicyContactHeader(const SipPolicyContactHeader& objHeader);

        /*destructor*/
        ~SipPolicyContactHeader();

        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
        /*virtual methods*/
        /*Function for encoding of headers*/
        SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
};



#endif //__SIP_POLICY_CONTACT_HEADER_H__
