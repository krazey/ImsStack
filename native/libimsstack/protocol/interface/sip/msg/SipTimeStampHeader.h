/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipTimeStampHeader.h
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

#ifndef __SIP_TIME_STAMP_HEADER_H__
#define __SIP_TIME_STAMP_HEADER_H__

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

class SipTimeStampHeader : public SipHeaderBase
{
    private:
        /*Time*/
        SIP_CHAR* m_pszTimeVal;

        /*Delay*/
        SIP_CHAR* m_pszDelay;

    public:
        /*constructor*/
        SipTimeStampHeader();

        /*Copy Constructor*/
        SipTimeStampHeader(const SipTimeStampHeader& objHeader);

        /*destructor*/
        ~SipTimeStampHeader();
        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

        /*virtual methods*/
        /*Function for encoding of headers*/
        SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

        /*Sets */
        SIP_BOOL SetTimeVal(const SIP_CHAR* pszTimeVal);

        /*Gets */
        inline const SIP_CHAR* GetTimeVal() const
        {
            return m_pszTimeVal;
        }


        /*Sets */
        SIP_BOOL SetDelay(const SIP_CHAR* pszDelay);

        /*Gets */
        inline const SIP_CHAR* GetDelay() const
        {
            return m_pszDelay;
        }
        inline SIP_BOOL IsValidHeader() const
        {
            return (m_pszTimeVal == SIP_NULL) ? SIP_FALSE : SIP_TRUE;
        }

};
#endif //__SIP_TIME_STAMP_HEADER_H__
