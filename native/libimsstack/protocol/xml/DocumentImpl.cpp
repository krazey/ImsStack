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
#include "Document.h"
#include "DocumentImpl.h"

PUBLIC
DocumentImpl::DocumentImpl(IN xmlDocPtr pstDoc, IN xmlXPathContextPtr pstXpathContext) :
        m_pDocument(new Document(pstDoc, pstXpathContext))
{
}

PUBLIC VIRTUAL DocumentImpl::~DocumentImpl()
{
    if (m_pDocument != IMS_NULL)
    {
        delete m_pDocument;
        m_pDocument = IMS_NULL;
    }
}

PUBLIC VIRTUAL INode* DocumentImpl::AppendChild(IN INode* piChild)
{
    return m_pDocument->AppendChild(piChild);
}

PUBLIC VIRTUAL INode* DocumentImpl::CloneNode(IN IMS_BOOL bDeep)
{
    return m_pDocument->CloneNode(bDeep);
}

PUBLIC VIRTUAL INamedNodeMap* DocumentImpl::GetAttributes() const
{
    return m_pDocument->GetAttributes();
}

PUBLIC VIRTUAL INodeList* DocumentImpl::GetChildNodes() const
{
    return m_pDocument->GetChildNodes();
}

PUBLIC VIRTUAL INode* DocumentImpl::GetFirstChild() const
{
    return m_pDocument->GetFirstChild();
}

PUBLIC VIRTUAL INode* DocumentImpl::GetLastChild() const
{
    return m_pDocument->GetLastChild();
}

PUBLIC VIRTUAL const AString& DocumentImpl::GetLocalName() const
{
    return m_pDocument->GetLocalName();
}

PUBLIC VIRTUAL const AString& DocumentImpl::GetNameSpaceUri() const
{
    return m_pDocument->GetNameSpaceUri();
}

PUBLIC VIRTUAL INode* DocumentImpl::GetNextSibling() const
{
    return m_pDocument->GetNextSibling();
}

PUBLIC VIRTUAL const AString& DocumentImpl::GetNodeName() const
{
    return m_pDocument->GetNodeName();
}

PUBLIC VIRTUAL IMS_SINT32 DocumentImpl::GetNodeType() const
{
    return m_pDocument->GetNodeType();
}

PUBLIC VIRTUAL const AString& DocumentImpl::GetNodeValue() const
{
    return m_pDocument->GetNodeValue();
}

PUBLIC VIRTUAL void DocumentImpl::DestroyNodeList(IN INodeList*& piNodeList)
{
    return m_pDocument->DestroyNodeList(piNodeList);
}

PUBLIC VIRTUAL void DocumentImpl::DestroyNamedNodeMap(IN INamedNodeMap*& piNamedNodeMap)
{
    return m_pDocument->DestroyNamedNodeMap(piNamedNodeMap);
}

PUBLIC VIRTUAL void DocumentImpl::SetNextSibling(INode* piNode)
{
    m_pDocument->SetNextSibling(piNode);
}

PUBLIC VIRTUAL IDocument* DocumentImpl::GetOwnerDocument() const
{
    return m_pDocument->GetOwnerDocument();
}

PUBLIC VIRTUAL INode* DocumentImpl::GetParentNode() const
{
    return m_pDocument->GetParentNode();
}

PUBLIC VIRTUAL const AString& DocumentImpl::GetPrefix() const
{
    return m_pDocument->GetPrefix();
}

PUBLIC VIRTUAL INode* DocumentImpl::GetPreviousSibling() const
{
    return m_pDocument->GetPreviousSibling();
}

PUBLIC VIRTUAL const AString& DocumentImpl::GetTextContent() const
{
    return m_pDocument->GetTextContent();
}

PUBLIC VIRTUAL IMS_BOOL DocumentImpl::HasAttribute() const
{
    return m_pDocument->HasAttribute();
}

PUBLIC VIRTUAL IMS_BOOL DocumentImpl::HasChildNode() const
{
    return m_pDocument->HasChildNode();
}

PUBLIC VIRTUAL INode* DocumentImpl::InsertBefore(IN INode* piNewChild, IN INode* piRefChild)
{
    return m_pDocument->InsertBefore(piNewChild, piRefChild);
}

PUBLIC VIRTUAL IMS_BOOL DocumentImpl::IsSupported(
        IN const AString& strFeature, IN const AString& strVersion)
{
    return m_pDocument->IsSupported(strFeature, strVersion);
}

