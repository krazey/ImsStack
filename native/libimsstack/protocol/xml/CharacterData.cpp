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
        m_strCharacterData = (const IMS_CHAR*)pstNode->content;
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
