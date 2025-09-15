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
#ifndef ATTR_IMPL_H_
#define ATTR_IMPL_H_

#include "IAttr.h"
#include "XmlApiTree.h"

class Attr;
class IDocument;
class IElement;

class AttrImpl : public IAttr
{
public:
    AttrImpl(IN xmlAttrPtr pstAttr, IN IElement* piOwnerElement);
    ~AttrImpl() override;

    AttrImpl(IN const AttrImpl&) = delete;
    AttrImpl& operator=(IN const AttrImpl&) = delete;

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

    // IAttr
    const AString& GetName() const override;
    IElement* GetOwnerElement() const override;
    IMS_BOOL GetSpecified() const override;
    const AString& GetValue() const override;
    IMS_BOOL IsId() const override;
    void SetValue(IN const AString& strValue) override;

private:
    Attr* m_pAttr;
};

#endif
