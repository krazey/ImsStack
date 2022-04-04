/******************************************************************************
 * Project Name   : SIP_RTP
 * Group    : IP-CS [MSG-2]
 * Security   : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename      : SipContentTypeHeader.h
 * Purpose     :
 * Platform      : Windows OR Android
 * Author(s)     :
 * E-mail id.    : saurabh31.srivastava@
 * Creation date   : Month. Date,10
 *
 * Edit History   Modification description(s)
 * Date      Name    Version    Bug-ID    Description
 * ----------    ----------    -------    ------    -------------
 * Month. Date,10    Name       0.0a    Initial creation
 *****************************************************************************/

#ifndef __SIP_CONTENT_TYPE_HEADER_H__
#define __SIP_CONTENT_TYPE_HEADER_H__

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

class SipContentTypeHeader : public SipHeaderBase
{
    private:
        /*Media type*/
        SIP_CHAR* m_pszMType;

        SIP_CHAR* m_pszMSubType;
    public:
        /*constructor*/
        SipContentTypeHeader();
        SipContentTypeHeader(const SipContentTypeHeader& objHeader);

        /*destructor*/
        ~SipContentTypeHeader();
        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
        /*virtual methods*/
        /*Function for encoding of headers*/
        SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

        /*set methods*/
        SIP_BOOL SetMediaType(const SIP_CHAR* pszMtype);
        /*set methods*/
        SIP_BOOL SetSubMediaType(const SIP_CHAR* pszMSubtype);

        SIP_CHAR* GetBoundary();

        /*Get Display Name*/
        inline const SIP_CHAR* GetMediaType() const
        {
           return m_pszMType;
        }

        inline const SIP_CHAR* GetSubMediaType() const
        {
           return m_pszMSubType;
        }

        SIP_CHAR* StripDQUOTE(const SIP_CHAR* pszStr);
        SIP_BOOL IsValidHeader() const;

};
#endif //__SIP_CONTENT_TYPE_HEADER_H__
