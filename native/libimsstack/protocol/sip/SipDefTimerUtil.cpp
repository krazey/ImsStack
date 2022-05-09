#include "sip_pf_datatypes.h"
#include "platform/sip_pf_string.h"
#include "SipDefTimerUtil.h"
#include "ServiceTimer.h"

extern SIP_BOOL sip_cbk_startTimer(IN SIP_UINT32 nDuration,
        IN SipTimerCallback pfnTimerCallback, IN SIP_VOID* pvData, IN SIP_VOID** ppvHandle);
extern SIP_BOOL sip_cbk_stopTimer(IN SIP_VOID* pvHandle, IN SIP_VOID** ppvData);

SipDefTimerUtil::SipDefTimerUtil()
{

}
SipDefTimerUtil::~SipDefTimerUtil()
{

}

SIP_BOOL SipDefTimerUtil::StartTimer(SIP_VOID** ppvTimerId, SIP_UINT32 nDuration,
        SIP_UINT16 nResetFlag, SipTimerCallback pfnTimerCallback, SIP_VOID* pvData)
{
    if (ppvTimerId == SIP_NULL)
    {
        return SIP_FALSE;
    }

    (void)nResetFlag;

    return sip_cbk_startTimer(nDuration, pfnTimerCallback, pvData, ppvTimerId);

}
SIP_VOID* SipDefTimerUtil::StopTimer(SIP_VOID* pvTimerId)
{
    if (pvTimerId == SIP_NULL)
    {
        return SIP_NULL;
    }

    SIP_VOID* pvData = SIP_NULL;
    sip_cbk_stopTimer(pvTimerId, &pvData);

    return pvData;
}

SIP_VOID* SipDefTimerUtil::StopTimerEx(SIP_VOID* pvTimerId)
{
    return StopTimer(pvTimerId);
}

SIP_BOOL SipDefTimerUtil::ResetTimer(SIP_VOID* pvTimerId, SIP_UINT32 nNewDuration)
{
    (void)pvTimerId;
    (void)nNewDuration;
    return SIP_FALSE;
}
