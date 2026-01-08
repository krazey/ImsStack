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
#include "SdpSessionName.h"

PUBLIC
SdpSessionName::SdpSessionName() :
        SdpLine(),
        m_strName(&TextParser::CHAR_HYPHEN, 1)
{
}

PUBLIC
SdpSessionName::SdpSessionName(IN const SdpSessionName& other) :
        SdpLine(other),
        m_strName(other.m_strName)
{
}

PUBLIC VIRTUAL SdpSessionName::~SdpSessionName() {}

PUBLIC
SdpSessionName& SdpSessionName::operator=(IN const SdpSessionName& other)
{
    if (this != &other)
    {
        SdpLine::operator=(other);

        m_strName = other.m_strName;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpSessionName::Decode(IN const AString& strValue)
{
    // s=<session name>

    if (!Sdp::IsTextString(strValue))
    {
        // Invalid session name field
        return IMS_FALSE;
    }

    m_strName = strValue;

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpSessionName::Encode() const
{
    // s=<session name>
    AString strLine(1, Sdp::LINE_S);

    strLine.Append(TextParser::CHAR_EQUAL);
    strLine.Append(GetValue());
    strLine.Append(TextParser::CHAR_CR);
    strLine.Append(TextParser::CHAR_LF);

    return strLine;
}
