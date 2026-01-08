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
#ifndef __INTERFACE_SIP_TRANSACTION_CALLBACK_H__
#define __INTERFACE_SIP_TRANSACTION_CALLBACK_H__

#include "SipDatatypes.h"

class ISipUserData;
class SipMessage;
class SipTimeoutData;
class SipTxn;
class SipTxnKey;

typedef SIP_VOID (*SipTimerCallback)(SIP_VOID* pvData, const SIP_VOID* pvTimerId);

class ISipTransactionCallback
{
protected:
    virtual ~ISipTransactionCallback() = default;

public:
    virtual SIP_BOOL FetchTransaction(IN SipTxnKey* pTxnKey, IN SIP_INT32 nOption,
            OUT SipTxnKey*& pOutTxnKey, OUT SipTxn*& pTxn) = 0;
    virtual SIP_BOOL FetchTransaction(
            IN SipTxnKey* pTxnKey, IN SIP_INT32 nOption, OUT SipTxn*& pTxn) = 0;
    virtual SIP_BOOL ReleaseTransaction(IN SipTxnKey* pTxnKey, IN SIP_INT32 nOption,
            OUT SipTxnKey*& pOutTxnKey, OUT SipTxn*& pTxn) = 0;
    virtual SipMessage* CreateAckRequest(IN SipMessage* pRespMsg, IN ISipUserData* pUserData) = 0;
    virtual SIP_VOID PreProcessMessageSentByStack(
            IN SipMessage* pSipMsg, IN ISipUserData* pUserData) = 0;
    virtual SIP_VOID PostProcessMessageSentByStack(IN SipMessage* pSipMsg, IN SIP_CHAR* pcBuffer,
            IN SIP_UINT32 uiBufferLen, IN ISipUserData* pUserData) = 0;
    virtual SIP_VOID DisplayTxnKey(IN SipTxnKey* pTxnKey) = 0;
    virtual SIP_VOID NotifyTimerExpired(IN ISipUserData* pUserData, IN SIP_INT32 nTimerType) = 0;
    virtual SIP_VOID NotifyTransactionTerminated(IN ISipUserData* pUserData) = 0;
    virtual SIP_VOID* StartTimer(IN SIP_UINT32 nDuration, IN SipTimerCallback pfnTimerCallback,
            IN SipTimeoutData* pData) = 0;
    virtual SIP_VOID StopTimer(IN SIP_VOID* pvHandle, OUT SipTimeoutData*& pTimeoutData) = 0;
};
#endif  //__INTERFACE_SIP_TRANSACTION_CALLBACK_H__
