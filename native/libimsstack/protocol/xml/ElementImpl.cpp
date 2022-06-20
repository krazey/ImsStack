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
#include "Element.h"
#include "ElementImpl.h"
#include "INamedNodeMap.h"

PUBLIC
ElementImpl::ElementImpl(xmlNodePtr pstNode) :
        m_pElement(new Element(pstNode, this))
{
}

PUBLIC VIRTUAL ElementImpl::~ElementImpl()
{
    if (m_pElement != IMS_NULL)
    {
        delete m_pElement;
        m_pElement = IMS_NULL;
    }
}

PUBLIC VIRTUAL INode* ElementImpl::AppendChild(IN INode* piChild)
{
    return m_pElement->AppendChild(piChild);
}

PUBLIC VIRTUAL INode* ElementImpl::CloneNode(IN IMS_BOOL bDeep)
{
    return m_pElement->CloneNode(bDeep);
}

PUBLIC VIRTUAL INamedNodeMap* ElementImpl::GetAttributes() const
{
    return m_pElement->GetAttributes();
}

PUBLIC VIRTUAL INodeList* ElementImpl::GetChildNodes() const
{
    return m_pElement->GetChildNodes();
}

PUBLIC VIRTUAL INode* ElementImpl::GetFirstChild() const
{
    return m_pElement->GetFirstChild();
}

PUBLIC VIRTUAL INode* ElementImpl::GetLastChild() const
{
    return m_pElement->GetLastChild();
}

PUBLIC VIRTUAL const AString& ElementImpl::GetLocalName() const
{
    return m_pElement->GetLocalName();
}

PUBLIC VIRTUAL const AString& ElementImpl::GetNameSpaceUri() const
{
    return m_pElement->GetNameSpaceUri();
}

PUBLIC VIRTUAL INode* ElementImpl::GetNextSibling() const
{
    return m_pElement->GetNextSibling();
}

PUBLIC VIRTUAL const AString& ElementImpl::GetNodeName() const
{
    return m_pElement->GetNodeName();
}

PUBLIC VIRTUAL IMS_SINT32 ElementImpl::GetNodeType() const
{
    return m_pElement->GetNodeType();
}

PUBLIC VIRTUAL const AString& ElementImpl::GetNodeValue() const
{
    return m_pElement->GetNodeValue();
}

PUBLIC VIRTUAL IDocument* ElementImpl::GetOwnerDocument() const
{
    return m_pElement->GetOwnerDocument();
}

PUBLIC VIRTUAL INode* ElementImpl::GetParentNode() const
{
    return m_pElement->GetParentNode();
}

PUBLIC VIRTUAL const AString& ElementImpl::GetPrefix() const
{
    return m_pElement->GetPrefix();
}

PUBLIC VIRTUAL INode* ElementImpl::GetPreviousSibling() const
{
    return m_pElement->GetPreviousSibling();
}

PUBLIC VIRTUAL const AString& ElementImpl::GetTextContent() const
{
    return m_pElement->GetTextContent();
}

PUBLIC VIRTUAL IMS_BOOL ElementImpl::HasAttribute() const
{
    return m_pElement->HasAttribute();
}

PUBLIC VIRTUAL IMS_BOOL ElementImpl::HasChildNode() const
{
    return m_pElement->HasChildNode();
}

PUBLIC VIRTUAL INode* ElementImpl::InsertBefore(IN INode* piNewChild, IN INode* piRefChild)
{
    return m_pElement->InsertBefore(piNewChild, piRefChild);
}

PUBLIC VIRTUAL IMS_BOOL ElementImpl::IsSupported(
        IN const AString& strFeature, IN const AString& strVersion)
{
    return m_pElement->IsSupported(strFeature, strVersion);
}

PUBLIC VIRTUAL IMS_RESULT ElementImpl::Normalize()
{
    return m_pElement->Normalize();
}

PUBLIC VIRTUAL INode* ElementImpl::RemoveChild(IN INode* piChild)
{
    return m_pElement->RemoveChild(piChild);
}

PUBLIC VIRTUAL INode* ElementImpl::ReplaceChild(IN INode* piNewChild, IN INode* piOldChild)
{
    return m_pElement->ReplaceChild(piNewChild, piOldChild);
}

PUBLIC VIRTUAL void ElementImpl::SetNodeValue(IN const AString& strNodeValue)
{
    m_pElement->SetNodeValue(strNodeValue);
}

PUBLIC VIRTUAL void ElementImpl::SetPrefix(IN const AString& strPrefix)
{
    m_pElement->SetPrefix(strPrefix);
}

PUBLIC VIRTUAL void ElementImpl::SetTextContent(IN const AString& strTextContext)
{
    m_pElement->SetTextContent(strTextContext);
}

PUBLIC VIRTUAL void ElementImpl::DestroyNodeList(IN INodeList*& piNodeList)
{
    m_pElement->DestroyNodeList(piNodeList);
}

PUBLIC VIRTUAL void ElementImpl::DestroyNamedNodeMap(IN INamedNodeMap*& piNamedNodeMap)
{
    m_pElement->DestroyNamedNodeMap(piNamedNodeMap);
}

PUBLIC VIRTUAL void ElementImpl::SetNextSibling(IN INode* piNode)
{
    m_pElement->SetNextSibling(piNode);
}

