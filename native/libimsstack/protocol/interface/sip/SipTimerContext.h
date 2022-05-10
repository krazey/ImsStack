#ifndef __SIP_TIMER_CONTEXT_H__
#define __SIP_TIMER_CONTEXT_H__

#include "txn/SipTxnTimerValues.h"

class SipTimerContext
{
public:
    SipTimerContext();
    virtual ~SipTimerContext();

private:
    SipTimerContext(const SipTimerContext& objRHS);
    SipTimerContext& operator=(const SipTimerContext& objRHS);

public:
    SipTxnTimerValues* pTxnSipTxnTimers;
    SIP_UINT32 nTimerOptions;
};

#endif  //__SIP_TIMER_CONTEXT_H__
