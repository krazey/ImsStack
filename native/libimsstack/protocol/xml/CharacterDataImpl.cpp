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
#include "CharacterDataImpl.h"

PUBLIC
CharacterDataImpl::CharacterDataImpl() :
        m_pCharacterData(new CharacterData())
{
}

PUBLIC VIRTUAL CharacterDataImpl::~CharacterDataImpl()
{
    if (m_pCharacterData != IMS_NULL)
    {
        delete m_pCharacterData;
        m_pCharacterData = IMS_NULL;
    }
}

PUBLIC VIRTUAL INode* CharacterDataImpl::AppendChild(IN INode* piChild)
{
    return m_pCharacterData->AppendChild(piChild);
}

PUBLIC VIRTUAL INode* CharacterDataImpl::CloneNode(IN IMS_BOOL bDeep)
{
    return m_pCharacterData->CloneNode(bDeep);
}

PUBLIC VIRTUAL INamedNodeMap* CharacterDataImpl::GetAttributes() const
{
    return m_pCharacterData->GetAttributes();
}

PUBLIC VIRTUAL INodeList* CharacterDataImpl::GetChildNodes() const
{
    return m_pCharacterData->GetChildNodes();
}

PUBLIC VIRTUAL INode* CharacterDataImpl::GetFirstChild() const
{
    return m_pCharacterData->GetFirstChild();
}

PUBLIC VIRTUAL INode* CharacterDataImpl::GetLastChild() const
{
    return m_pCharacterData->GetLastChild();
}

PUBLIC VIRTUAL const AString& CharacterDataImpl::GetLocalName() const
{
    return m_pCharacterData->GetLocalName();
}

PUBLIC VIRTUAL const AString& CharacterDataImpl::GetNameSpaceUri() const
{
    return m_pCharacterData->GetNameSpaceUri();
}

PUBLIC VIRTUAL INode* CharacterDataImpl::GetNextSibling() const
{
    return m_pCharacterData->GetNextSibling();
}

PUBLIC VIRTUAL const AString& CharacterDataImpl::GetNodeName() const
{
    return m_pCharacterData->GetNodeName();
}

PUBLIC VIRTUAL IMS_SINT32 CharacterDataImpl::GetNodeType() const
{
    return m_pCharacterData->GetNodeType();
}

PUBLIC VIRTUAL const AString& CharacterDataImpl::GetNodeValue() const
{
    return m_pCharacterData->GetNodeValue();
}

PUBLIC VIRTUAL IDocument* CharacterDataImpl::GetOwnerDocument() const
{
    return m_pCharacterData->GetOwnerDocument();
}

PUBLIC VIRTUAL INode* CharacterDataImpl::GetParentNode() const
{
    return m_pCharacterData->GetParentNode();
}

PUBLIC VIRTUAL const AString& CharacterDataImpl::GetPrefix() const
{
    return m_pCharacterData->GetPrefix();
}

PUBLIC VIRTUAL INode* CharacterDataImpl::GetPreviousSibling() const
{
    return m_pCharacterData->GetPreviousSibling();
}

PUBLIC VIRTUAL const AString& CharacterDataImpl::GetTextContent() const
{
    return m_pCharacterData->GetTextContent();
}

PUBLIC VIRTUAL IMS_BOOL CharacterDataImpl::HasAttribute() const
{
    return m_pCharacterData->HasAttribute();
}

PUBLIC VIRTUAL IMS_BOOL CharacterDataImpl::HasChildNode() const
{
    return m_pCharacterData->HasChildNode();
}

PUBLIC VIRTUAL INode* CharacterDataImpl::InsertBefore(IN INode* piNewChild, IN INode* piRefChild)
{
    return m_pCharacterData->InsertBefore(piNewChild, piRefChild);
}

PUBLIC VIRTUAL IMS_BOOL CharacterDataImpl::IsSupported(
        IN const AString& strFeature, IN const AString& strVersion)
{
    return m_pCharacterData->IsSupported(strFeature, strVersion);
}

