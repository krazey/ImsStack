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
#include "AStringArray.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "Sdp.h"
#include "SdpTime.h"
#include "SdpTimeDescription.h"

__IMS_TRACE_TAG_SDP__;

PUBLIC
SdpTimeDescription::SdpTimeDescription() :
        m_pTime(IMS_NULL)
{
}

PUBLIC
SdpTimeDescription::SdpTimeDescription(IN SdpTime* pTime) :
        m_pTime(pTime)
{
}

PUBLIC
SdpTimeDescription::SdpTimeDescription(IN const SdpTimeDescription& other) :
        m_pTime(IMS_NULL),
        m_objRepeatTimes(other.m_objRepeatTimes)
{
    if (other.m_pTime != IMS_NULL)
    {
        m_pTime = new SdpTime(*(other.m_pTime));
    }
}

PUBLIC
SdpTimeDescription::~SdpTimeDescription()
{
    if (m_pTime != IMS_NULL)
    {
        delete m_pTime;
    }

    m_objRepeatTimes.Clear();
}

PUBLIC
SdpTimeDescription& SdpTimeDescription::operator=(IN const SdpTimeDescription& other)
{
    if (this != &other)
    {
        if (m_pTime != IMS_NULL)
        {
            delete m_pTime;
            m_pTime = IMS_NULL;
        }

        if (other.m_pTime != IMS_NULL)
        {
            m_pTime = new SdpTime(*(other.m_pTime));
        }

        m_objRepeatTimes.Clear();
        m_objRepeatTimes = other.m_objRepeatTimes;
    }

    return (*this);
}

PUBLIC
IMS_BOOL SdpTimeDescription::Decode(IN const AStringArray& objLines,
        IN IMS_SINT32 nStartLine /*= 0*/, IN IMS_SINT32 nEndLine /*= -1*/)
{
    // SDP order: t, *(r)

    if (nStartLine < 0)
    {
        return IMS_FALSE;
    }

    if (nEndLine == -1)
    {
        nEndLine = objLines.GetCount();
    }

    AString strLineBody;

    for (IMS_SINT32 i = nStartLine; i < nEndLine; ++i)
    {
        const AString& strLine = objLines.GetElementAt(i);

        strLineBody = strLine.GetSubStr(2);

        if (strLine[0] == Sdp::LINE_R)
        {
            SdpRepeatTime objRepeatTime;

            if (!objRepeatTime.Decode(strLineBody))
            {
                IMS_TRACE_E(0, "SDP decoding failed: r-line (%s)", strLineBody.GetStr(), 0, 0);
                return IMS_FALSE;
            }

            if (!m_objRepeatTimes.Append(objRepeatTime))
            {
                return IMS_FALSE;
            }
        }
    }

    return IMS_TRUE;
}

PUBLIC
AString SdpTimeDescription::Encode() const
{
    // SDP order: t, *(r)

    if (m_pTime == IMS_NULL)
    {
        return AString::ConstEmpty();
    }

    AString strTimeDescription;

    // t=<start-time> <stop-time>
    strTimeDescription.Append(m_pTime->Encode());

    // r=<repeat interval> <active duration> <offsets from start time>
    for (IMS_UINT32 i = 0; i < m_objRepeatTimes.GetSize(); ++i)
    {
        const SdpRepeatTime& objRepeatTime = m_objRepeatTimes.GetAt(i);
        strTimeDescription.Append(objRepeatTime.Encode());
    }

    return strTimeDescription;
}

PUBLIC
IMS_BOOL SdpTimeDescription::AddRepeatTime(IN const SdpRepeatTime& objRepeatTime)
{
    return m_objRepeatTimes.Append(objRepeatTime);
}
