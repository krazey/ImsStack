/******************************************************************************
 * Project Name   : SIP_RTP
 * Group    : IP-CS [MSG-2]
 * Security   : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename      : SipAuthBase.h
 * Purpose     :
 * Platform      : Windows OR Android
 * Author(s)     : Saurabh Srivastava
 * E-mail id.    : saurabh31.srivastava@
 * Creation date   : May. 9 ,2011
 *
 * Edit HisAlertry   Modification description(s)
 * Date      Name    Version    Bug-ID    Description
 * ----------    ----------    -------    ------    -------------
 * Month. Date,10    Saurabh      0.0a    Initial creation
 *****************************************************************************/

#ifndef __SIP_AUTH_BASE_H__
#define __SIP_AUTH_BASE_H__

/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "msg/SipHeaderBase.h"
#include "msg/sip_comdef.h"

/****************************************************************************
  Macro Definitions
 *****************************************************************************/

/****************************************************************************
  Enum Declaration
 *****************************************************************************/

/****************************************************************************
  Class Declaration Starts
 *****************************************************************************/


class SipAuthBase : public SipHeaderBase
{
    protected:
        /*Credential*/
        SipVector<SipNameValue*> m_objAuthList;

        SIP_BOOL EncodeAuthList(SIP_CHAR** ppCurrPos, SIP_CHAR cDelimiter);

    public:
        /*constructor*/
        SipAuthBase(SIP_INT32 eHdrType);
        SipAuthBase(const SipAuthBase& objHeader);

        /*destructor*/
        ~SipAuthBase();

        /*virtual methods*/
        /*Function for encoding of headers*/
        SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

        SIP_BOOL SetParams(const SIP_CHAR* pszName, const SIP_CHAR* pszVal,
                SIP_BOOL bIsFeatureParam);

        SIP_BOOL FindElement(const SIP_CHAR* pszName, SipNameValue*& pNmvl, SIP_UINT32& nPos);

        SIP_CHAR* GetAuthValue(const SIP_CHAR* pszName);

        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
};
#endif //__SIP_AUTH_BASE_H__
