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
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"

#include "RegFlow.h"
#include "SipDebug.h"
#include "SipFactory.h"

__IMS_TRACE_TAG_REG__;

PUBLIC
RegFlow::RegFlow(IN const RegKey& objRegKey) :
        m_objRegKey(objRegKey),
        m_strCallId(AString::ConstNull()),
        m_nCSeqValue(0),
        m_nSubscriber(NO_SUBSCRIBER),
        m_strSessionId(AString::ConstNull())
{
    SipFactory::GenerateCallId(AString::ConstNull(), m_strCallId);
    SipFactory::GenerateSessionId(objRegKey.GetSlotId(), m_strCallId, m_strSessionId);
}

PUBLIC
RegFlow::RegFlow(IN const RegFlow& other) :
        m_objRegKey(other.m_objRegKey),
        m_strCallId(other.m_strCallId),
        m_nCSeqValue(other.m_nCSeqValue),
        m_nSubscriber(other.m_nSubscriber),
        m_strSessionId(other.m_strSessionId)
{
}

PUBLIC
RegFlow::~RegFlow()
{
    IMS_TRACE_D("Destructor :: %X, %s, %u", m_nSubscriber,
            SipDebug::GetCharA1(m_strCallId.GetStr(), 8, '@'), m_nCSeqValue);
}

PUBLIC
RegFlow& RegFlow::operator=(IN const RegFlow& other)
{
    if (this != &other)
    {
        m_objRegKey = other.m_objRegKey;

        m_strCallId = other.m_strCallId;
        m_nCSeqValue = other.m_nCSeqValue;
        m_nSubscriber = other.m_nSubscriber;
        m_strSessionId = other.m_strSessionId;
    }

    return (*this);
}

PUBLIC
IMS_BOOL RegFlow::Capture(IN IMS_UINT32 nSubscriber /*= DEFAULT_SUSCRIBER*/)
{
    if (m_nSubscriber != NO_SUBSCRIBER)
    {
        return IMS_FALSE;
    }

    m_nSubscriber = nSubscriber;

    return IMS_TRUE;
}

PUBLIC
IMS_SINT32 RegFlow::IncreaseNGetCSeqValue(IN IMS_SINT32 nIncrement /*= 1*/)
{
    if (nIncrement == 0)
    {
        return m_nCSeqValue;
    }

    m_nCSeqValue += nIncrement;

    return m_nCSeqValue;
}

PUBLIC
IMS_BOOL RegFlow::IsReserved(OUT IMS_UINT32* pnSubscriber /*= IMS_NULL*/) const
{
    if (pnSubscriber != IMS_NULL)
    {
        *pnSubscriber = m_nSubscriber;
    }

    return (m_nSubscriber != NO_SUBSCRIBER) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC
void RegFlow::Restore()
{
    SipFactory::GenerateCallId(AString::ConstNull(), m_strCallId);

    m_nSubscriber = NO_SUBSCRIBER;
    m_nCSeqValue = 0;
    SipFactory::GenerateSessionId(m_objRegKey.GetSlotId(), m_strCallId, m_strSessionId);
}

PUBLIC
void RegFlow::UpdateCallId(IN const IpAddress& objIpAddr)
{
    // Check if the Call-ID already contains '@' character
    if (m_strCallId.Contains('@'))
    {
        return;
    }

    if (objIpAddr.IsIPv4Address() || objIpAddr.IsIPv6Address())
    {
        m_strCallId.Append('@');
        m_strCallId.Append(objIpAddr.ToString());
        SipFactory::GenerateSessionId(m_objRegKey.GetSlotId(), m_strCallId, m_strSessionId);
    }
}
