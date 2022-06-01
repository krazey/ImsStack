/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "SipStackHeaders.h"
#include "SipTxnContextData.h"
#include "SipStackState.h"
#include "SipClientTransactionState.h"
#include "SipTransactionTimer.h"
#include "SipStackTxnLayer.h"

// Implements the function prototypes for SIP stack transaction layer
LOCAL SIP_BOOL SIPStackTxnLayer_FetchTransaction(IN SIP_VOID* pvTxnKey, IN SIP_INT32 nOption,
        OUT SIP_VOID** ppvOutTxnKey, OUT SIP_VOID** ppvTxn)
{
    SipTxnKey* pOutKey = IMS_NULL;
    SipTxn* pOutTxn = IMS_NULL;
    SIPStackState* pStackState = SIPStackState::GetInstance();

    if (ppvOutTxnKey != IMS_NULL)
    {
        pOutKey = static_cast<SipTxnKey*>(*ppvOutTxnKey);
    }

    if (ppvTxn != IMS_NULL)
    {
        pOutTxn = static_cast<SipTxn*>(*ppvTxn);
    }

    if (pStackState->FetchTransaction(static_cast<SipTxnKey*>(pvTxnKey), nOption, pOutKey, pOutTxn))
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
    SipTxnKey* pOutKey = IMS_NULL;
    SipTxn* pOutTxn = IMS_NULL;
    SIPStackState* pStackState = SIPStackState::GetInstance();

    if (ppvOutTxnKey != IMS_NULL)
    {
        pOutKey = static_cast<SipTxnKey*>(*ppvOutTxnKey);
    }

    if (ppvTxn != IMS_NULL)
    {
        pOutTxn = static_cast<SipTxn*>(*ppvTxn);
    }

    if (pStackState->ReleaseTransaction(
                static_cast<SipTxnKey*>(pvInTxnKey), nOption, pOutKey, pOutTxn))
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
    SipTxnContext* pTC = reinterpret_cast<SipTxnContext*>(pUserData->GetUserData());
    SIPTxnContextData* pTCD = (pTC != IMS_NULL)
            ? reinterpret_cast<SIPTxnContextData*>(pTC->pTxnContextData)
            : IMS_NULL;
    SIPClientTransactionState* pCTState = (pTCD != IMS_NULL)
            ? reinterpret_cast<SIPClientTransactionState*>(pTCD->GetTxnState())
            : IMS_NULL;

    if (pCTState == IMS_NULL)
    {
        return IMS_NULL;
    }

    return reinterpret_cast<void*>(
            pCTState->CreateAckRequest(reinterpret_cast<SipMessage*>(pvRespMsg)));
}

LOCAL SIP_VOID SIPStackTxnLayer_PreProcessMessageSentByStack(
        IN SIP_VOID* pvSipMsg, IN ISipUserData* pUserData)
{
    SipTxnContext* pTC = reinterpret_cast<SipTxnContext*>(pUserData->GetUserData());
    SIPTxnContextData* pTCD = (pTC != IMS_NULL)
            ? reinterpret_cast<SIPTxnContextData*>(pTC->pTxnContextData)
            : IMS_NULL;
    SIPTransactionState* pTState = (pTCD != IMS_NULL) ? pTCD->GetTxnState() : IMS_NULL;

    if (pTState == IMS_NULL)
    {
        return;
    }

    pTState->PreProcessMessageSentByStack(reinterpret_cast<SipMessage*>(pvSipMsg));
}

LOCAL SIP_VOID SIPStackTxnLayer_PostProcessMessageSentByStack(IN SIP_VOID* pvSipMsg,
        IN SIP_CHAR* pcBuffer, IN SIP_UINT32 uiBufferLen, IN ISipUserData* pUserData)
{
    SipTxnContext* pTC = reinterpret_cast<SipTxnContext*>(pUserData->GetUserData());
    SIPTxnContextData* pTCD = (pTC != IMS_NULL)
            ? reinterpret_cast<SIPTxnContextData*>(pTC->pTxnContextData)
            : IMS_NULL;
    SIPTransactionState* pTState = (pTCD != IMS_NULL) ? pTCD->GetTxnState() : IMS_NULL;

    if (pTState == IMS_NULL)
    {
        return;
    }

    ByteArray objBuffer(reinterpret_cast<const IMS_BYTE*>(pcBuffer), uiBufferLen);

    pTState->PostProcessMessageSentByStack(reinterpret_cast<SipMessage*>(pvSipMsg), objBuffer);
}

