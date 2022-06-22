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

#include "conf/ConfigSection.h"

PUBLIC
ConfigSection::ConfigSection() :
        m_strSectionName(AString::ConstNull())
{
}

PUBLIC
ConfigSection::~ConfigSection()
{
    if (!m_objSectionData.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objSectionData.GetSize(); ++i)
        {
            ConfigSectionData* pData = m_objSectionData.GetAt(i);

            if (pData != IMS_NULL)
            {
                delete pData;
            }
        }

        m_objSectionData.Clear();
    }
}

PUBLIC
void ConfigSection::GetKeys(OUT AStringArray& objKeys) const
{
    for (IMS_UINT32 i = 0; i < m_objSectionData.GetSize(); ++i)
    {
        const ConfigSectionData* pSectionData = m_objSectionData.GetAt(i);

        objKeys.AddElement(pSectionData->GetKey());
    }
}

PUBLIC
const AString& ConfigSection::GetValue(IN const IMS_CHAR* pszKey) const
{
    if (m_objSectionData.IsEmpty())
    {
        return AString::ConstNull();
    }

    for (IMS_UINT32 i = 0; i < m_objSectionData.GetSize(); ++i)
    {
        const ConfigSectionData* pData = m_objSectionData.GetAt(i);

        if (pData->GetKey().EqualsIgnoreCase(pszKey))
        {
            return pData->GetValue();
        }
    }

    return AString::ConstNull();
}

PUBLIC
IMS_BOOL ConfigSection::SetValue(IN const IMS_CHAR* pszKey, IN const AString& strValue)
{
    if (strValue.IsNULL())
    {
        return IMS_FALSE;
    }

    if (m_objSectionData.IsEmpty())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objSectionData.GetSize(); ++i)
    {
        ConfigSectionData* pData = m_objSectionData.GetAt(i);

        if (pData->GetKey().EqualsIgnoreCase(pszKey))
        {
            pData->SetValue(strValue);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
AString ConfigSection::ToString() const
{
    AString strTmpVal;

    // Append the comment
    strTmpVal = m_objComment.ToString();

    // Section: [SectionName]
    strTmpVal.Append(TextParser::CHAR_LSBRACKET);
    strTmpVal.Append(m_strSectionName);
    strTmpVal.Append(TextParser::CHAR_RSBRACKET);
    strTmpVal.Append(TextParser::STR_CRLF);

    // Section parameters
    for (IMS_UINT32 i = 0; i < m_objSectionData.GetSize(); ++i)
    {
        const ConfigSectionData* pData = m_objSectionData.GetAt(i);

        strTmpVal.Append(pData->ToString());
        strTmpVal.Append(TextParser::STR_CRLF);
    }

    return strTmpVal;
}

PRIVATE
IMS_BOOL ConfigSection::AddSectionData(IN const AString& strKeyValue)
{
    IMS_SINT32 nStartIndex = strKeyValue.GetIndexOf(TextParser::CHAR_EQUAL);

    if (nStartIndex == AString::NPOS)
    {
        return IMS_FALSE;
    }

    AString strKey = strKeyValue.GetSubStr(0, nStartIndex).Trim();
    AString strValue = strKeyValue.GetSubStr(nStartIndex + 1).Trim();

    ConfigSectionData* pData = new ConfigSectionData(strKey, strValue);

    if (pData == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!m_objSectionData.Append(pData))
    {
        delete pData;

        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
ConfigSectionData* ConfigSection::GetLastElement() const
{
    if (m_objSectionData.IsEmpty())
    {
        return IMS_NULL;
    }

    return m_objSectionData.GetAt(m_objSectionData.GetSize() - 1);
}
