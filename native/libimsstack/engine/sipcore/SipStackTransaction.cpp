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
#include "ServiceMemory.h"

#include "SipStack.h"
#include "SipStackTransaction.h"

PUBLIC
SipStackTransaction::SipStackTransaction() :
        m_pKey(IMS_NULL),
        m_pTxn(IMS_NULL)
{
}

PUBLIC
SipStackTransaction::SipStackTransaction(IN ::SipTxnKey* pKey, IN SipTxn* pTxn) :
        m_pKey(pKey),
        m_pTxn(pTxn)
{
    SipStack::AddReference(m_pKey);
    SipStack::AddReference(m_pTxn);
}

PUBLIC
SipStackTransaction::SipStackTransaction(IN const SipStackTransaction& other) :
        m_pKey(other.m_pKey),
        m_pTxn(other.m_pTxn)
{
    SipStack::AddReference(m_pKey);
    SipStack::AddReference(m_pTxn);
}

PUBLIC VIRTUAL SipStackTransaction::~SipStackTransaction()
{
    SipStack::FreeTxnKey(m_pKey);
    SipStack::FreeTxn(m_pTxn);
}

PUBLIC
SipStackTransaction& SipStackTransaction::operator=(IN const SipStackTransaction& other)
{
    if (this != &other)
    {
        SipStack::FreeTxnKey(m_pKey);
        SipStack::FreeTxn(m_pTxn);

        m_pKey = other.m_pKey;
        m_pTxn = other.m_pTxn;

        SipStack::AddReference(m_pKey);
        SipStack::AddReference(m_pTxn);
    }

    return (*this);
}

PUBLIC
IMS_BOOL SipStackTransaction::CompareKey(IN ::SipTxnKey* pKey)
{
    return SipStack::CompareTxnKeys(m_pKey, pKey);
}
