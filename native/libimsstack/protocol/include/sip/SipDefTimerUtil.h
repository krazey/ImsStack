#ifndef __SIP_DEFTIMERUTIL_H__
#define __SIP_DEFTIMERUTIL_H__

#include "sip_pf_datatypes.h"
#include "ISipTimerUtil.h"

/*This can be an inner private class of SipDefTimerUtil*/



class SipDefTimerUtil: public ISipTimerUtil
{
    //    DefTimerListener* m_pListener;
    public:
        SipDefTimerUtil();
        ~SipDefTimerUtil();

    public:
        SIP_BOOL StartTimer(SIP_VOID** ppvTimerId, SIP_UINT32 nDuration, SIP_UINT16 nResetFlag,
                SipTimerCallback pfnTimerCallback, SIP_VOID* pvData);//{return SIP_TRUE;}

        SIP_VOID* StopTimer(SIP_VOID* pvTimerId);//{return SIP_TRUE;}

        SIP_BOOL ResetTimer(SIP_VOID* pvTimerId, SIP_UINT32 nNewDuration);//{return SIP_TRUE;}

        SIP_VOID* StopTimerEx(SIP_VOID* pvTimerId);//{return SIP_TRUE;}
};

#endif //__SIP_DEFTIMERUTIL_H__
