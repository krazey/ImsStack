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
#ifndef NAEMD_NODE_MAP_H_
#define NAEMD_NODE_MAP_H_

#include "INamedNodeMap.h"
#include "ImsList.h"

class NamedNodeMap : public INamedNodeMap
{
public:
    NamedNodeMap();
    ~NamedNodeMap() override;

    NamedNodeMap(IN const NamedNodeMap&) = delete;
    NamedNodeMap& operator=(IN const NamedNodeMap&) = delete;

    // INamedNodeMap
    IMS_SINT32 GetLength() const override;
    INode* GetNamedItem(IN const AString& strName) const override;
    INode* GetNamedItemNs(
            IN const AString& strNamespaceUri, IN const AString& strName) const override;
    INode* Item(IN IMS_SINT32 nIndex) const override;
    INode* RemoveNamedItem(IN const AString& strName) override;
    INode* RemoveNamedItemNs(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) override;
    INode* SetNamedItem(IN INode* piNode) override;
    INode* SetNamedItemNs(IN INode* piNode) override;

public:
    IMS_RESULT AddNamedItem(IN INode* piNode);

private:
    ImsList<INode*> m_objNamedItems;
};

#endif
