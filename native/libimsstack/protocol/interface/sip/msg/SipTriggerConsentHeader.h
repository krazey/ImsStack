/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipTriggerConsentHeader.h
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)           : Saurabh Srivastava
 * E-mail id.            : saurabh31.srivastava@
 * Creation date       : May. 10, 2011
 *
 * Edit History         Modification description(s)
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * Month. Date,10        Name                 0.0a            Initial creation
 *****************************************************************************/

#ifndef __SIP_TRIGGER_CONSENT_HEADER_H__
#define __SIP_TRIGGER_CONSENT_HEADER_H__

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

class SipTriggerConsentHeader : public SipHeaderBase
{
    private:
        /*Media Disp*/
        SipUri* m_pSipUri;
    public:
        /*constructor*/
        SipTriggerConsentHeader();
        SipTriggerConsentHeader(const SipTriggerConsentHeader& objHeader);

        /*destructor*/
        ~SipTriggerConsentHeader();
        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

        /*virtual methods*/
        /*Function for encoding of headers*/
        SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

        SipUri* GetSipUri();

        SIP_BOOL SetSipUri(SipUri* pSipUri);
        inline SIP_BOOL IsValidHeader() const
        {
            return (m_pSipUri == SIP_NULL) ? SIP_FALSE : SIP_TRUE;
        }
};
#endif //__SIP_TRIGGER_CONSENT_HEADER_H__
