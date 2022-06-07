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
#ifndef SIP_TXN_CONTEXT_DATA_H_
#define SIP_TXN_CONTEXT_DATA_H_

#include "SipTransactionState.h"

class SipTxnContextData
{
public:
    inline SipTxnContextData() :
            m_pTxnState(IMS_NULL)
    {
    }
    ~SipTxnContextData();

    SipTxnContextData(IN const SipTxnContextData&) = delete;
    SipTxnContextData& operator=(IN const SipTxnContextData&) = delete;

public:
    inline const SipMethod& GetMethod() const { return m_objMethod; }
    inline SipTransactionState* GetTxnState() const { return m_pTxnState.Get(); }
    inline void SetMethod(IN const SipMethod& objMethod) { m_objMethod = objMethod; }
    inline void SetTxnState(IN SipTransactionState* pTxnState) { m_pTxnState = pTxnState; }

private:
    SipMethod m_objMethod;
    RCPtr<SipTransactionState> m_pTxnState;
};

#endif
