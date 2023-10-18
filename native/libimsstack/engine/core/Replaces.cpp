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
#include "TextParser.h"

#include "Replaces.h"

PUBLIC
Replaces::Replaces() :
        m_strCallId(AString::ConstNull()),
        m_strFromTag(AString::ConstNull()),
        m_strToTag(AString::ConstNull()),
        m_bIsEarlyOnly(IMS_FALSE),
        m_pDialog(IMS_NULL)
{
}

PUBLIC
Replaces::Replaces(IN const AString& strCallId, IN const AString& strLocalTag,
        IN const AString& strRemoteTag, IN IMS_BOOL bIsEarlyOnly /*= IMS_FALSE*/) :
        m_strCallId(strCallId),
        m_strFromTag(strLocalTag),
        m_strToTag(strRemoteTag),
        m_bIsEarlyOnly(bIsEarlyOnly)
{
    m_pDialog = new Dialog(m_strCallId, m_strFromTag, m_strToTag);
}

PUBLIC
Replaces::Replaces(IN const Replaces& other) :
        m_strCallId(other.m_strCallId),
        m_strFromTag(other.m_strFromTag),
        m_strToTag(other.m_strToTag),
        m_bIsEarlyOnly(other.m_bIsEarlyOnly),
        m_pDialog(IMS_NULL)
{
    if (other.m_pDialog != IMS_NULL)
    {
        if (m_strToTag.Equals(other.m_pDialog->GetLocalTag()))
        {
            m_pDialog = new Dialog(m_strCallId, m_strToTag, m_strFromTag);
        }
        else
        {
            m_pDialog = new Dialog(m_strCallId, m_strFromTag, m_strToTag);
        }
    }
}

PUBLIC
Replaces::~Replaces()
{
    if (m_pDialog != IMS_NULL)
    {
        delete m_pDialog;
        m_pDialog = IMS_NULL;
    }
}

PUBLIC
Replaces& Replaces::operator=(IN const Replaces& other)
{
    if (this != &other)
    {
        m_strCallId = other.m_strCallId;
        m_strFromTag = other.m_strFromTag;
        m_strToTag = other.m_strToTag;
        m_bIsEarlyOnly = other.m_bIsEarlyOnly;

        if (other.m_pDialog != IMS_NULL)
        {
            if (m_pDialog != IMS_NULL)
            {
                delete m_pDialog;
                m_pDialog = IMS_NULL;
            }

            if (m_strToTag.Equals(other.m_pDialog->GetLocalTag()))
            {
                m_pDialog = new Dialog(m_strCallId, m_strToTag, m_strFromTag);
            }
            else
            {
                m_pDialog = new Dialog(m_strCallId, m_strFromTag, m_strToTag);
            }
        }
    }

    return (*this);
}

PUBLIC
IMS_BOOL Replaces::Create(IN const AString& strReplacesHeader, IN IMS_BOOL bUas /*= IMS_TRUE*/)
{
    AString strReplaces = TextParser::DoPercentDecoding(strReplacesHeader);
    ImsList<AString> objTokens = strReplaces.Split(TextParser::CHAR_SEMICOLON);

    if (objTokens.GetSize() < 3)
    {
        return IMS_FALSE;
    }

    // call-id
    m_strCallId = objTokens.GetAt(0);

    // to-tag & from-tag
    for (IMS_UINT32 i = 1; i < objTokens.GetSize(); ++i)
    {
        const AString& strTemp = objTokens.GetAt(i);
        IMS_SINT32 nIndex = strTemp.GetIndexOf(TextParser::CHAR_EQUAL);
        AString strName = strTemp.GetSubStr(0, nIndex);

        if (strName.EqualsIgnoreCase("to-tag"))
        {
            m_strToTag = strTemp.GetSubStr(nIndex + 1);
        }
        else if (strName.EqualsIgnoreCase("from-tag"))
        {
            m_strFromTag = strTemp.GetSubStr(nIndex + 1);
        }
        else if (strName.EqualsIgnoreCase("early-only"))
        {
            m_bIsEarlyOnly = IMS_TRUE;
        }
    }

    if (m_strCallId.IsNULL() || m_strToTag.IsNULL() || m_strFromTag.IsNULL())
    {
        return IMS_FALSE;
    }

    if (m_pDialog != IMS_NULL)
    {
        delete m_pDialog;
        m_pDialog = IMS_NULL;
    }

    if (bUas)
    {
        m_pDialog = new Dialog(m_strCallId, m_strToTag, m_strFromTag);
    }
    else
    {
        m_pDialog = new Dialog(m_strCallId, m_strFromTag, m_strToTag);
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL Replaces::Equals(IN const Replaces* pOther) const
{
    if (pOther == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!m_strCallId.Equals(pOther->m_strCallId))
    {
        return IMS_FALSE;
    }

    if (!m_strFromTag.Equals(pOther->m_strFromTag))
    {
        return IMS_FALSE;
    }

    if (!m_strToTag.Equals(pOther->m_strToTag))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL Replaces::IsSameDialog(IN const Replaces* pOther) const
{
    if (pOther == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if ((m_pDialog == IMS_NULL) || (pOther->m_pDialog == IMS_NULL))
    {
        return IMS_FALSE;
    }

    return m_pDialog->Equals(pOther->m_pDialog);
}

PUBLIC
AString Replaces::ToString(IN IMS_BOOL bPercentEncoding) const
{
    AString strReplaces;

    if (bPercentEncoding)
    {
        // Check if '@' is present in Call-ID header
        IMS_SINT32 nIndex = m_strCallId.GetIndexOf(TextParser::CHAR_AT);

        if (nIndex == AString::NPOS)
        {
            strReplaces.Append(m_strCallId);
        }
        else
        {
            AString strUserPart = m_strCallId.GetSubStr(0, nIndex);
            AString strDomainPart = m_strCallId.GetSubStr(nIndex + 1);

            strReplaces.Append(strUserPart);
            strReplaces.Append("%40");
            strReplaces.Append(strDomainPart);
        }

        strReplaces.Append("%3Bfrom-tag%3D");
        strReplaces.Append(m_strFromTag);
        strReplaces.Append("%3Bto-tag%3D");
        strReplaces.Append(m_strToTag);
    }
    else
    {
        strReplaces.Append(m_strCallId);
        strReplaces.Append(";from-tag=");
        strReplaces.Append(m_strFromTag);
        strReplaces.Append(";to-tag=");
        strReplaces.Append(m_strToTag);
    }

    if (m_bIsEarlyOnly)
    {
        strReplaces.Append(";early-only");
    }

    return strReplaces;
}
