#ifndef __SIP_EVENT_HEADER_H__
#define __SIP_EVENT_HEADER_H__

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

class SipEventHeader : public SipHeaderBase
{
    private:
        /*This is a part of header value but kept as prm*/
        SipParameterList* m_pEventTemplateList;

    public:
        /*constructor*/
        SipEventHeader();
        SipEventHeader(const SipEventHeader& objHeader);
        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
        /*destructor*/
        ~SipEventHeader();

        /*virtual methods*/
        /*Function for encoding of headers*/
        SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
};
#endif //__SIP_EVENT_HEADER_H__
