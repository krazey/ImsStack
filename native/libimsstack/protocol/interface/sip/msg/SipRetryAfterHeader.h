/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipRetryAfterHeader.h
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

#ifndef __SIP_RETRY_AFTER_HEADER_H__
#define __SIP_RETRY_AFTER_HEADER_H__

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

class SipRetryAfterHeader : public SipHeaderBase
{
    private:
        /*Delta seconds*/
        SIP_UINT32 m_nDeltaSec;

        /*Comment*/
        SIP_CHAR* m_pszComment;

    public:
        /*constructor*/
        SipRetryAfterHeader();

        SipRetryAfterHeader(const SipRetryAfterHeader& objHeader);

        /*destructor*/
        ~SipRetryAfterHeader();

        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

        /*virtual methods*/
        /*Function for encoding of headers*/
        SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

        /*Sets */
        inline SIP_VOID SetDeltaSec(SIP_UINT32 nDeltaSec)
        {
            m_nDeltaSec = nDeltaSec;
        }
        /*Gets */
        inline SIP_UINT32 GetDeltaSec() const
        {
           return m_nDeltaSec;
        }

        /*Sets */
        SIP_BOOL SetComment(const SIP_CHAR* pszComment);
        /*Gets */
        inline const SIP_CHAR* GetComment() const
        {
           return m_pszComment;
        }

};
#endif //__SIP_RETRY_AFTER_HEADER_H__
