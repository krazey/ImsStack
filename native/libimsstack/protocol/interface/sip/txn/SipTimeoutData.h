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
#ifndef _SIP_TXN_TIMEOUT_DATA_H
#define _SIP_TXN_TIMEOUT_DATA_H

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
    SipTimeoutData(SIP_INT32 eTxnType, SIP_INT32 eTimerType, SipTxnKey* pTxnKey);
    virtual ~SipTimeoutData();

    SipTxnKey* GetTxnKey() const;
    SIP_INT32 GetTimerType() const;
    SIP_BOOL SetTxnKey(SipTxnKey* pTxnKey);
    SIP_BOOL SetTimerType(SIP_INT32 eTimerType);
};

#endif
