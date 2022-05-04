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
