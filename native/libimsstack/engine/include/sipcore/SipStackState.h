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
#ifndef SIP_STACK_STATE_H_
#define SIP_STACK_STATE_H_

#include "ImsList.h"
#include "ImsMap.h"

class IMutex;
class SipProfile;
class SipStackTransaction;
class SipTransactionState;

class SipStackState
{
private:
    SipStackState();

public:
    ~SipStackState();

    SipStackState(IN const SipStackState&) = delete;
    SipStackState& operator=(IN const SipStackState&) = delete;

public:
    void CleanUp();
    void StartUp();

    IMS_BOOL AbortTransaction(IN ::SipTxnKey* pKey, IN SipTransactionState* pTxnState);
    IMS_BOOL FetchTransaction(IN ::SipTxnKey* pKey, IN IMS_SINT32 nOption,
            OUT ::SipTxnKey*& pOutKey, OUT SipTxn*& pTxn);
    IMS_BOOL ReleaseTransaction(IN ::SipTxnKey* pKey, IN IMS_SINT32 nOption,
            OUT ::SipTxnKey*& pOutKey, OUT SipTxn*& pTxn);
    void SetTransactionTimerValues(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile);

    static SipStackState* GetInstance();

private:
    IMS_BOOL AddTransaction(IN ::SipTxnKey* pKey, IN SipTxn* pTxn);
    SipStackTransaction* FindTransaction(IN ::SipTxnKey* pKey);
    SipStackTransaction* RemoveTransaction(IN ::SipTxnKey* pKey, IN IMS_SINT32 nOption);
    IMS_UINT32 GetTransactionCount() const;

public:
    enum
    {
        TXN_FETCH = 0,
        TXN_CREATE = 1,
        TXN_REMOVE = 2
    };

private:
    IMutex* m_piLock;
    IMSMap<IMS_UINT32, IMSList<SipStackTransaction*>> m_objTxnAggregate;
};

#endif
