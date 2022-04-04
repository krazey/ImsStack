#ifndef __SIP_PERCENT_ENCODING_H__
#define __SIP_PERCENT_ENCODING_H__

#include "sip_pf_datatypes.h"

class SipPercentEncoding
{

    public:
        static SIP_CHAR* DoPercentDecoding(SIP_CHAR* pszString);
        static SIP_CHAR* DoPercentDecoding(SIP_CHAR* pszString, SIP_INT32* pFinalLength);
        static SIP_CHAR* DoPerEnc_UserAndHeader(SIP_CHAR* pszString, SIP_CHAR* pType);
        static SIP_CHAR* DoPerEnc_Password(SIP_CHAR* pszString);
        static SIP_CHAR* DoPerEnc_Host(SIP_CHAR* pszString);
        static SIP_CHAR* DoPerEnc_Param(SIP_CHAR* pszName, SIP_CHAR* pszValue);
        static SIP_CHAR* DoPerEnc_TokenParam(SIP_CHAR* pszString);

        static SIP_CHAR* DoPerEnc_MddrParam(SIP_CHAR* pszString);
        static SIP_CHAR* DoPerEnc_TtlParam(SIP_CHAR* pszString);

        static SIP_CHAR* DoPerEnc_OtherParam(SIP_CHAR* pszString);
};



#endif //__SIP_PERCENT_ENCODING_H__
