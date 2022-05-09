#ifndef __SIP_CONTEXT_UTILS_H__
#define __SIP_CONTEXT_UTILS_H__

#include "SipTxnContext.h"

class SipContextUtils
{
    SipContextUtils();

public:
    virtual ~SipContextUtils();

    static SipContextUtils* GetInstance();
    static SIP_VOID Destruct();

    SipTxnContext* Sip_CreateTxnContext();
    void Sip_DestroyTxnContext(IN SipTxnContext* pContext);
};

#endif  //__SIP_CONTEXT_UTILS_H__