PUBLIC VIRTUAL IMS_RESULT CharacterDataImpl::Normalize()
{
    return m_pCharacterData->Normalize();
}

PUBLIC VIRTUAL INode* CharacterDataImpl::RemoveChild(IN INode* piChild)
{
    return m_pCharacterData->RemoveChild(piChild);
}

PUBLIC VIRTUAL INode* CharacterDataImpl::ReplaceChild(IN INode* piNewChild, IN INode* piOldChild)
{
    return m_pCharacterData->ReplaceChild(piNewChild, piOldChild);
}

PUBLIC VIRTUAL void CharacterDataImpl::SetNodeValue(IN const AString& strNodeValue)
{
    m_pCharacterData->SetNodeValue(strNodeValue);
}

PUBLIC VIRTUAL void CharacterDataImpl::SetPrefix(IN const AString& strPrefix)
{
    m_pCharacterData->SetPrefix(strPrefix);
}

PUBLIC VIRTUAL void CharacterDataImpl::SetTextContent(IN const AString& strTextContext)
{
    m_pCharacterData->SetTextContent(strTextContext);
}

PUBLIC VIRTUAL void CharacterDataImpl::DestroyNodeList(IN INodeList*& piNodeList)
{
    m_pCharacterData->DestroyNodeList(piNodeList);
}

PUBLIC VIRTUAL void CharacterDataImpl::DestroyNamedNodeMap(IN INamedNodeMap*& piNamedNodeMap)
{
    m_pCharacterData->DestroyNamedNodeMap(piNamedNodeMap);
}

PUBLIC VIRTUAL void CharacterDataImpl::SetNextSibling(IN INode* piNode)
{
    m_pCharacterData->SetNextSibling(piNode);
}

PUBLIC VIRTUAL void CharacterDataImpl::SetPreviousSibling(IN INode* piNode)
{
    m_pCharacterData->SetPreviousSibling(piNode);
}

PUBLIC VIRTUAL void CharacterDataImpl::SetParent(IN INode* piNode)
{
    m_pCharacterData->SetParent(piNode);
}

PUBLIC VIRTUAL IMS_RESULT CharacterDataImpl::SetChildren(IN INode* piNode)
{
    return m_pCharacterData->SetChildren(piNode);
}

PUBLIC VIRTUAL void CharacterDataImpl::SetOwnerDocument(IN IDocument* piDocument)
{
    m_pCharacterData->SetOwnerDocument(piDocument);
}

PUBLIC VIRTUAL void CharacterDataImpl::AppendData(IN const AString& strData)
{
    m_pCharacterData->AppendData(strData);
}

PUBLIC VIRTUAL void CharacterDataImpl::DeleteData(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount)
{
    m_pCharacterData->DeleteData(nOffset, nCount);
}

PUBLIC VIRTUAL const AString& CharacterDataImpl::GetData() const
{
    return m_pCharacterData->GetData();
}

PUBLIC VIRTUAL IMS_SINT32 CharacterDataImpl::GetLength() const
{
    return m_pCharacterData->GetLength();
}

PUBLIC VIRTUAL void CharacterDataImpl::InsertData(IN IMS_SINT32 nOffset, IN const AString& strData)
{
    return m_pCharacterData->InsertData(nOffset, strData);
}

PUBLIC VIRTUAL void CharacterDataImpl::ReplaceData(
        IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount, IN const AString& strData)
{
    m_pCharacterData->ReplaceData(nOffset, nCount, strData);
}

PUBLIC VIRTUAL void CharacterDataImpl::SetData(IN const AString& strData)
{
    m_pCharacterData->SetData(strData);
}

PUBLIC VIRTUAL AString CharacterDataImpl::SubstringData(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount)
{
    return m_pCharacterData->SubstringData(nOffset, nCount);
}
