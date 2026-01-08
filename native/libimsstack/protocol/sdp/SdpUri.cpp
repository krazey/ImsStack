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
#include "SdpUri.h"

PUBLIC
SdpUri::SdpUri() :
        SdpLine(),
        m_strUri(AString::ConstNull())
{
}

PUBLIC
SdpUri::SdpUri(IN const SdpUri& other) :
        SdpLine(other),
        m_strUri(other.m_strUri)
{
}

PUBLIC VIRTUAL SdpUri::~SdpUri() {}

/*

Remarks

*/
PUBLIC
SdpUri& SdpUri::operator=(IN const SdpUri& other)
{
    if (this != &other)
    {
        SdpLine::operator=(other);

        m_strUri = other.m_strUri;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpUri::Decode(IN const AString& strValue)
{
    // u=<uri>

    if (!Sdp::IsUriString(strValue))
    {
        return IMS_FALSE;
    }

    m_strUri = strValue;

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpUri::Encode() const
{
    if (m_strUri.GetLength() == 0)
    {
        return AString::ConstNull();
    }

    // u=<uri>
    AString strLine(1, Sdp::LINE_U);

    strLine.Append(TextParser::CHAR_EQUAL);
    strLine.Append(m_strUri);
    strLine.Append(TextParser::CHAR_CR);
    strLine.Append(TextParser::CHAR_LF);

    return strLine;
}
