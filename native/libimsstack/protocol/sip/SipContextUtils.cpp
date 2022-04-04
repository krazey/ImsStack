#include "SipContextUtils.h"

static SipContextUtils *gpUtil = SIP_NULL;

SipContextUtils* SipContextUtils::GetInstance()
{
    if (gpUtil == SIP_NULL)
    {
        gpUtil = new SipContextUtils();
    }

    return gpUtil;
}

SIP_VOID SipContextUtils::Destruct()
{
    if (gpUtil != SIP_NULL)
    {
        delete gpUtil;
        gpUtil = SIP_NULL;
    }
}

SipContextUtils::SipContextUtils()
{
}

SipContextUtils::~SipContextUtils()
{
}

SipTxnContext* SipContextUtils::Sip_CreateTxnContext()
{
    return new SipTxnContext();
}

void SipContextUtils::Sip_DestroyTxnContext(IN SipTxnContext* pContext)
{
    // Destroy SipTxnContext
    if (pContext != SIP_NULL)
    {
        delete pContext;
        pContext = SIP_NULL;
    }
}
