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

#include "SIPStackHeaders.h"
#include "SIPStackTransaction.h"

PUBLIC
SIPStackTransaction::SIPStackTransaction() :
        m_pKey(IMS_NULL),
        m_pTxn(IMS_NULL)
{
}

PUBLIC
SIPStackTransaction::SIPStackTransaction(IN SipTxnKey* pKey, IN SipTxn* pTxn) :
        m_pKey(pKey),
        m_pTxn(pTxn)
{
    SIPStack::AddReference(m_pKey);
    SIPStack::AddReference(m_pTxn);
}

PUBLIC
SIPStackTransaction::SIPStackTransaction(IN const SIPStackTransaction& other) :
        m_pKey(other.m_pKey),
        m_pTxn(other.m_pTxn)
{
    SIPStack::AddReference(m_pKey);
    SIPStack::AddReference(m_pTxn);
}

PUBLIC VIRTUAL SIPStackTransaction::~SIPStackTransaction()
{
    SIPStack::FreeTxnKey(m_pKey);
    SIPStack::FreeTxn(m_pTxn);
}

PUBLIC
SIPStackTransaction& SIPStackTransaction::operator=(IN const SIPStackTransaction& other)
{
    if (this != &other)
    {
        SIPStack::FreeTxnKey(m_pKey);
        SIPStack::FreeTxn(m_pTxn);

        m_pKey = other.m_pKey;
        m_pTxn = other.m_pTxn;

        SIPStack::AddReference(m_pKey);
        SIPStack::AddReference(m_pTxn);
    }

    return (*this);
}

PUBLIC
IMS_BOOL SIPStackTransaction::CompareKey(IN SipTxnKey* pKey)
{
    return SIPStack::CompareTxnKeys(m_pKey, pKey);
}
