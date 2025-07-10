/*
 * Copyright (C) 2025 The Android Open Source Project
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
#include "SipStackState.h"
#include "SipTransactionCallback.h"
#include "SipTransactionTimer.h"

__IMS_TRACE_TAG_SIP_CORE__;

SIP_BOOL SipTransactionCallback::FetchTransaction(
        IN SipTxnKey* pTxnKey, IN SIP_INT32 nOption, OUT SipTxnKey*& pOutKey, OUT SipTxn*& pOutTxn)
{
    SipStackState* pStackState = SipStackState::GetInstance();
    return pStackState->FetchTransaction(pTxnKey, nOption, pOutKey, pOutTxn) ? SIP_TRUE : SIP_FALSE;
}

SIP_BOOL SipTransactionCallback::ReleaseTransaction(IN SipTxnKey* pInTxnKey, IN SIP_INT32 nOption,
        OUT SipTxnKey*& pOutKey, OUT SipTxn*& pOutTxn)
{
    SipStackState* pStackState = SipStackState::GetInstance();
    return pStackState->ReleaseTransaction(pInTxnKey, nOption, pOutKey, pOutTxn) ? SIP_TRUE
                                                                                 : SIP_FALSE;
}

SipMessage* SipTransactionCallback::CreateAckRequest(
        IN SipMessage* pRespMsg, IN ISipUserData* pUserData)
{
    SipClientTransactionState* pCtState = IMS_NULL;
    SipStack::GetTransactionState(*pUserData, pCtState);

    return (pCtState != IMS_NULL) ? pCtState->CreateAckRequest(pRespMsg) : IMS_NULL;
}

SIP_VOID SipTransactionCallback::PreProcessMessageSentByStack(
        IN SipMessage* pSipMsg, IN ISipUserData* pUserData)
{
    SipTransactionState* pTState = IMS_NULL;
    SipStack::GetTransactionState(*pUserData, pTState);

    if (pTState != IMS_NULL)
    {
        pTState->PreProcessMessageSentByStack(pSipMsg);
    }
}

SIP_VOID SipTransactionCallback::PostProcessMessageSentByStack(IN SipMessage* pSipMsg,
        IN SIP_CHAR* pcBuffer, IN SIP_UINT32 uiBufferLen, IN ISipUserData* pUserData)
{
    SipTransactionState* pTState = IMS_NULL;
    SipStack::GetTransactionState(*pUserData, pTState);

    if (pTState != IMS_NULL)
    {
        ByteArray objBuffer(reinterpret_cast<const IMS_BYTE*>(pcBuffer), uiBufferLen);
        pTState->PostProcessMessageSentByStack(pSipMsg, objBuffer);
    }
}

SIP_VOID SipTransactionCallback::DisplayTxnKey(IN SipTxnKey* pTxnKey)
{
    SipStack::DisplayTxnKey(pTxnKey);
}

SIP_VOID SipTransactionCallback::NotifyTimerExpired(
        IN ISipUserData* pUserData, IN IMS_SINT32 nTimerType)
{
    SipTxnContext* pTxnContext = static_cast<SipTxnContext*>(pUserData->GetUserData());
    SipTransactionState* pTState = IMS_NULL;
    SipStack::GetTransactionState(*pUserData, pTState);

    // Clear user data to avoid double memory free by aborting the transaction
    // when SIP transaction timer is expired.
    pUserData->SetUserData(IMS_NULL);

    if ((nTimerType == SipTxn::TIMER_D) || (nTimerType == SipTxn::TIMER_I) ||
            (nTimerType == SipTxn::TIMER_J) || (nTimerType == SipTxn::TIMER_K) ||
            (nTimerType == SipTxn::TIMER_L) || (nTimerType == SipTxn::TIMER_M))
    {
        // CSM moving from "Completed"/"Accepted" to "Terminated".
        // This is a normal case and ignore these cases.

        if (nTimerType == SipTxn::TIMER_M && pTState != IMS_NULL)
        {
            SipClientTransactionState* pCtState = static_cast<SipClientTransactionState*>(pTState);
            pCtState->ClearAllForkedTransactions();
        }
        SipStack::DestroyTxnContext(pTxnContext);
        return;
    }

    SipTransactionTimer::TimerExpired(nTimerType);

    if (pTState != IMS_NULL)
    {
        pTState->NotifyTimerExpired();
    }

    // Free the event context
    SipStack::DestroyTxnContext(pTxnContext);
}

SIP_VOID* SipTransactionCallback::StartTimer(IN SIP_UINT32 nDuration,
        IN SipTimerCallback pfnTimerCallback, IN SipTimeoutData* pTimeoutData)
{
    SipTransactionTimer* pTimer = new SipTransactionTimer(pTimeoutData, pfnTimerCallback);

    if (!pTimer->Start(nDuration))
    {
        delete pTimer;
        return SIP_NULL;
    }

    return pTimer;
}

SIP_VOID SipTransactionCallback::StopTimer(IN SIP_VOID* pvHandle, OUT SipTimeoutData*& pTimeOutData)
{
    SipTransactionTimer* pTimer = static_cast<SipTransactionTimer*>(pvHandle);

    if (pTimer != IMS_NULL)
    {
        pTimer->Stop(pTimeOutData);
        delete pTimer;
    }
}

SIP_VOID SipTransactionCallback::NotifyTransactionTerminated(ISipUserData* pUserData)
{
    IMS_TRACE_I("TransactionTerminated", 0, 0, 0);

    if (pUserData != SIP_NULL && pUserData->GetDeleteFlag() == SIP_TRUE)
    {
        SipTransactionState* pTState = IMS_NULL;
        SipStack::GetTransactionState(*pUserData, pTState);

        if (pTState != IMS_NULL && pTState->GetType() == SipTransactionState::TYPE_CLIENT)
        {
            SipClientTransactionState* pCtState = static_cast<SipClientTransactionState*>(pTState);
            pCtState->ClearAllForkedTransactions();
        }

        SipTxnContext* pTxnContext = reinterpret_cast<SipTxnContext*>(pUserData->GetUserData());

        if (pTxnContext != SIP_NULL)
        {
            IMS_TRACE_D("Destroy::SipTxnContext", 0, 0, 0);
            SipStack::DestroyTxnContext(pTxnContext);
            pUserData->SetUserData(SIP_NULL);
        }
    }
}
