#include "IMSTypeDef.h"
#include "SIPStackCallback.h"

static SIPStackCallbacks gstSIPStackCallbacks =
{
    IMS_NULL, // SIPStack_FetchTransaction
    IMS_NULL, // SIPStack_ReleaseTransaction
    IMS_NULL, // SIPStack_StartTimer
    IMS_NULL, // SIPStack_StopTimer
    IMS_NULL, // SIPStack_OnTimerExpired
    IMS_NULL, // SIPStack_CreateAckRequest
    IMS_NULL, // SIPStack_PreProcessMessageSentByStack
    IMS_NULL, // SIPStack_PostProcessMessageSentByStack
    IMS_NULL // SIPStack_DisplayTxnKey
};

GLOBAL void SIPStackCallback_SetCallbacks(IN const SIPStackCallbacks& objCallbacks)
{
    //---------------------------------------------------------------------------------------------

    gstSIPStackCallbacks = objCallbacks;
}

// Implements the function prototypes for SIP stack transaction layer
GLOBAL SIP_BOOL sip_cbk_fetchTransaction(IN SIP_VOID* pvTxnKey,
        IN SIP_INT32 nOption, OUT SIP_VOID** ppvOutTxnKey, OUT SIP_VOID** ppvTxn)
{
    if (gstSIPStackCallbacks.pfnFetchTransaction == IMS_NULL)
    {
        return SIP_FALSE;
    }

    return (gstSIPStackCallbacks.pfnFetchTransaction)(pvTxnKey, nOption, ppvOutTxnKey, ppvTxn);
}

GLOBAL SIP_BOOL sip_cbk_releaseTransaction(IN SIP_VOID* pvTxnKey,
        IN SIP_INT32 nOption, OUT SIP_VOID** ppvOutTxnKey, OUT SIP_VOID** ppvTxn)
{
    if (gstSIPStackCallbacks.pfnReleaseTransaction == IMS_NULL)
    {
        return SIP_FALSE;
    }

    return (gstSIPStackCallbacks.pfnReleaseTransaction)(pvTxnKey, nOption, ppvOutTxnKey, ppvTxn);
}

GLOBAL SIP_BOOL sip_cbk_startTimer(IN SIP_UINT32 nDuration,
        IN SipTimerCallback pfnTimerCallback, IN SIP_VOID* pvData, IN SIP_VOID** ppvHandle)
{
    if (gstSIPStackCallbacks.pfnStartTimer == IMS_NULL)
    {
        return SIP_FALSE;
    }

    return (gstSIPStackCallbacks.pfnStartTimer)(nDuration, pfnTimerCallback, pvData, ppvHandle);
}

GLOBAL SIP_BOOL sip_cbk_stopTimer(IN SIP_VOID* pvHandle, IN SIP_VOID** ppvData)
{
    if (gstSIPStackCallbacks.pfnStopTimer == IMS_NULL)
    {
        return SIP_FALSE;
    }

    return (gstSIPStackCallbacks.pfnStopTimer)(pvHandle, ppvData);
}

GLOBAL SIP_VOID sip_cbk_onTimerExpired(IN ISipUserData* pUserData,
        IN SIP_INT32 enTimerType)
{
    if (gstSIPStackCallbacks.pfnOnTimerExpired == IMS_NULL)
    {
        return;
    }

    return (gstSIPStackCallbacks.pfnOnTimerExpired)(pUserData, enTimerType);
}

GLOBAL SIP_VOID* sip_cbk_createAckRequest(IN SIP_VOID* pvRespMsg,
        IN ISipUserData* pUserData)
{
    if (gstSIPStackCallbacks.pfnCreateAckRequest == IMS_NULL)
    {
        return IMS_NULL;
    }

    return (gstSIPStackCallbacks.pfnCreateAckRequest)(pvRespMsg, pUserData);
}

GLOBAL SIP_VOID sip_cbk_preProcessMessageSentByStack(IN SIP_VOID* pvSipMsg,
        IN ISipUserData* pUserData)
{
    if (gstSIPStackCallbacks.pfnPreProcessMessageSentByStack == IMS_NULL)
    {
        return;
    }

    (gstSIPStackCallbacks.pfnPreProcessMessageSentByStack)(pvSipMsg, pUserData);
}

GLOBAL SIP_VOID sip_cbk_postProcessMessageSentByStack(IN SIP_VOID* pvSipMsg,
        IN SIP_CHAR* pBuffer, IN SIP_UINT32 nBufferLen, IN ISipUserData* pUserData)
{
    if (gstSIPStackCallbacks.pfnPostProcessMessageSentByStack == IMS_NULL)
    {
        return;
    }

    (gstSIPStackCallbacks.pfnPostProcessMessageSentByStack)(pvSipMsg,
            pBuffer, nBufferLen, pUserData);
}

GLOBAL SIP_VOID sip_cbk_displayTxnKey(IN SIP_VOID* pvTxnKey)
{
    if (gstSIPStackCallbacks.pfnDisplayTxnKey == IMS_NULL)
    {
        return;
    }

    (gstSIPStackCallbacks.pfnDisplayTxnKey)(pvTxnKey);
}
