/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20191224  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SIP_STACK_CALLBACK_H_
#define _SIP_STACK_CALLBACK_H_

#include "ISipUserData.h"
#include "ISipTimerUtil.h"
#include "txn/sip_txn_common.h"

// Definition of function prototypes for SIP stack transaction layer
typedef SIP_BOOL (*SIPStack_FetchTransaction)
        (IN SIP_VOID*, IN SIP_INT32, OUT SIP_VOID**, OUT SIP_VOID**);
typedef SIP_BOOL (*SIPStack_ReleaseTransaction)
        (IN SIP_VOID*, IN SIP_INT32, OUT SIP_VOID**, OUT SIP_VOID**);
typedef SIP_BOOL (*SIPStack_StartTimer)
        (IN SIP_UINT32, IN SipTimerCallback, IN SIP_VOID*, OUT SIP_VOID**);
typedef SIP_BOOL (*SIPStack_StopTimer)
        (IN SIP_VOID*, OUT SIP_VOID**);
typedef SIP_VOID (*SIPStack_OnTimerExpired)
        (IN ISipUserData*, IN SIP_INT32);

typedef SIP_VOID* (*SIPStack_CreateAckRequest)(IN SIP_VOID*,
        IN ISipUserData*);
typedef SIP_VOID (*SIPStack_PreProcessMessageSentByStack)(IN SIP_VOID*,
        IN ISipUserData*);
typedef SIP_VOID (*SIPStack_PostProcessMessageSentByStack)(IN SIP_VOID*,
        IN SIP_CHAR*, IN SIP_UINT32, IN ISipUserData*);
typedef SIP_VOID (*SIPStack_DisplayTxnKey)(IN SIP_VOID*);

// Definitions for all SIP stack callbacks
struct SIPStackCallbacks
{
    SIPStack_FetchTransaction pfnFetchTransaction;
    SIPStack_ReleaseTransaction pfnReleaseTransaction;
    SIPStack_StartTimer pfnStartTimer;
    SIPStack_StopTimer pfnStopTimer;
    SIPStack_OnTimerExpired pfnOnTimerExpired;
    SIPStack_CreateAckRequest pfnCreateAckRequest;
    SIPStack_PreProcessMessageSentByStack pfnPreProcessMessageSentByStack;
    SIPStack_PostProcessMessageSentByStack pfnPostProcessMessageSentByStack;
    SIPStack_DisplayTxnKey pfnDisplayTxnKey;
};

GLOBAL void SIPStackCallback_SetCallbacks(IN const SIPStackCallbacks &stCallbacks);

#endif // _SIP_STACK_CALLBACK_H_
