/******************************************************************************
 * Project Name   : SIP_RTP
 * Group    : IP-CS [MSG-2]
 * Security   : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename      : SipAcceptHeader.h
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

#ifndef __SIP_ACCEPT_HEADER_H__
#define __SIP_ACCEPT_HEADER_H__

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

class SipAcceptHeader : public SipHeaderBase
{
    private:
        /*Media range*/
        SIP_CHAR* m_pszMType;
        SIP_CHAR* m_pszMSubType;

    public:
        /*constructor*/
        SipAcceptHeader();

        /*Copy constructor*/
        SipAcceptHeader(const SipAcceptHeader& objHeader);

        /*destructor*/
        ~SipAcceptHeader();
        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

        /*virtual methods*/
        /*Function for encoding of headers*/
        SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
        SIP_BOOL IsValidHeader() const;
        /*set methods*/
        SIP_BOOL SetMediaType(const SIP_CHAR* pszMType);

        SIP_BOOL SetMediaSubType(const SIP_CHAR* pszMSubType);

        /*Get methods*/
        inline const SIP_CHAR* GetMType() const
        {
            return m_pszMType;
        }

        inline const SIP_CHAR* GetMSubType() const
        {
            return m_pszMSubType;
        }

};
#endif //__SIP_ACCEPT_HEADER_H__
