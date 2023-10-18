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
#include "Text.h"
#include "TextImpl.h"

PUBLIC
TextImpl::TextImpl() :
        m_pText(new Text())
{
}

PUBLIC
TextImpl::TextImpl(IN xmlNodePtr pstNode) :
        m_pText(new Text(pstNode))
{
}

PUBLIC VIRTUAL TextImpl::~TextImpl()
{
    if (m_pText != IMS_NULL)
    {
        delete m_pText;
        m_pText = IMS_NULL;
    }
}

PUBLIC VIRTUAL INode* TextImpl::AppendChild(IN INode* piChild)
{
    return m_pText->AppendChild(piChild);
}

PUBLIC VIRTUAL INode* TextImpl::CloneNode(IN IMS_BOOL bDeep)
{
    return m_pText->CloneNode(bDeep);
}

PUBLIC VIRTUAL INamedNodeMap* TextImpl::GetAttributes() const
{
    return m_pText->GetAttributes();
}

PUBLIC VIRTUAL INodeList* TextImpl::GetChildNodes() const
{
    return m_pText->GetChildNodes();
}

PUBLIC VIRTUAL INode* TextImpl::GetFirstChild() const
{
    return m_pText->GetFirstChild();
}

PUBLIC VIRTUAL INode* TextImpl::GetLastChild() const
{
    return m_pText->GetLastChild();
}

PUBLIC VIRTUAL const AString& TextImpl::GetLocalName() const
{
    return m_pText->GetLocalName();
}

PUBLIC VIRTUAL const AString& TextImpl::GetNameSpaceUri() const
{
    return m_pText->GetNameSpaceUri();
}

PUBLIC VIRTUAL INode* TextImpl::GetNextSibling() const
{
    return m_pText->GetNextSibling();
}

PUBLIC VIRTUAL const AString& TextImpl::GetNodeName() const
{
    return m_pText->GetNodeName();
}

PUBLIC VIRTUAL IMS_SINT32 TextImpl::GetNodeType() const
{
    return m_pText->GetNodeType();
}

PUBLIC VIRTUAL const AString& TextImpl::GetNodeValue() const
{
    return m_pText->GetNodeValue();
}

PUBLIC VIRTUAL IDocument* TextImpl::GetOwnerDocument() const
{
    return m_pText->GetOwnerDocument();
}

PUBLIC VIRTUAL INode* TextImpl::GetParentNode() const
{
    return m_pText->GetParentNode();
}

PUBLIC VIRTUAL const AString& TextImpl::GetPrefix() const
{
    return m_pText->GetPrefix();
}

PUBLIC VIRTUAL INode* TextImpl::GetPreviousSibling() const
{
    return m_pText->GetPreviousSibling();
}

PUBLIC VIRTUAL const AString& TextImpl::GetTextContent() const
{
    return m_pText->GetTextContent();
}

PUBLIC VIRTUAL IMS_BOOL TextImpl::HasAttribute() const
{
    return m_pText->HasAttribute();
}

PUBLIC VIRTUAL IMS_BOOL TextImpl::HasChildNode() const
{
    return m_pText->HasChildNode();
}

PUBLIC VIRTUAL INode* TextImpl::InsertBefore(IN INode* piNewChild, IN INode* piRefChild)
{
    return m_pText->InsertBefore(piNewChild, piRefChild);
}

PUBLIC VIRTUAL IMS_BOOL TextImpl::IsSupported(
        IN const AString& strFeature, IN const AString& strVersion)
{
    return m_pText->IsSupported(strFeature, strVersion);
}

PUBLIC VIRTUAL IMS_RESULT TextImpl::Normalize()
{
    return m_pText->Normalize();
}

PUBLIC VIRTUAL INode* TextImpl::RemoveChild(IN INode* piChild)
{
    return m_pText->RemoveChild(piChild);
}

PUBLIC VIRTUAL INode* TextImpl::ReplaceChild(IN INode* piNewChild, IN INode* piOldChild)
{
    return m_pText->ReplaceChild(piNewChild, piOldChild);
}

PUBLIC VIRTUAL void TextImpl::SetNodeValue(IN const AString& strNodeValue)
{
    m_pText->SetNodeValue(strNodeValue);
}

PUBLIC VIRTUAL void TextImpl::SetPrefix(IN const AString& strPrefix)
{
    m_pText->SetPrefix(strPrefix);
}

PUBLIC VIRTUAL void TextImpl::SetTextContent(IN const AString& strTextContext)
{
    m_pText->SetTextContent(strTextContext);
}

PUBLIC VIRTUAL void TextImpl::DestroyNodeList(IN INodeList*& piNodeList)
{
    m_pText->DestroyNodeList(piNodeList);
}

PUBLIC VIRTUAL void TextImpl::DestroyNamedNodeMap(IN INamedNodeMap*& piNamedNodeMap)
{
    m_pText->DestroyNamedNodeMap(piNamedNodeMap);
}

PUBLIC VIRTUAL void TextImpl::SetNextSibling(IN INode* piNode)
{
    m_pText->SetNextSibling(piNode);
}

PUBLIC VIRTUAL void TextImpl::SetPreviousSibling(IN INode* piNode)
{
    m_pText->SetPreviousSibling(piNode);
}

PUBLIC VIRTUAL void TextImpl::SetParent(IN INode* piNode)
{
    m_pText->SetParent(piNode);
}

PUBLIC VIRTUAL void TextImpl::SetOwnerDocument(IN IDocument* piDocument)
{
    m_pText->SetOwnerDocument(piDocument);
}

PUBLIC VIRTUAL IMS_RESULT TextImpl::SetChildren(INode* piNode)
{
    return m_pText->SetChildren(piNode);
}

PUBLIC VIRTUAL void TextImpl::AppendData(IN const AString& strData)
{
    m_pText->AppendData(strData);
}

PUBLIC VIRTUAL void TextImpl::DeleteData(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount)
{
    m_pText->DeleteData(nOffset, nCount);
}

PUBLIC VIRTUAL const AString& TextImpl::GetData() const
{
    return m_pText->GetData();
}

PUBLIC VIRTUAL IMS_SINT32 TextImpl::GetLength() const
{
    return m_pText->GetLength();
}

PUBLIC VIRTUAL void TextImpl::InsertData(IN IMS_SINT32 nOffset, IN const AString& strData)
{
    m_pText->InsertData(nOffset, strData);
}

PUBLIC VIRTUAL void TextImpl::ReplaceData(
        IN IMS_SINT32 nOffSet, IN IMS_SINT32 nCount, IN const AString& strData)
{
    m_pText->ReplaceData(nOffSet, nCount, strData);
}

PUBLIC VIRTUAL void TextImpl::SetData(IN const AString& strData)
{
    m_pText->SetData(strData);
}

PUBLIC VIRTUAL AString TextImpl::SubstringData(IN IMS_SINT32 nOffSet, IN IMS_SINT32 nCount)
{
    return m_pText->SubstringData(nOffSet, nCount);
}

PUBLIC VIRTUAL IText* TextImpl::SplitText(IN IMS_SINT32 nOffset)
{
    return m_pText->SplitText(nOffset);
}
