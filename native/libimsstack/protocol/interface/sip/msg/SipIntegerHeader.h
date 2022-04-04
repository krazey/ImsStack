#ifndef  SIP_INTEGER_HEADER_H_
#define  SIP_INTEGER_HEADER_H_

#include "msg/SipHeaderBase.h"

/**
 * @brief This class is for  Sip Headers which has header value as Integer.
 */

class SipIntegerHeader
    : public SipHeaderBase
{
    public:
        SipIntegerHeader(SIP_INT32 nHeaderType);
        SipIntegerHeader(const SipIntegerHeader& objHeader);
        virtual ~SipIntegerHeader();
        SIP_BOOL SetValueInt(const SIP_UINT32 nContLen);
        SIP_UINT32 GetValueInt() const;
        virtual SIP_BOOL EncodeHdr(SIP_CHAR** ppszCurrPos, SIP_BOOL bParams = SIP_TRUE);
        virtual SIP_BOOL DecodeHdr(SIP_CHAR* pszStartPt, SIP_UINT32 nDecLen);
        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
};
#endif
