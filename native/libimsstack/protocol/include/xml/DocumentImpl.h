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
#ifndef DOCUMENT_IMPL_H_
#define DOCUMENT_IMPL_H_

#include "IDocument.h"
#include "XmlApiTree.h"
#include "XmlApiXPath.h"

class Document;

class DocumentImpl : public IDocument
{
public:
    DocumentImpl(IN xmlDocPtr pstDoc, IN xmlXPathContextPtr pstXpathContext);
    ~DocumentImpl() override;

    DocumentImpl(IN const DocumentImpl&) = delete;
    DocumentImpl& operator=(IN const DocumentImpl&) = delete;

    // INode
    INode* AppendChild(IN INode* piChild) override;
    INode* CloneNode(IN IMS_BOOL bDeep) override;
    INamedNodeMap* GetAttributes() const override;
    INodeList* GetChildNodes() const override;
    INode* GetFirstChild() const override;
    INode* GetLastChild() const override;
    const AString& GetLocalName() const override;
    const AString& GetNameSpaceUri() const override;
    INode* GetNextSibling() const override;
    const AString& GetNodeName() const override;
    IMS_SINT32 GetNodeType() const override;
    const AString& GetNodeValue() const override;
    IDocument* GetOwnerDocument() const override;
    INode* GetParentNode() const override;
    const AString& GetPrefix() const override;
    INode* GetPreviousSibling() const override;
    const AString& GetTextContent() const override;
    IMS_BOOL HasAttribute() const override;
    IMS_BOOL HasChildNode() const override;
    INode* InsertBefore(IN INode* piNewChild, IN INode* piRefChild) override;
    IMS_BOOL IsSupported(IN const AString& strFeature, IN const AString& strVersion) override;
    IMS_RESULT Normalize() override;
    INode* RemoveChild(IN INode* piChild) override;
    INode* ReplaceChild(IN INode* piNewChild, IN INode* piOldChild) override;
    void SetNodeValue(IN const AString& strNodeValue) override;
    void SetPrefix(IN const AString& strPrefix) override;
    void SetTextContent(IN const AString& strTextContext) override;

    // INode: extensions
    void DestroyNodeList(IN INodeList*& piNodeList) override;
    void DestroyNamedNodeMap(IN INamedNodeMap*& piNamedNodeMap) override;
    void SetNextSibling(IN INode* piNode) override;
    void SetPreviousSibling(IN INode* piNode) override;
    void SetParent(IN INode* piNode) override;
    IMS_RESULT SetChildren(IN INode* piNode) override;
    void SetOwnerDocument(IN IDocument* piDocument) override;

    // IDocument
    INode* AdoptNode(IN INode* piNode) override;
    IAttr* CreateAttribute(IN const AString& strName) override;
    IAttr* CreateAttributeNs(
            IN const AString& strNamespaceUri, IN const AString& strQualifiedName) override;
    IElement* CreateElement(IN const AString& strTagName) override;
    IElement* CreateElementNs(
            IN const AString& strNamespaceUri, IN const AString& strQualifiedName) override;
    IText* CreateTextNode(IN const AString& strData) override;
    IElement* GetDocumentElement() const override;
    IElement* GetElementById(IN const AString& strElementId) const override;
    INodeList* GetElementsByTagName(IN const AString& strTagName) const override;
    INodeList* GetElementsByTagNameNs(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) const override;
    INode* ImportNode(IN INode* piNode, IN IMS_BOOL bDeep) override;
    void DestroyDocument() override;
    const AString& GetEncodingScheme() const override;
    const AString& GetUrl() const override;
    const AString& GetVersion() const override;

private:
    Document* m_pDocument;
};

#endif
