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
#include "SipStackError.h"
#include "txn/SipTimeoutData.h"
#include "txn/SipTxn.h"

SipTimeoutData::SipTimeoutData()
{
    m_eTxnType = SipTxn::INVALID;
    m_eTimerType = SipTxn::TIMER_TYPE_INVALID;
    m_pTxnKey = SIP_NULL;
}

SipTimeoutData::SipTimeoutData(SIP_INT32 eTxnType, SIP_INT32 eTimerType, const SipTxnKey* pTxnKey)
{
    m_eTxnType = eTxnType;
    m_eTimerType = eTimerType;

    SIP_UINT16 nError;
    m_pTxnKey = new SipTxnKey(pTxnKey, &nError);
    if (E_ERR_PF_MALLOCFAILED == nError)
    {
        m_pTxnKey->SipDelete();
        m_pTxnKey = SIP_NULL;
    }
}

SipTimeoutData::~SipTimeoutData()
{
    m_pTxnKey->SipDelete();
    m_pTxnKey = SIP_NULL;
}

SipTxnKey* SipTimeoutData::GetTxnKey() const
{
    return m_pTxnKey;
}

SIP_INT32 SipTimeoutData::GetTimerType() const
{
    return m_eTimerType;
}
