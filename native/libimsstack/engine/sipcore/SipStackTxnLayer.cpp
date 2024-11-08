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
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "SipClientTransactionState.h"
#include "SipStack.h"
#include "SipStackCallback.h"
#include "SipStackState.h"
#include "SipStackTxnLayer.h"
#include "SipTransactionTimer.h"
#include "SipTxnContextData.h"

// Implements the function prototypes for SIP stack transaction layer
LOCAL SIP_BOOL SIPStackTxnLayer_FetchTransaction(IN SIP_VOID* pvTxnKey, IN SIP_INT32 nOption,
        OUT SIP_VOID** ppvOutTxnKey, OUT SIP_VOID** ppvTxn)
{
    ::SipTxnKey* pOutKey = IMS_NULL;
    SipTxn* pOutTxn = IMS_NULL;
    SipStackState* pStackState = SipStackState::GetInstance();

    if (ppvOutTxnKey != IMS_NULL)
    {
        pOutKey = static_cast<::SipTxnKey*>(*ppvOutTxnKey);
    }

    if (ppvTxn != IMS_NULL)
    {
        pOutTxn = static_cast<SipTxn*>(*ppvTxn);
    }

    if (pStackState->FetchTransaction(
                static_cast<::SipTxnKey*>(pvTxnKey), nOption, pOutKey, pOutTxn))
    {
        if (ppvOutTxnKey != IMS_NULL)
        {
            (*ppvOutTxnKey) = pOutKey;
        }

        if (ppvTxn != IMS_NULL)
        {
            (*ppvTxn) = pOutTxn;
        }

        return SIP_TRUE;
    }

    return SIP_FALSE;
}

LOCAL SIP_BOOL SIPStackTxnLayer_ReleaseTransaction(IN SIP_VOID* pvInTxnKey, IN SIP_INT32 nOption,
        OUT SIP_VOID** ppvOutTxnKey, OUT SIP_VOID** ppvTxn)
{
    ::SipTxnKey* pOutKey = IMS_NULL;
    SipTxn* pOutTxn = IMS_NULL;
    SipStackState* pStackState = SipStackState::GetInstance();

    if (ppvOutTxnKey != IMS_NULL)
    {
        pOutKey = static_cast<::SipTxnKey*>(*ppvOutTxnKey);
    }

    if (ppvTxn != IMS_NULL)
    {
        pOutTxn = static_cast<SipTxn*>(*ppvTxn);
    }

    if (pStackState->ReleaseTransaction(
                static_cast<::SipTxnKey*>(pvInTxnKey), nOption, pOutKey, pOutTxn))
    {
        if (ppvOutTxnKey != IMS_NULL)
        {
            (*ppvOutTxnKey) = pOutKey;
        }

        if (ppvTxn != IMS_NULL)
        {
            (*ppvTxn) = pOutTxn;
        }

        return SIP_TRUE;
    }

    return SIP_FALSE;
}

LOCAL SIP_VOID* SIPStackTxnLayer_CreateAckRequest(
        IN SIP_VOID* pvRespMsg, IN ISipUserData* pUserData)
{
    SipTxnContext* pTxnContext = reinterpret_cast<SipTxnContext*>(pUserData->GetUserData());
    SipTxnContextData* pTxnContextData = (pTxnContext != IMS_NULL)
            ? reinterpret_cast<SipTxnContextData*>(pTxnContext->m_pTxnContextData)
            : IMS_NULL;
    SipClientTransactionState* pCtState = (pTxnContextData != IMS_NULL)
            ? reinterpret_cast<SipClientTransactionState*>(pTxnContextData->GetTxnState())
            : IMS_NULL;

    if (pCtState == IMS_NULL)
    {
        return IMS_NULL;
    }

    return reinterpret_cast<void*>(
            pCtState->CreateAckRequest(reinterpret_cast<::SipMessage*>(pvRespMsg)));
}

LOCAL SIP_VOID SIPStackTxnLayer_PreProcessMessageSentByStack(
        IN SIP_VOID* pvSipMsg, IN ISipUserData* pUserData)
{
    SipTxnContext* pTxnContext = reinterpret_cast<SipTxnContext*>(pUserData->GetUserData());
    SipTxnContextData* pTxnContextData = (pTxnContext != IMS_NULL)
            ? reinterpret_cast<SipTxnContextData*>(pTxnContext->m_pTxnContextData)
            : IMS_NULL;
    SipTransactionState* pTState =
            (pTxnContextData != IMS_NULL) ? pTxnContextData->GetTxnState() : IMS_NULL;

    if (pTState == IMS_NULL)
    {
        return;
    }

    pTState->PreProcessMessageSentByStack(reinterpret_cast<::SipMessage*>(pvSipMsg));
}

