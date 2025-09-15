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
#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include "Node.h"

class IAttr;
class IElement;
class IText;

class Document : public Node
{
public:
    Document(IN xmlDocPtr pstDoc, IN xmlXPathContextPtr pstXpathContext);
    ~Document() override;

    Document(IN const Document&) = delete;
    Document& operator=(IN const Document&) = delete;

    // Document
    virtual INode* AdoptNode(IN INode* piNode);
    virtual IAttr* CreateAttribute(IN const AString& strName);
    virtual IAttr* CreateAttributeNs(
            IN const AString& strNamespaceUri, IN const AString& strQualifiedName);
    virtual IElement* CreateElement(IN const AString& strTagName);
    virtual IElement* CreateElementNs(
            IN const AString& strNamespaceUri, IN const AString& strQualifiedName);
    virtual IText* CreateTextNode(IN const AString& strData);
    virtual IElement* GetDocumentElement() const;
    virtual IElement* GetElementById(IN const AString& strElementId) const;

    // Notice: INodeList* MUST be freed after calling these methods.
    virtual INodeList* GetElementsByTagName(IN const AString& strTagName) const;
    virtual INodeList* GetElementsByTagNameNs(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) const;
    virtual INode* ImportNode(IN INode* piNode, IN IMS_BOOL bDeep);

    virtual const AString& GetEncodingScheme() const;
    virtual const AString& GetUrl() const;
    virtual const AString& GetVersion() const;

private:
    AString m_strVersion;
    AString m_strEncoding;
    AString m_strUrl;
    xmlDocPtr m_pstDoc;
    xmlXPathContextPtr m_pstXpathContext;
};

#endif
