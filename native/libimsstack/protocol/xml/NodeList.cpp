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
#include "INode.h"
#include "NodeList.h"

PUBLIC
NodeList::NodeList() {}

PUBLIC VIRTUAL NodeList::~NodeList() {}

PUBLIC VIRTUAL IMS_SINT32 NodeList::GetLength() const
{
    return m_objNodes.GetSize();
}

PUBLIC VIRTUAL INode* NodeList::Item(IN IMS_SINT32 nIndex) const
{
    if ((nIndex < 0) || (nIndex >= GetLength()))
    {
        return IMS_NULL;
    }

    if (m_objNodes.IsEmpty())
    {
        return IMS_NULL;
    }

    return m_objNodes.GetAt(nIndex);
}

PUBLIC
IMS_RESULT NodeList::AddNode(IN INode* piNode)
{
    if (piNode == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!m_objNodes.Append(piNode))
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}