LOCAL SIP_VOID SIPStackTxnLayer_PostProcessMessageSentByStack(IN SIP_VOID* pvSipMsg,
        IN SIP_CHAR* pcBuffer, IN SIP_UINT32 uiBufferLen, IN ISipUserData* pUserData)
{
    SipTxnContext* pTxnContext = reinterpret_cast<SipTxnContext*>(pUserData->GetUserData());
    SipTxnContextData* pTxnContextData = (pTxnContext != IMS_NULL)
            ? reinterpret_cast<SipTxnContextData*>(pTxnContext->m_pTxnContextData)
            : IMS_NULL;
    SipTransactionState* pTState =
            (pTxnContextData != IMS_NULL) ? pTxnContextData->GetTxnState() : IMS_NULL;

    if (pTState == IMS_NULL)
    {
        return;
    }

    ByteArray objBuffer(reinterpret_cast<const IMS_BYTE*>(pcBuffer), uiBufferLen);

    pTState->PostProcessMessageSentByStack(reinterpret_cast<::SipMessage*>(pvSipMsg), objBuffer);
}

LOCAL SIP_VOID SIPStackTxnLayer_DisplayTxnKey(IN SIP_VOID* pvTxnKey)
{
    SipStack::DisplayTxnKey((::SipTxnKey*)pvTxnKey);
}

LOCAL SIP_VOID SIPStackTxnLayer_OnTimerExpired(IN ISipUserData* pUserData, IN IMS_SINT32 nTimerType)
{
    SipTxnContext* pTxnContext = static_cast<SipTxnContext*>(pUserData->GetUserData());

    // Clear user data to avoid double memory free by aborting the transaction
    // when SIP transaction timer is expired.
    pUserData->SetUserData(IMS_NULL);

    if ((nTimerType == SipTxn::TIMER_D) || (nTimerType == SipTxn::TIMER_I) ||
            (nTimerType == SipTxn::TIMER_J) || (nTimerType == SipTxn::TIMER_K))
    {
        // CSM moving from "Completed" to "Terminated".
        // This is a normal case and ignore these cases.
        SipStack::DestroyTxnContext(pTxnContext);
        return;
    }

    SipTransactionTimer::TimerExpired(nTimerType);

    if (pTxnContext != IMS_NULL)
    {
        SipTxnContextData* pTxnContextData =
                static_cast<SipTxnContextData*>(pTxnContext->m_pTxnContextData);

        if (pTxnContextData != IMS_NULL)
        {
            SipTransactionState* pTState = pTxnContextData->GetTxnState();

            if (pTState != IMS_NULL)
            {
                pTState->NotifyTimerExpired();
            }
        }
    }

    // Free the event context
    SipStack::DestroyTxnContext(pTxnContext);
}

LOCAL SIP_BOOL SIPStackTxnLayer_StartTimer(IN SIP_UINT32 nDuration,
        IN SipTimerCallback pfnTimerCallback, IN SIP_VOID* pvData, OUT SIP_VOID** ppvHandle)
{
    SipTimeoutData* pTimeoutData = reinterpret_cast<SipTimeoutData*>(pvData);
    SipTransactionTimer* pTimer = new SipTransactionTimer(pTimeoutData, pfnTimerCallback);

    if (pTimer == IMS_NULL)
    {
        return SIP_FALSE;
    }

    if (!pTimer->Start(nDuration))
    {
        return SIP_FALSE;
    }

    (*ppvHandle) = pTimer;

    return SIP_TRUE;
}

LOCAL SIP_BOOL SIPStackTxnLayer_StopTimer(IN SIP_VOID* pvHandle, OUT SIP_VOID** ppvData)
{
    SipTransactionTimer* pTimer = static_cast<SipTransactionTimer*>(pvHandle);

    if (pTimer == IMS_NULL)
    {
        return SIP_FALSE;
    }

    pTimer->Stop(reinterpret_cast<SipTimeoutData*&>(*ppvData));

    delete pTimer;

    return SIP_TRUE;
}

// Initialization function for SIP transaction layer
GLOBAL void SipStackTxnLayer_Initialize()
{
    // clang-format off
    static const SipStackCallbacks stCallbacks = {
            &SIPStackTxnLayer_FetchTransaction,
            &SIPStackTxnLayer_ReleaseTransaction,
            &SIPStackTxnLayer_StartTimer,
            &SIPStackTxnLayer_StopTimer,
            &SIPStackTxnLayer_OnTimerExpired,
            &SIPStackTxnLayer_CreateAckRequest,
            &SIPStackTxnLayer_PreProcessMessageSentByStack,
            &SIPStackTxnLayer_PostProcessMessageSentByStack,
            &SIPStackTxnLayer_DisplayTxnKey
        };
    // clang-format on

    SipStackCallback_SetCallbacks(stCallbacks);
}
