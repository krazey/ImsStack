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
#include "CharacterData.h"

PUBLIC
CharacterData::CharacterData() :
        Node(),
        m_strCharacterData(AString::ConstNull())
{
}

PUBLIC
CharacterData::CharacterData(IN xmlNodePtr pstNode) :
        Node(pstNode),
        m_strCharacterData(AString::ConstNull())
{
    if (pstNode->content != IMS_NULL)
    {
        m_strCharacterData = reinterpret_cast<const IMS_CHAR*>(pstNode->content);
    }
}

PUBLIC VIRTUAL CharacterData::~CharacterData() {}

PUBLIC VIRTUAL void CharacterData::AppendData(IN const AString& strData)
{
    m_strCharacterData = m_strCharacterData.Append(strData);
}

PUBLIC VIRTUAL void CharacterData::DeleteData(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount)
{
    m_strCharacterData.Erase(nOffset, nCount);
}

PUBLIC VIRTUAL const AString& CharacterData::GetData() const
{
    return m_strCharacterData;
}

PUBLIC VIRTUAL IMS_SINT32 CharacterData::GetLength() const
{
    return m_strCharacterData.GetLength();
}

PUBLIC VIRTUAL void CharacterData::InsertData(IN IMS_SINT32 nOffset, IN const AString& strData)
{
    m_strCharacterData = m_strCharacterData.Insert(nOffset, strData);
}

PUBLIC VIRTUAL void CharacterData::ReplaceData(
        IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount, IN const AString& strData)
{
    m_strCharacterData = m_strCharacterData.Replace(nOffset, nCount, strData);
}

PUBLIC VIRTUAL void CharacterData::SetData(IN const AString& strData)
{
    m_strCharacterData = strData;
}

PUBLIC VIRTUAL AString CharacterData::SubstringData(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount)
{
    m_strCharacterData = m_strCharacterData.GetSubStr(nOffset, nCount);
    return m_strCharacterData;
}
