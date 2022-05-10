#ifndef __SIP_UTIL_H__
#define __SIP_UTIL_H__

#include "ISipLoggerUtil.h"
#include "ISipNetworkUtil.h"
#include "ISipTimerUtil.h"
#include "ISipTxnListener.h"

class SipUtil
{
public:
    SipUtil();
    virtual ~SipUtil();

    SIP_VOID RegisterTimer(ISipTimerUtil* pTimerUtil);
    SIP_VOID RegisterLogger(ISipLoggerUtil* pLoggerUtil);
    SIP_VOID RegisterNetwork(ISipNetworkUtil* pNwUtil);
    SIP_VOID RegisterTxnListener(ISipTxnListener* pTxnListener);
    ISipTimerUtil* GetTimer();
    ISipLoggerUtil* GetLogger();
    ISipNetworkUtil* GetNetwork();
    ISipTxnListener* GetTxnListener();

private:
    ISipTimerUtil* m_pTimerUtil;
    ISipLoggerUtil* m_pLoggerUtil;
    ISipNetworkUtil* m_pNetworkUtil;
    ISipTxnListener* m_pTxnListener;

    SipUtil& operator=(IN const SipUtil& objRHS);
    SipUtil(IN const SipUtil& objRHS);
};

void SipUtil_Construct();
void SipUtil_Destruct();
SipUtil* SipUtil_GetInstance();

#endif  //__SIP_UTIL_H__
