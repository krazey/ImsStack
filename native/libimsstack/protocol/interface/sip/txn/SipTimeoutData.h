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
#ifndef __SIP_TIMEOUT_DATA_H__
#define __SIP_TIMEOUT_DATA_H__

#include "txn/SipTxn.h"
#include "txn/SipTxnKey.h"

/*Timerdata returned at timeout of Timer */
class SipTimeoutData
{
    /* : Txn which started the timer and its type */
    SIP_INT32 m_eTxnType;
    SIP_INT32 m_eTimerType;
    SipTxnKey* m_pTxnKey;

    /*******************************************************
      Private Member Functions
     ********************************************************/
    SipTimeoutData& operator=(IN const SipTimeoutData& objRHS);
    SipTimeoutData(IN const SipTimeoutData& objRHS);

public:
    SipTimeoutData();
    SipTimeoutData(SIP_INT32 eTxnType, SIP_INT32 eTimerType, const SipTxnKey* pTxnKey);
    virtual ~SipTimeoutData();

    SipTxnKey* GetTxnKey() const;
    SIP_INT32 GetTimerType() const;

    inline SIP_VOID SetTxnKey(SipTxnKey* pTxnKey) { this->m_pTxnKey = pTxnKey; }
    inline SIP_VOID SetTimerType(SIP_INT32 eTimerType) { this->m_eTimerType = eTimerType; }
};

#endif  //__SIP_TIMEOUT_DATA_H__