PUBLIC VIRTUAL void ElementImpl::SetPreviousSibling(IN INode* piNode)
{
    m_pElement->SetPreviousSibling(piNode);
}

PUBLIC VIRTUAL void ElementImpl::SetParent(IN INode* piNode)
{
    m_pElement->SetParent(piNode);
}

PUBLIC VIRTUAL IMS_RESULT ElementImpl::SetChildren(IN INode* piNode)
{
    return m_pElement->SetChildren(piNode);
}

PUBLIC VIRTUAL void ElementImpl::SetOwnerDocument(IN IDocument* piDocument)
{
    m_pElement->SetOwnerDocument(piDocument);
}

PUBLIC VIRTUAL const AString& ElementImpl::GetAttribute(IN const AString& strName) const
{
    return m_pElement->GetAttribute(strName);
}

PUBLIC VIRTUAL IAttr* ElementImpl::GetAttributeNode(IN const AString& strName) const
{
    return m_pElement->GetAttributeNode(strName);
}

PUBLIC VIRTUAL IAttr* ElementImpl::GetAttributeNodeNs(
        IN const AString& strNamespaceUri, IN const AString& strLocalName) const
{
    return m_pElement->GetAttributeNodeNs(strNamespaceUri, strLocalName);
}

PUBLIC VIRTUAL const AString& ElementImpl::GetAttributeNs(
        IN const AString& strNamespaceUri, IN const AString& strLocalName) const
{
    return m_pElement->GetAttributeNs(strNamespaceUri, strLocalName);
}

PUBLIC VIRTUAL INodeList* ElementImpl::GetElementsByTagName(IN const AString& strName) const
{
    return m_pElement->GetElementsByTagName(strName);
}

PUBLIC VIRTUAL INodeList* ElementImpl::GetElementsByTagNameNs(
        IN const AString& strNamespaceUri, IN const AString& strLocalName) const
{
    return m_pElement->GetElementsByTagNameNs(strNamespaceUri, strLocalName);
}

PUBLIC VIRTUAL const AString& ElementImpl::GetTagName() const
{
    return m_pElement->GetTagName();
}

PUBLIC VIRTUAL IMS_BOOL ElementImpl::HasAttributeB(IN const AString& strName) const
{
    return m_pElement->HasAttributeB(strName);
}

PUBLIC VIRTUAL IMS_BOOL ElementImpl::HasAttributeNs(
        IN const AString& strNamespaceUri, IN const AString& strLocalName) const
{
    return m_pElement->HasAttributeNs(strNamespaceUri, strLocalName);
}

PUBLIC VIRTUAL void ElementImpl::RemoveAttribute(IN const AString& strName)
{
    m_pElement->RemoveAttribute(strName);
}

PUBLIC VIRTUAL IAttr* ElementImpl::RemoveAttributeNode(IN IAttr* piAttr)
{
    return m_pElement->RemoveAttributeNode(piAttr);
}

PUBLIC VIRTUAL void ElementImpl::RemoveAttributeNs(
        IN const AString& strNamespaceUri, IN const AString& strLocalName)
{
    return m_pElement->RemoveAttributeNs(strNamespaceUri, strLocalName);
}

PUBLIC VIRTUAL void ElementImpl::SetAttribute(IN const AString& strName, IN const AString& strValue)
{
    m_pElement->SetAttribute(strName, strValue);
}

PUBLIC VIRTUAL IAttr* ElementImpl::SetAttributeNode(IN IAttr* piAttr)
{
    return m_pElement->SetAttributeNode(piAttr);
}

PUBLIC VIRTUAL IAttr* ElementImpl::SetAttributeNodeNs(IN IAttr* piAttr)
{
    return m_pElement->SetAttributeNodeNs(piAttr);
}

PUBLIC VIRTUAL void ElementImpl::SetAttributeNs(IN const AString& strNamespaceUri,
        IN const AString& strQualifiedName, IN const AString& strValue)
{
    m_pElement->SetAttributeNs(strNamespaceUri, strQualifiedName, strValue);
}

PUBLIC VIRTUAL void ElementImpl::SetIdAttribute(IN const AString& strName, IN IMS_BOOL bIsId)
{
    m_pElement->SetIdAttribute(strName, bIsId);
}

PUBLIC VIRTUAL void ElementImpl::SetIdAttributeNode(IN IAttr* piIdAttr, IN IMS_BOOL bIsId)
{
    m_pElement->SetIdAttributeNode(piIdAttr, bIsId);
}

PUBLIC VIRTUAL void ElementImpl::SetIdAttributeNodeNs(
        IN const AString& strNamespaceUri, IN const AString& strLocalName, IN IMS_BOOL bIsId)
{
    m_pElement->SetIdAttributeNodeNs(strNamespaceUri, strLocalName, bIsId);
}

PUBLIC VIRTUAL const AString& ElementImpl::GetAttribute(IN const IMS_CHAR* pszName) const
{
    return m_pElement->GetAttribute(pszName);
}

PUBLIC VIRTUAL IAttr* ElementImpl::GetAttributeNode(IN const IMS_CHAR* pszName) const
{
    return m_pElement->GetAttributeNode(pszName);
}

PUBLIC VIRTUAL INodeList* ElementImpl::GetElementsByTagName(IN const IMS_CHAR* pszName) const
{
    return m_pElement->GetElementsByTagName(pszName);
}
