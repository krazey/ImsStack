/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipRSeqHeader.h
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

#ifndef __SIP_RSEQ_HEADER_H__
#define __SIP_RSEQ_HEADER_H__

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

class SipRSeqHeader : public SipHeaderBase
{
    public:
        /*constructor*/
        SipRSeqHeader();

        SipRSeqHeader(const SipRSeqHeader& objHeader);

        /*destructor*/
        ~SipRSeqHeader();
        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
        /*Sets */
        SIP_BOOL SetRSeqValue(SIP_UINT32 nRSeqValue);

        /*Gets */
        SIP_UINT32 GetRSeqValue();
        SIP_BOOL IsValidHeader() const;

};
#endif //__SIP_RSEQ_HEADER_H__
