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
#include "NamedNodeMap.h"

PUBLIC
NamedNodeMap::NamedNodeMap() {}

PUBLIC VIRTUAL NamedNodeMap::~NamedNodeMap() {}

PUBLIC VIRTUAL IMS_SINT32 NamedNodeMap::GetLength() const
{
    return m_objNamedItems.GetSize();
}

PUBLIC VIRTUAL INode* NamedNodeMap::GetNamedItem(IN const AString& strName) const
{
    if (strName.GetLength() == 0)
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < m_objNamedItems.GetSize(); i++)
    {
        INode* piNode = m_objNamedItems.GetAt(i);

        if (strName.EqualsIgnoreCase(piNode->GetLocalName()))
        {
            return piNode;
        }
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL INode* NamedNodeMap::GetNamedItemNs(
        IN const AString& strNamespaceUri, IN const AString& strName) const
{
    if ((strNamespaceUri.GetLength() == 0) || (strName.GetLength() == 0))
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < m_objNamedItems.GetSize(); i++)
    {
        INode* piNode = m_objNamedItems.GetAt(i);

        if (strName.EqualsIgnoreCase(piNode->GetLocalName()) &&
                strNamespaceUri.EqualsIgnoreCase(piNode->GetNameSpaceUri()))
        {
            return piNode;
        }
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL INode* NamedNodeMap::Item(IN IMS_SINT32 nIndex) const
{
    if ((nIndex < 0) || (nIndex >= GetLength()))
    {
        return IMS_NULL;
    }

    if (m_objNamedItems.IsEmpty())
    {
        return IMS_NULL;
    }

    return m_objNamedItems.GetAt(nIndex);
}

PUBLIC VIRTUAL INode* NamedNodeMap::RemoveNamedItem(IN const AString& strName)
{
    (void)strName;
    return IMS_NULL;
}

PUBLIC VIRTUAL INode* NamedNodeMap::RemoveNamedItemNs(
        IN const AString& strNamespaceUri, IN const AString& strLocalName)
{
    (void)strNamespaceUri;
    (void)strLocalName;
    return IMS_NULL;
}

PUBLIC VIRTUAL INode* NamedNodeMap::SetNamedItem(IN INode* piNode)
{
    (void)piNode;
    return IMS_NULL;
}

PUBLIC VIRTUAL INode* NamedNodeMap::SetNamedItemNs(IN INode* piNode)
{
    (void)piNode;
    return IMS_NULL;
}

PUBLIC
IMS_RESULT NamedNodeMap::AddNamedItem(IN INode* piNode)
{
    if (piNode == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!m_objNamedItems.Append(piNode))
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}
