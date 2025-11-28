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
#include "TextParser.h"

#include "Sdp.h"
#include "SdpTime.h"

PUBLIC
SdpTime::SdpTime() :
        SdpLine(),
        m_nStartTime(0),
        m_nStopTime(0)
{
}

PUBLIC
SdpTime::SdpTime(IN const SdpTime& other) :
        SdpLine(other),
        m_nStartTime(other.m_nStartTime),
        m_nStopTime(other.m_nStopTime)
{
}

PUBLIC VIRTUAL SdpTime::~SdpTime() {}

PUBLIC
SdpTime& SdpTime::operator=(IN const SdpTime& other)
{
    if (this != &other)
    {
        SdpLine::operator=(other);

        m_nStartTime = other.m_nStartTime;
        m_nStopTime = other.m_nStopTime;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpTime::Decode(IN const AString& strValue)
{
    // t=<start-time> <stop-time>
    AStringArray objTokens;

    if (!Sdp::SplitLine(strValue, 2, objTokens))
    {
        // Invalid time line
        return IMS_FALSE;
    }

    // Check start-time field
    if (!Sdp::IsDigitString(objTokens.GetElementAt(0)))
    {
        // Invalid start-time field
        return IMS_FALSE;
    }

    // Check stop-time field
    if (!Sdp::IsDigitString(objTokens.GetElementAt(1)))
    {
        // Invalid stop-time field
        return IMS_FALSE;
    }

    m_nStartTime = objTokens.GetElementAt(0).ToUInt32();
    m_nStopTime = objTokens.GetElementAt(1).ToUInt32();

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpTime::Encode() const
{
    // t=<start-time> <stop-time>
    AString strLine(1, Sdp::LINE_T);

    strLine.Append(TextParser::CHAR_EQUAL);
    strLine.Append(GetValue());
    strLine.Append(TextParser::CHAR_CR);
    strLine.Append(TextParser::CHAR_LF);

    return strLine;
}

PUBLIC VIRTUAL AString SdpTime::GetValue() const
{
    AString strValue;

    return strValue.Sprintf("%u %u", m_nStartTime, m_nStopTime);
}

PUBLIC
IMS_BOOL SdpTime::SetValue(IN IMS_UINT32 nStartTime /*= 0*/, IN IMS_UINT32 nStopTime /*= 0*/)
{
    m_nStartTime = nStartTime;
    m_nStopTime = nStopTime;

    return IMS_TRUE;
}
