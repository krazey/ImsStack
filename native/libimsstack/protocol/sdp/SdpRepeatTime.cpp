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
#include "ServiceTrace.h"
#include "TextParser.h"

#include "Sdp.h"
#include "SdpRepeatTime.h"

__IMS_TRACE_TAG_SDP__;

PUBLIC
SdpRepeatTime::SdpRepeatTime() :
        SdpLine(),
        m_nRepeatInterval(0),
        m_nActiveDuration(0),
        m_nFirstOffset(0)
{
}

PUBLIC
SdpRepeatTime::SdpRepeatTime(IN const SdpRepeatTime& other) :
        SdpLine(other),
        m_nRepeatInterval(other.m_nRepeatInterval),
        m_nActiveDuration(other.m_nActiveDuration),
        m_nFirstOffset(other.m_nFirstOffset),
        m_objAdditionalOffsets(other.m_objAdditionalOffsets)
{
}

PUBLIC VIRTUAL SdpRepeatTime::~SdpRepeatTime() {}

PUBLIC
SdpRepeatTime& SdpRepeatTime::operator=(IN const SdpRepeatTime& other)
{
    if (this != &other)
    {
        SdpLine::operator=(other);

        m_nRepeatInterval = other.m_nRepeatInterval;
        m_nActiveDuration = other.m_nActiveDuration;
        m_nFirstOffset = other.m_nFirstOffset;

        m_objAdditionalOffsets.Clear();
        m_objAdditionalOffsets = other.m_objAdditionalOffsets;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpRepeatTime::Decode(IN const AString& strValue)
{
    // r=<repeat interval> <active duration> <offsets from start-time>
    AStringArray objTokens;

    if (!Sdp::SplitLine(strValue, 3, objTokens))
    {
        IMS_TRACE_E(0, "r-line is malformed: %s", strValue.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    // repeat interval field
    const AString& strRepeatInterval = objTokens.GetElementAt(0);

    if ((strRepeatInterval[0] < 0x31) || (strRepeatInterval[0] > 0x39))
    {
        IMS_TRACE_E(0, "r-line is malformed: %s", strValue.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    if (!Sdp::IsTypedTimeString(strRepeatInterval))
    {
        IMS_TRACE_E(0, "r-line is malformed: %s", strValue.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    m_nRepeatInterval = Sdp::ConvertTypedTimeToSeconds(strRepeatInterval);

    // active duration field
    if (!Sdp::IsTypedTimeString(objTokens.GetElementAt(1)))
    {
        IMS_TRACE_E(0, "Invalid active duration in r-line: %s", strValue.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    m_nActiveDuration = Sdp::ConvertTypedTimeToSeconds(objTokens.GetElementAt(1));

    const AString& strOffsets = objTokens.GetElementAt(2);
    ImsList<AString> objOffsets = strOffsets.Split(TextParser::CHAR_SP);

    if (objOffsets.IsEmpty())
    {
        IMS_TRACE_E(0, "Offset is not present in r-line: %s", strValue.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    const AString& strFirstOffset = objOffsets.GetAt(0);

    // offsets from start time field
    if (!Sdp::IsTypedTimeString(strFirstOffset))
    {
        IMS_TRACE_E(0, "Invalid first offset in r-line: %s", strValue.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    m_nFirstOffset = Sdp::ConvertTypedTimeToSeconds(strFirstOffset);

    m_objAdditionalOffsets.Clear();

    if (objOffsets.GetSize() > 1)
    {
        for (IMS_UINT32 i = 1; i < objOffsets.GetSize(); ++i)
        {
            const AString& strExtraOffset = objOffsets.GetAt(i);

            if (!Sdp::IsTypedTimeString(strExtraOffset))
            {
                IMS_TRACE_E(0, "Invalid extra offset in r-line: %s", strValue.GetStr(), 0, 0);
                return IMS_FALSE;
            }

            m_objAdditionalOffsets.Append(Sdp::ConvertTypedTimeToSeconds(strExtraOffset));
        }
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpRepeatTime::Encode() const
{
    if (!IsValid())
    {
        return AString::ConstNull();
    }

    // r=<repeat interval> <active duration> <offsets from start-time>
    AString strLine(1, Sdp::LINE_R);

    strLine.Append(TextParser::CHAR_EQUAL);
    strLine.Append(GetValue());
    strLine.Append(TextParser::CHAR_CR);
    strLine.Append(TextParser::CHAR_LF);

    return strLine;
}

PUBLIC VIRTUAL AString SdpRepeatTime::GetValue() const
{
    if (!IsValid())
    {
        return AString::ConstNull();
    }

    AString strValue;
    AString strOffset;

    strValue.Sprintf("%u %u %u", m_nRepeatInterval, m_nActiveDuration, m_nFirstOffset);

    for (IMS_UINT32 i = 0; i < m_objAdditionalOffsets.GetSize(); ++i)
    {
        strOffset.SetNumber(m_objAdditionalOffsets.GetAt(i));

        strValue.Append(TextParser::CHAR_SP);
        strValue.Append(strOffset);
    }

    return strValue;
}

PUBLIC
IMS_BOOL SdpRepeatTime::SetValue(IN IMS_UINT32 nInterval, IN IMS_UINT32 nActiveDuration,
        IN IMS_UINT32 nFirstOffset, IN const ImsList<IMS_UINT32>& objAdditionalOffsets)
{
    if (nInterval == 0)
    {
        return IMS_FALSE;
    }

    m_nRepeatInterval = nInterval;
    m_nActiveDuration = nActiveDuration;
    m_nFirstOffset = nFirstOffset;
    m_objAdditionalOffsets = objAdditionalOffsets;

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SdpRepeatTime::IsValid() const
{
    return m_nRepeatInterval != 0 && m_nActiveDuration != 0;
}
