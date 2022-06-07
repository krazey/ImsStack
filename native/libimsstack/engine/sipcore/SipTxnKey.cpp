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

#include "SipTxnKey.h"

namespace sipcore
{

PUBLIC
SipTxnKey::SipTxnKey() :
        m_objMethod(SipMethod::INVALID),
        m_nStatusCode(0),
        m_strViaBranch(AString::ConstNull()),
        m_nCSeq(0),
        m_nExtraInt(0),
        m_strExtraString(AString::ConstNull())
{
}

PUBLIC
SipTxnKey::SipTxnKey(IN const SipMethod& objMethod, IN IMS_SINT32 nStatusCode,
        IN const AString& strViaBranch, IN IMS_UINT32 nCSeq) :
        m_objMethod(objMethod),
        m_nStatusCode(nStatusCode),
        m_strViaBranch(strViaBranch),
        m_nCSeq(nCSeq),
        m_nExtraInt(0),
        m_strExtraString(AString::ConstNull())
{
}

PUBLIC
SipTxnKey::SipTxnKey(IN const SipTxnKey& other) :
        m_objMethod(other.m_objMethod),
        m_nStatusCode(other.m_nStatusCode),
        m_strViaBranch(other.m_strViaBranch),
        m_nCSeq(other.m_nCSeq),
        m_nExtraInt(other.m_nExtraInt),
        m_strExtraString(other.m_strExtraString)
{
}

PUBLIC
SipTxnKey& SipTxnKey::operator=(IN const SipTxnKey& other)
{
    if (this != &other)
    {
        m_objMethod = other.m_objMethod;
        m_nStatusCode = other.m_nStatusCode;
        m_strViaBranch = other.m_strViaBranch;
        m_nCSeq = other.m_nCSeq;

        m_nExtraInt = other.m_nExtraInt;
        m_strExtraString = other.m_strExtraString;
    }

    return (*this);
}

PUBLIC
IMS_BOOL SipTxnKey::Equals(IN const SipTxnKey* pKey) const
{
    if (pKey == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!m_objMethod.Equals(pKey->m_objMethod) || (m_nCSeq != pKey->m_nCSeq) ||
            !m_strViaBranch.Equals(pKey->m_strViaBranch))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

}  // namespace sipcore
