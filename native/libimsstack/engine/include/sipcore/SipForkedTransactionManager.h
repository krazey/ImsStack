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
#ifndef SIP_FORKED_TRANSACTION_MANAGER_H_
#define SIP_FORKED_TRANSACTION_MANAGER_H_

#include "IMSList.h"
#include "RCObject.h"

#include "SipStackHeaders.h"
#include "SipStatusCode.h"

class SipClientTransactionState;

class SipForkedTransactionManager : public RCObject
{
public:
    inline SipForkedTransactionManager() :
            RCObject(),
            m_nStatusCode(SipStatusCode::SC_INVALID)
    {
    }
    inline SipForkedTransactionManager(IN const SipForkedTransactionManager& other) :
            RCObject(other),
            m_nStatusCode(other.m_nStatusCode)
    {
    }
    inline virtual ~SipForkedTransactionManager() {}

    SipForkedTransactionManager& operator=(IN const SipForkedTransactionManager&) = delete;

public:
    IMS_BOOL Add(IN SipClientTransactionState* pCtState);
    inline IMS_BOOL IsEmpty() const { return m_objTxnStates.IsEmpty(); }
    inline IMS_BOOL IsTransactionCompleted() const { return SipStatusCode::IsFinal(m_nStatusCode); }
    SipClientTransactionState* Lookup(IN ::SipMessage* pSipMsg) const;
    void Remove(IN SipClientTransactionState* pCtState);
    inline void SetTransactionCompleted(IN IMS_SINT32 nStatusCode) { m_nStatusCode = nStatusCode; }

private:
    // FIX_NO_ACK_RETRANSMISSION :: this will be used for 2xx response received case only
    IMS_SINT32 m_nStatusCode;
    IMSList<RCPtr<SipClientTransactionState>> m_objTxnStates;
};

#endif
