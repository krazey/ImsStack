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
#include "AString.h"
#include "TextParser.h"

#include "Sdp.h"
#include "SdpTimezone.h"

PUBLIC
SdpTimezone::SdpTimezone() :
        SdpLine()
{
}

PUBLIC
SdpTimezone::SdpTimezone(IN const SdpTimezone& other) :
        SdpLine(other),
        m_objZoneAdjustments(other.m_objZoneAdjustments)
{
}

PUBLIC VIRTUAL SdpTimezone::~SdpTimezone()
{
    m_objZoneAdjustments.Clear();
}

PUBLIC
SdpTimezone& SdpTimezone::operator=(IN const SdpTimezone& other)
{
    if (this != &other)
    {
        SdpLine::operator=(other);

        m_objZoneAdjustments.Clear();
        m_objZoneAdjustments = other.m_objZoneAdjustments;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpTimezone::Decode(IN const AString& strValue)
{
    // z=<adjustment time> <offset> <adjustment time> <offset> ...
    ImsList<AString> objTokens = strValue.Split(TextParser::CHAR_SP);

    if (objTokens.GetSize() < 2)
    {
        // Invalid line
        return IMS_FALSE;
    }

    if (objTokens.GetSize() % 2 != 0)
    {
        // Invalid line
        return IMS_FALSE;
    }

    m_objZoneAdjustments.Clear();

    for (IMS_UINT32 i = 0; i < objTokens.GetSize(); i += 2)
    {
        IMS_BOOL bNegative = IMS_FALSE;
        const AString& strAdjustment = objTokens.GetAt(i);
        AString strOffset = objTokens.GetAt(i + 1);

        if ((strAdjustment.GetLength() != 10) || !Sdp::IsDigitString(strAdjustment))
        {
            // Invalid adjustment field
            return IMS_FALSE;
        }

        if (strOffset.StartsWith('-'))
        {
            bNegative = IMS_TRUE;
            strOffset = strOffset.GetSubStr(1);
        }

        if (!Sdp::IsTypedTimeString(strOffset))
        {
            // Invalid offset field
            return IMS_FALSE;
        }

        IMS_UINT32 nAdjustment = strAdjustment.ToUInt32();
        IMS_SINT32 nOffset = Sdp::ConvertTypedTimeToSeconds(strOffset);

        if (bNegative)
        {
            nOffset *= -1;
        }

        if (!m_objZoneAdjustments.Append(ZoneAdjustment(nAdjustment, nOffset)))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpTimezone::Encode() const
{
    if (m_objZoneAdjustments.IsEmpty())
    {
        return AString::ConstNull();
    }

    // z=<adjustment time> <offset> <adjustment time> <offset> ...
    AString strLine(1, Sdp::LINE_Z);

    strLine.Append(TextParser::CHAR_EQUAL);
    strLine.Append(GetValue());
    strLine.Append(TextParser::CHAR_CR);
    strLine.Append(TextParser::CHAR_LF);

    return strLine;
}

PUBLIC VIRTUAL AString SdpTimezone::GetValue() const
{
    if (m_objZoneAdjustments.IsEmpty())
    {
        return AString::ConstNull();
    }

    AString strValue;
    AString strAdjustment;

    for (IMS_UINT32 i = 0; i < m_objZoneAdjustments.GetSize(); ++i)
    {
        strAdjustment.Sprintf("%u %d ", m_objZoneAdjustments.GetAt(i).GetAdjustmentTime(),
                m_objZoneAdjustments.GetAt(i).GetOffset());

        strValue.Append(strAdjustment);
    }

    if (strValue.GetLength() > 0)
        strValue.Chop(1);

    return strValue;
}

PUBLIC
IMS_BOOL SdpTimezone::AddAdjustment(IN IMS_UINT32 nAdjustmentTime, IN IMS_SINT32 nOffset)
{
    if (nAdjustmentTime == 0)
    {
        return IMS_FALSE;
    }

    AString strTmp;

    strTmp.SetNumber(nAdjustmentTime);

    if (strTmp.GetLength() != 10)
    {
        return IMS_FALSE;
    }

    return m_objZoneAdjustments.Append(ZoneAdjustment(nAdjustmentTime, nOffset));
}
