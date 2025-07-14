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

#ifndef MOCK_I_SIP_TRANSACTION_CALLBACK_H_
#define MOCK_I_SIP_TRANSACTION_CALLBACK_H_

#include <gmock/gmock.h>

#include "ISipTransactionCallback.h"
#include "txn/SipTxn.h"

class MockISipTransactionCallback : public ISipTransactionCallback
{
public:
    MOCK_METHOD(SIP_BOOL, FetchTransaction,
            (IN SipTxnKey * pTxnKey, IN SIP_INT32 nOption, OUT SipTxnKey*& pOutTxnKey,
                    OUT SipTxn*& pTxn),
            (override));
    MOCK_METHOD(SIP_BOOL, FetchTransaction,
            (IN SipTxnKey * pTxnKey, IN SIP_INT32 nOption, OUT SipTxn*& pTxn), (override));
    MOCK_METHOD(SIP_BOOL, ReleaseTransaction,
            (IN SipTxnKey * pTxnKey, IN SIP_INT32 nOption, OUT SipTxnKey*& pOutTxnKey,
                    OUT SipTxn*& pTxn),
            (override));
    MOCK_METHOD(SipMessage*, CreateAckRequest,
            (IN SipMessage * pRespMsg, IN ISipUserData* pUserData), (override));
    MOCK_METHOD(SIP_VOID, PreProcessMessageSentByStack,
            (IN SipMessage * pSipMsg, IN ISipUserData* pUserData), (override));
    MOCK_METHOD(SIP_VOID, PostProcessMessageSentByStack,
            (IN SipMessage * pSipMsg, IN SIP_CHAR* pcBuffer, IN SIP_UINT32 uiBufferLen,
                    IN ISipUserData* pUserData),
            (override));
    MOCK_METHOD(SIP_VOID, DisplayTxnKey, (IN SipTxnKey * pTxnKey), (override));
    MOCK_METHOD(SIP_VOID, NotifyTimerExpired,
            (IN ISipUserData * pUserData, IN SIP_INT32 nTimerType), (override));
    MOCK_METHOD(SIP_VOID, NotifyTransactionTerminated, (IN ISipUserData * pUserData), (override));
    MOCK_METHOD(SIP_VOID*, StartTimer,
            (IN SIP_UINT32 nDuration, IN SipTimerCallback pfnTimerCallback,
                    IN SipTimeoutData* pData),
            (override));
    MOCK_METHOD(SIP_VOID, StopTimer, (IN SIP_VOID * pvHandle, OUT SipTimeoutData*& pTimeoutData),
            (override));
};

#endif  // MOCK_I_SIP_TRANSACTION_CALLBACK_H_
