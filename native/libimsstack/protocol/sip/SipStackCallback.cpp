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
#include "SipStackCallback.h"

static SipStackCallbacks s_objSipStackCallbacks = {
        IMS_NULL,  // SipStack_FetchTransaction
        IMS_NULL,  // SipStack_ReleaseTransaction
        IMS_NULL,  // SipStack_StartTimer
        IMS_NULL,  // SipStack_StopTimer
        IMS_NULL,  // SipStack_OnTimerExpired
        IMS_NULL,  // SipStack_CreateAckRequest
        IMS_NULL,  // SipStack_PreProcessMessageSentByStack
        IMS_NULL,  // SipStack_PostProcessMessageSentByStack
        IMS_NULL   // SipStack_DisplayTxnKey
};

GLOBAL void SipStackCallback_SetCallbacks(IN const SipStackCallbacks& objCallbacks)
{
    s_objSipStackCallbacks = objCallbacks;
}

// Implements the function prototypes for SIP stack transaction layer
GLOBAL SIP_BOOL Sip_Cbk_FetchTransaction(IN SIP_VOID* pvTxnKey, IN SIP_INT32 nOption,
        OUT SIP_VOID** ppvOutTxnKey, OUT SIP_VOID** ppvTxn)
{
    if (s_objSipStackCallbacks.pfnFetchTransaction == IMS_NULL)
    {
        return SIP_FALSE;
    }

    return (s_objSipStackCallbacks.pfnFetchTransaction)(pvTxnKey, nOption, ppvOutTxnKey, ppvTxn);
}

GLOBAL SIP_BOOL Sip_Cbk_ReleaseTransaction(IN SIP_VOID* pvTxnKey, IN SIP_INT32 nOption,
        OUT SIP_VOID** ppvOutTxnKey, OUT SIP_VOID** ppvTxn)
{
    if (s_objSipStackCallbacks.pfnReleaseTransaction == IMS_NULL)
    {
        return SIP_FALSE;
    }

    return (s_objSipStackCallbacks.pfnReleaseTransaction)(pvTxnKey, nOption, ppvOutTxnKey, ppvTxn);
}

GLOBAL SIP_BOOL Sip_Cbk_StartTimer(IN SIP_UINT32 nDuration, IN SipTimerCallback pfnTimerCallback,
        IN SIP_VOID* pvData, IN SIP_VOID** ppvHandle)
{
    if (s_objSipStackCallbacks.pfnStartTimer == IMS_NULL)
    {
        return SIP_FALSE;
    }

    return (s_objSipStackCallbacks.pfnStartTimer)(nDuration, pfnTimerCallback, pvData, ppvHandle);
}

GLOBAL SIP_BOOL Sip_Cbk_StopTimer(IN SIP_VOID* pvHandle, IN SIP_VOID** ppvData)
{
    if (s_objSipStackCallbacks.pfnStopTimer == IMS_NULL)
    {
        return SIP_FALSE;
    }

    return (s_objSipStackCallbacks.pfnStopTimer)(pvHandle, ppvData);
}

GLOBAL SIP_VOID Sip_Cbk_OnTimerExpired(IN ISipUserData* pUserData, IN SIP_INT32 enTimerType)
{
    if (s_objSipStackCallbacks.pfnOnTimerExpired == IMS_NULL)
    {
        return;
    }

    return (s_objSipStackCallbacks.pfnOnTimerExpired)(pUserData, enTimerType);
}

GLOBAL SIP_VOID* Sip_Cbk_CreateAckRequest(IN SIP_VOID* pvRespMsg, IN ISipUserData* pUserData)
{
    if (s_objSipStackCallbacks.pfnCreateAckRequest == IMS_NULL)
    {
        return IMS_NULL;
    }

    return (s_objSipStackCallbacks.pfnCreateAckRequest)(pvRespMsg, pUserData);
}

GLOBAL SIP_VOID Sip_Cbk_PreProcessMessageSentByStack(
        IN SIP_VOID* pvSipMsg, IN ISipUserData* pUserData)
{
    if (s_objSipStackCallbacks.pfnPreProcessMessageSentByStack == IMS_NULL)
    {
        return;
    }

    (s_objSipStackCallbacks.pfnPreProcessMessageSentByStack)(pvSipMsg, pUserData);
}

GLOBAL SIP_VOID Sip_Cbk_PostProcessMessageSentByStack(IN SIP_VOID* pvSipMsg, IN SIP_CHAR* pBuffer,
        IN SIP_UINT32 nBufferLen, IN ISipUserData* pUserData)
{
    if (s_objSipStackCallbacks.pfnPostProcessMessageSentByStack == IMS_NULL)
    {
        return;
    }

    (s_objSipStackCallbacks.pfnPostProcessMessageSentByStack)(
            pvSipMsg, pBuffer, nBufferLen, pUserData);
}

GLOBAL SIP_VOID Sip_Cbk_DisplayTxnKey(IN SIP_VOID* pvTxnKey)
{
    if (s_objSipStackCallbacks.pfnDisplayTxnKey == IMS_NULL)
    {
        return;
    }

    (s_objSipStackCallbacks.pfnDisplayTxnKey)(pvTxnKey);
}
