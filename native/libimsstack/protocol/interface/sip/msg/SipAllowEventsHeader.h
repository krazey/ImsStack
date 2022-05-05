#ifndef __SIP_ALLOW_EVENTS_HEADER_H__
#define __SIP_ALLOW_EVENTS_HEADER_H__

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
class SipAllowEventsHeader : public SipHeaderBase
{
    private:
        SipParameterList* m_pEventTemplateList;

    public:
        /*constructor*/
        SipAllowEventsHeader();
        SipAllowEventsHeader(const SipAllowEventsHeader& objHeader);

        /*destructor*/
        ~SipAllowEventsHeader();

        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

        /*virtual methods*/
        /*Function for encoding of headers*/
        SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
};
#endif //__SIP_ALLOW_EVENTS_HEADER_H__
