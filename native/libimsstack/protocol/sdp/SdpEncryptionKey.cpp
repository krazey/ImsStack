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
#include "SdpEncryptionKey.h"

PRIVATE GLOBAL const IMS_CHAR* SdpEncryptionKey::METHOD[SdpEncryptionKey::METHOD_MAX] = {
        "prompt",
        "clear",
        "base64",
        "uri",
};

PUBLIC
SdpEncryptionKey::SdpEncryptionKey() :
        SdpLine(),
        m_nMethod(METHOD_INVALID),
        m_strKey(AString::ConstNull())
{
}

PUBLIC
SdpEncryptionKey::SdpEncryptionKey(IN const SdpEncryptionKey& other) :
        SdpLine(other),
        m_nMethod(other.m_nMethod),
        m_strKey(other.m_strKey)
{
}

PUBLIC VIRTUAL SdpEncryptionKey::~SdpEncryptionKey() {}

PUBLIC
SdpEncryptionKey& SdpEncryptionKey::operator=(IN const SdpEncryptionKey& other)
{
    if (this != &other)
    {
        SdpLine::operator=(other);

        m_nMethod = other.m_nMethod;
        m_strKey = other.m_strKey;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpEncryptionKey::Decode(IN const AString& strValue)
{
    // k=<method>
    // k=<method>:<encryption key>
    AString strMethod;
    IMS_SINT32 nIndex = strValue.GetIndexOf(TextParser::CHAR_COLON);

    if (nIndex != AString::NPOS)
    {
        strMethod = strValue.GetSubStr(0, nIndex);
        m_strKey = strValue.GetSubStr(nIndex + 1);
    }
    else
    {
        strMethod = strValue;
        m_strKey = AString::ConstNull();
    }

    m_nMethod = METHOD_INVALID;

    for (IMS_UINT32 i = 0; i < METHOD_MAX; ++i)
    {
        if (strMethod.Equals(METHOD[i]))
        {
            m_nMethod = i;
            break;
        }
    }

    if (m_nMethod == METHOD_INVALID)
    {
        // Invalid method field
        return IMS_FALSE;
    }

    if ((m_nMethod == METHOD_PROMPT) && (m_strKey.GetLength() > 0))
    {
        // Invalid encryption line
        return IMS_FALSE;
    }

    if ((m_nMethod != METHOD_PROMPT) && (m_strKey.GetLength() == 0))
    {
        // Invalid encryption line
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpEncryptionKey::Encode() const
{
    if (m_nMethod == METHOD_INVALID)
    {
        return AString::ConstNull();
    }

    // k=<method>
    // k=<method>:<encryption key>
    AString strLine(1, Sdp::LINE_K);

    strLine.Append(TextParser::CHAR_EQUAL);
    strLine.Append(GetValue());
    strLine.Append(TextParser::CHAR_CR);
    strLine.Append(TextParser::CHAR_LF);

    return strLine;
}

PUBLIC VIRTUAL AString SdpEncryptionKey::GetValue() const
{
    if (m_nMethod == METHOD_INVALID)
    {
        return AString::ConstNull();
    }

    AString strValue;

    strValue.Append(METHOD[m_nMethod]);

    if (m_nMethod != METHOD_PROMPT)
    {
        strValue.Append(TextParser::STR_COLON);
        strValue.Append(m_strKey);
    }

    return strValue;
}

PUBLIC
IMS_BOOL SdpEncryptionKey::SetValue(
        IN IMS_SINT32 nMethod, IN const AString& strKey /*= AString::ConstNull()*/)
{
    if ((nMethod <= METHOD_INVALID) || (nMethod >= METHOD_MAX))
    {
        return IMS_FALSE;
    }

    if (nMethod == METHOD_PROMPT)
    {
        if (strKey.GetLength() != 0)
        {
            return IMS_FALSE;
        }
    }
    else if (strKey.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    m_nMethod = nMethod;
    m_strKey = strKey;

    return IMS_TRUE;
}
