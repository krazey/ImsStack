#ifndef __SIP_TXN_CONTEXT_H__
#define __SIP_TXN_CONTEXT_H__

#include "txn/SipTxnTimerValues.h"
#include "SipTimerContext.h"

class SipTxnContext
{
    public:
        SipTxnContext();
        virtual ~SipTxnContext();

    private:
        SipTxnContext(const SipTxnContext& objRHS);
        SipTxnContext& operator=(const SipTxnContext& objRHS);

    public:
        SipTimerContext* pSipTimerContext;
        SIP_VOID* pTxnContextData;
};


#endif //__SIP_TXN_CONTEXT_H__
