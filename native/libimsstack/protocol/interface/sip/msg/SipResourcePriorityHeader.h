#ifndef __SIP_RESOURCE_PRIORITY_HEADER_H__
#define __SIP_RESOURCE_PRIORITY_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipResourcePriorityHeader : public SipHeaderBase
{
private:
    SIP_CHAR* m_pszNameSpace;
    SIP_CHAR* m_pszRPriority;

public:
    /*constructor*/
    SipResourcePriorityHeader();
    SipResourcePriorityHeader(const SipResourcePriorityHeader& objHeader);

    /*destructor*/
    ~SipResourcePriorityHeader();
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    /*virtual methods*/
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    /*set methods*/
    SIP_BOOL SetNameSpace(const SIP_CHAR* pszNameSpace);

    SIP_BOOL SetRPriority(const SIP_CHAR* pszRPriority);

    /*Get methods*/
    inline const SIP_CHAR* GetNameSpace() const { return m_pszNameSpace; }

    inline const SIP_CHAR* GetResourcePriority() const { return m_pszRPriority; }

    inline SIP_BOOL IsValidHeader() const
    {
        return ((m_pszNameSpace == SIP_NULL) || (m_pszRPriority == SIP_NULL)) ? SIP_FALSE
                                                                              : SIP_TRUE;
    }
};
#endif  //__SIP_RESOURCE_PRIORITY_HEADER_H__
