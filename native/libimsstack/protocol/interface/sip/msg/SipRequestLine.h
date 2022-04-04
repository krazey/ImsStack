/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipRequestLine.h
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)           :
 * E-mail id.            : saurabh31.srivastava@
 * Creation date       : July. 26, 2010
 *
 * Edit History         Modification description(s)
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * Month. Date,10        Name                 0.0a            Initial creation
 *****************************************************************************/
#ifndef __SIP_REQUEST_LINE_H__
#define __SIP_REQUEST_LINE_H__

/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "SipRefBase.h"
#include "msg/sip_comdef.h"
#include "SipPercentEncoding.h"
#include "msg/IParameterComponent.h"

/****************************************************************************
  Macro Definitions
 *****************************************************************************/

/****************************************************************************
  Enum Declaration
 *****************************************************************************/

/****************************************************************************
  Class Declaration Starts
 *****************************************************************************/

class SipRequestLine: public SipRefBase , public IParameterComponent
{
    private:
        SIP_CHAR* m_pszMethod;
        SipAddrSpec* m_pReqUri;
        SIP_CHAR* m_pszSipVersion;

    public:
        /*Constructor*/
        SipRequestLine();

        SipRequestLine(SIP_CHAR* pszMethod, SipAddrSpec* pReqUri, const SIP_CHAR* pszSipVersion);
        SipRequestLine(const SipRequestLine& objHeader);
        /*Destructor*/
        ~SipRequestLine();


        /*Function for encoding*/
        SIP_BOOL EncodeRequestLine(SIP_CHAR** ppCurrPos);

        /*Function for decoding*/
        SIP_BOOL DecodeRequestLine(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

        /*Set Methods*/
        SIP_BOOL SetMethod(const SIP_CHAR* pszMethod);

        SIP_BOOL SetSipVersion(const SIP_CHAR* pszVer);

        SIP_BOOL SetReqUri(SipAddrSpec* pAddrSpec);

        /*###TODO */
        inline const SIP_CHAR* GetMethod() const
        {
           return m_pszMethod;
        }

        inline const SIP_CHAR* GetSipVersion() const
        {
            return m_pszSipVersion;
        }

        SipAddrSpec* GetReqUri();
        SIP_BOOL IsValidComponent(const SIP_CHAR* pszComponent) const;

};

/****************************************************************************
  Class Declaration Ends
 *****************************************************************************/

#endif //__SIP_REQUEST_LINE_H__