PUBLIC VIRTUAL IMS_RESULT DocumentImpl::Normalize()
{
    return m_pDocument->Normalize();
}

PUBLIC VIRTUAL INode* DocumentImpl::RemoveChild(IN INode* piChild)
{
    return m_pDocument->RemoveChild(piChild);
}

PUBLIC VIRTUAL INode* DocumentImpl::ReplaceChild(IN INode* piNewChild, IN INode* piOldChild)
{
    return m_pDocument->ReplaceChild(piNewChild, piOldChild);
}

PUBLIC VIRTUAL void DocumentImpl::SetNodeValue(IN const AString& strNodeValue)
{
    m_pDocument->SetNodeValue(strNodeValue);
}

PUBLIC VIRTUAL void DocumentImpl::SetPrefix(IN const AString& strPrefix)
{
    m_pDocument->SetPrefix(strPrefix);
}

PUBLIC VIRTUAL void DocumentImpl::SetTextContent(IN const AString& strTextContext)
{
    m_pDocument->SetTextContent(strTextContext);
}

PUBLIC VIRTUAL void DocumentImpl::SetPreviousSibling(IN INode* piNode)
{
    m_pDocument->SetPreviousSibling(piNode);
}

PUBLIC VIRTUAL void DocumentImpl::SetParent(IN INode* piNode)
{
    return m_pDocument->SetParent(piNode);
}

PUBLIC VIRTUAL IMS_RESULT DocumentImpl::SetChildren(IN INode* piNode)
{
    return m_pDocument->SetChildren(piNode);
}

PUBLIC VIRTUAL void DocumentImpl::SetOwnerDocument(IN IDocument* piDocument)
{
    m_pDocument->SetOwnerDocument(piDocument);
}

PUBLIC VIRTUAL INode* DocumentImpl::AdoptNode(IN INode* piNode)
{
    return m_pDocument->AdoptNode(piNode);
}

PUBLIC VIRTUAL IAttr* DocumentImpl::CreateAttribute(IN const AString& strName)
{
    return m_pDocument->CreateAttribute(strName);
}

PUBLIC VIRTUAL IAttr* DocumentImpl::CreateAttributeNs(
        IN const AString& strNamespaceUri, IN const AString& strQualifiedName)
{
    return m_pDocument->CreateAttributeNs(strNamespaceUri, strQualifiedName);
}

PUBLIC VIRTUAL IElement* DocumentImpl::CreateElement(IN const AString& strTagName)
{
    return m_pDocument->CreateElement(strTagName);
}

PUBLIC VIRTUAL IElement* DocumentImpl::CreateElementNs(
        IN const AString& strNamespaceUri, IN const AString& strQualifiedName)
{
    return m_pDocument->CreateElementNs(strNamespaceUri, strQualifiedName);
}

PUBLIC VIRTUAL IText* DocumentImpl::CreateTextNode(IN const AString& strData)
{
    return m_pDocument->CreateTextNode(strData);
}

PUBLIC VIRTUAL IElement* DocumentImpl::GetDocumentElement() const
{
    return m_pDocument->GetDocumentElement();
}

PUBLIC VIRTUAL IElement* DocumentImpl::GetElementById(IN const AString& strElementId) const
{
    return m_pDocument->GetElementById(strElementId);
}

PUBLIC VIRTUAL INodeList* DocumentImpl::GetElementsByTagName(IN const AString& strTagName) const
{
    return m_pDocument->GetElementsByTagName(strTagName);
}

PUBLIC VIRTUAL INodeList* DocumentImpl::GetElementsByTagNameNs(
        IN const AString& strNamespaceUri, IN const AString& strLocalName) const
{
    return m_pDocument->GetElementsByTagNameNs(strNamespaceUri, strLocalName);
}

PUBLIC VIRTUAL INode* DocumentImpl::ImportNode(IN INode* piNode, IN IMS_BOOL bDeep)
{
    return m_pDocument->ImportNode(piNode, bDeep);
}

PUBLIC VIRTUAL void DocumentImpl::DestroyDocument()
{
    delete this;
}

PUBLIC VIRTUAL const AString& DocumentImpl::GetEncodingScheme() const
{
    return m_pDocument->GetEncodingScheme();
}

PUBLIC VIRTUAL const AString& DocumentImpl::GetUrl() const
{
    return m_pDocument->GetUrl();
}

PUBLIC VIRTUAL const AString& DocumentImpl::GetVersion() const
{
    return m_pDocument->GetVersion();
}
