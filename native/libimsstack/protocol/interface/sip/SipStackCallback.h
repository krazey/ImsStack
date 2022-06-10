/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef SIP_STACK_CALLBACK_H_
#define SIP_STACK_CALLBACK_H_

#include "ImsTypeDef.h"

#include "ISipTimerUtil.h"
#include "ISipUserData.h"
#include "txn/sip_txn_common.h"

// Definition of function prototypes for SIP stack transaction layer
typedef SIP_BOOL (*SipStack_FetchTransaction)(
        IN SIP_VOID*, IN SIP_INT32, OUT SIP_VOID**, OUT SIP_VOID**);
typedef SIP_BOOL (*SipStack_ReleaseTransaction)(
        IN SIP_VOID*, IN SIP_INT32, OUT SIP_VOID**, OUT SIP_VOID**);
typedef SIP_BOOL (*SipStack_StartTimer)(
        IN SIP_UINT32, IN SipTimerCallback, IN SIP_VOID*, OUT SIP_VOID**);
typedef SIP_BOOL (*SipStack_StopTimer)(IN SIP_VOID*, OUT SIP_VOID**);
typedef SIP_VOID (*SipStack_OnTimerExpired)(IN ISipUserData*, IN SIP_INT32);

typedef SIP_VOID* (*SipStack_CreateAckRequest)(IN SIP_VOID*, IN ISipUserData*);
typedef SIP_VOID (*SipStack_PreProcessMessageSentByStack)(IN SIP_VOID*, IN ISipUserData*);
typedef SIP_VOID (*SipStack_PostProcessMessageSentByStack)(
        IN SIP_VOID*, IN SIP_CHAR*, IN SIP_UINT32, IN ISipUserData*);
typedef SIP_VOID (*SipStack_DisplayTxnKey)(IN SIP_VOID*);

// Definitions for all SIP stack callbacks
struct SipStackCallbacks
{
    SipStack_FetchTransaction pfnFetchTransaction;
    SipStack_ReleaseTransaction pfnReleaseTransaction;
    SipStack_StartTimer pfnStartTimer;
    SipStack_StopTimer pfnStopTimer;
    SipStack_OnTimerExpired pfnOnTimerExpired;
    SipStack_CreateAckRequest pfnCreateAckRequest;
    SipStack_PreProcessMessageSentByStack pfnPreProcessMessageSentByStack;
    SipStack_PostProcessMessageSentByStack pfnPostProcessMessageSentByStack;
    SipStack_DisplayTxnKey pfnDisplayTxnKey;
};

GLOBAL void SipStackCallback_SetCallbacks(IN const SipStackCallbacks& objCallbacks);

#endif