LOCAL SIP_VOID SIPStackTxnLayer_DisplayTxnKey(IN SIP_VOID* pvTxnKey)
{
    SIPStack::DisplayTxnKey((SipTxnKey*)pvTxnKey);
}

LOCAL SIP_VOID SIPStackTxnLayer_OnTimerExpired(IN ISipUserData* pUserData, IN IMS_SINT32 nTimerType)
{
    SipEn_TimerType enTimerType = static_cast<SipEn_TimerType>(nTimerType);
    /* TimerE--> non-INV retx
       ETXN_TIMERA --> INV retx
       ETXN_TIMERG --> INV response retx
    */
    SipTxnContext* pTC = reinterpret_cast<SipTxnContext*>(pUserData->GetUserData());

    // Clear user data to avoid double memory free by aborting the transaction
    // when SIP transaction timer is expired.
    pUserData->SetUserData(IMS_NULL);

    if ((enTimerType == ETXN_TIMERD) || (enTimerType == ETXN_TIMERI) ||
            (enTimerType == ETXN_TIMERJ) || (enTimerType == ETXN_TIMERK))
    {
        // CSM moving from "Completed" to "Terminated".
        // This is a normal case and ignore these cases.
        SIPStack::DestroyTxnContext(pTC);
        return;
    }

    SIPTransactionTimer::TimerExpired(enTimerType);

    if (pTC != IMS_NULL)
    {
        SIPTxnContextData* pTxnContextData = (SIPTxnContextData*)pTC->pTxnContextData;

        if (pTxnContextData != IMS_NULL)
        {
            SIPTransactionState* pTState = pTxnContextData->GetTxnState();

            if (pTState != IMS_NULL)
            {
                pTState->NotifyTimerExpired();
            }
        }
    }

    // Free the event context
    SIPStack::DestroyTxnContext(pTC);
}

LOCAL SIP_BOOL SIPStackTxnLayer_StartTimer(IN SIP_UINT32 nDuration,
        IN SipTimerCallback pfnTimerCallback, IN SIP_VOID* pvData, OUT SIP_VOID** ppvHandle)
{
    SipTimeoutData* pTimeoutData = reinterpret_cast<SipTimeoutData*>(pvData);
    SIPTransactionTimer* pTimer = new SIPTransactionTimer(pTimeoutData, pfnTimerCallback);

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
    SIPTransactionTimer* pTimer = static_cast<SIPTransactionTimer*>(pvHandle);

    if (pTimer == IMS_NULL)
    {
        return SIP_FALSE;
    }

    pTimer->Stop(reinterpret_cast<SipTimeoutData*&>(*ppvData));

    delete pTimer;

    return SIP_TRUE;
}

// Initialization function for SIP transaction layer
GLOBAL void SIPStackTxnLayer_Initialize()
{
    static const SIPStackCallbacks stCallbacks = {&SIPStackTxnLayer_FetchTransaction,
            &SIPStackTxnLayer_ReleaseTransaction, &SIPStackTxnLayer_StartTimer,
            &SIPStackTxnLayer_StopTimer, &SIPStackTxnLayer_OnTimerExpired,
            &SIPStackTxnLayer_CreateAckRequest, &SIPStackTxnLayer_PreProcessMessageSentByStack,
            &SIPStackTxnLayer_PostProcessMessageSentByStack, &SIPStackTxnLayer_DisplayTxnKey};

    //---------------------------------------------------------------------------------------------

    SIPStackCallback_SetCallbacks(stCallbacks);
}
