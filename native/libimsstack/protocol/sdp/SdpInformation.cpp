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
#include "TextParser.h"

#include "Sdp.h"
#include "SdpInformation.h"

PUBLIC
SdpInformation::SdpInformation() :
        SdpLine(),
        m_strInformation(AString::ConstNull())
{
}

PUBLIC
SdpInformation::SdpInformation(IN const SdpInformation& other) :
        SdpLine(other),
        m_strInformation(other.m_strInformation)
{
}

PUBLIC VIRTUAL SdpInformation::~SdpInformation() {}

PUBLIC
SdpInformation& SdpInformation::operator=(IN const SdpInformation& other)
{
    if (this != &other)
    {
        SdpLine::operator=(other);

        m_strInformation = other.m_strInformation;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpInformation::Decode(IN const AString& strValue)
{
    // i=<session description>

    if (!Sdp::IsTextString(strValue))
    {
        // Invalid session information field
        return IMS_FALSE;
    }

    m_strInformation = strValue;

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpInformation::Encode() const
{
    if (m_strInformation.GetLength() == 0)
    {
        return AString::ConstNull();
    }

    // i=<session description>
    AString strLine(1, Sdp::LINE_I);

    strLine.Append(TextParser::CHAR_EQUAL);
    strLine.Append(GetValue());
    strLine.Append(TextParser::CHAR_CR);
    strLine.Append(TextParser::CHAR_LF);

    return strLine;
}
