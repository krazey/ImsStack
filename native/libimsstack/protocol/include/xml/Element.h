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
#ifndef ELEMENT_H_
#define ELEMENT_H_

#include "Node.h"

class IAttr;
class IElement;

class Element : public Node
{
public:
    Element(IN xmlNodePtr pstNode, IN IElement* piElement);
    ~Element() override;

    Element(IN const Element&) = delete;
    Element& operator=(IN const Element&) = delete;

    // Element
    virtual const AString& GetAttribute(IN const AString& strName) const;
    virtual IAttr* GetAttributeNode(IN const AString& strName) const;
    virtual IAttr* GetAttributeNodeNs(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) const;
    virtual const AString& GetAttributeNs(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) const;
    virtual INodeList* GetElementsByTagName(IN const AString& strName) const;
    virtual INodeList* GetElementsByTagNameNs(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) const;
    virtual const AString& GetTagName() const;
    virtual IMS_BOOL HasAttributeB(IN const AString& strName) const;
    virtual IMS_BOOL HasAttributeNs(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) const;
    virtual void RemoveAttribute(IN const AString& strName);
    virtual IAttr* RemoveAttributeNode(IN IAttr* piAttr);
    virtual void RemoveAttributeNs(
            IN const AString& strNamespaceUri, IN const AString& strLocalName);
    virtual void SetAttribute(IN const AString& strName, IN const AString& strValue);
    virtual IAttr* SetAttributeNode(IN IAttr* piAttr);
    virtual IAttr* SetAttributeNodeNs(IN IAttr* piAttr);
    virtual void SetAttributeNs(IN const AString& strNamespaceUri,
            IN const AString& strQualifiedName, IN const AString& strValue);
    virtual void SetIdAttribute(IN const AString& strName, IN IMS_BOOL bIsId);
    virtual void SetIdAttributeNode(IN IAttr* piIdAttr, IN IMS_BOOL bIsId);
    virtual void SetIdAttributeNodeNs(
            IN const AString& strNamespaceUri, IN const AString& strLocalName, IN IMS_BOOL bIsId);

    // Element: extensions
    virtual const AString& GetAttribute(IN const IMS_CHAR* pszName) const;
    virtual IAttr* GetAttributeNode(IN const IMS_CHAR* pszName) const;
    virtual INodeList* GetElementsByTagName(IN const IMS_CHAR* pszName) const;
};

#endif
