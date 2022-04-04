/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipBadHeader.h
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)           :
 * E-mail id.            : saurabh31.srivastava@
 * Creation date       : Month. Date,10
 *
 * Edit History         Modification description(s)
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * Month. Date,10        Name                 0.0a            Initial creation
 *****************************************************************************/

#ifndef __SIP_BAD_HEADER_H__
#define __SIP_BAD_HEADER_H__

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


class SipBadHeader : public SipHeaderBase
{
    private:
        SIP_CHAR* m_pszHdrName;

    public:
        /*constructor*/
        SipBadHeader();

        /*destructor*/
        ~SipBadHeader();

        SipBadHeader(const SipBadHeader& objHeader);

        /*virtual methods*/
        /*Function for encoding of headers*/
        SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

        /*set methods*/
        SIP_BOOL SetHeaderName(const SIP_CHAR* pszHdrName);

        /*Get methods*/
        inline const SIP_CHAR* GetHeaderName() const
        {
            return m_pszHdrName;
        }

        inline SIP_BOOL IsValidHeader() const
        {
            return SIP_TRUE;
        }

};
#endif //__SIP_BAD_HEADER_H__
