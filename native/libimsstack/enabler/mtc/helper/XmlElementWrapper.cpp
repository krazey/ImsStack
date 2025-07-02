/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "AString.h"
#include "IDocument.h"
#include "IElement.h"
#include "INode.h"
#include "INodeList.h"
#include "helper/XmlElementWrapper.h"

PUBLIC
XmlElementWrapper::XmlElementWrapper(IN IElement* pElement) :
        m_pElement(pElement)
{
}

PUBLIC
XmlElementWrapper::~XmlElementWrapper() {}

PUBLIC
const AString& XmlElementWrapper::GetTagName() const
{
    if (!IsValid())
    {
        return AString::ConstNull();
    }

    return m_pElement->GetTagName();
}

PUBLIC
const AString& XmlElementWrapper::GetValue() const
{
    if (!IsValid())
    {
        return AString::ConstNull();
    }

    const INode* pValueNode = m_pElement->GetFirstChild();
    if (pValueNode == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return pValueNode->GetNodeValue();
}

PUBLIC
XmlElementWrapper XmlElementWrapper::GetFirstChild(IN const AString& strTagName) const
{
    if (!IsValid())
    {
        return *this;
    }

    const INodeList* pChilds = m_pElement->GetElementsByTagName(strTagName);
    if (pChilds == IMS_NULL)
    {
        return XmlElementWrapper(IMS_NULL);
    }

    INode* pChild = pChilds->Item(0);
    return XmlElementWrapper(DYNAMIC_CAST(IElement*, pChild));
}

PUBLIC
XmlElementWrapper XmlElementWrapper::GetFirstChild() const
{
    if (!IsValid())
    {
        return *this;
    }

    return XmlElementWrapper(DYNAMIC_CAST(IElement*, m_pElement->GetFirstChild()));
}

PUBLIC
XmlElementWrapper XmlElementWrapper::GetNextSibling() const
{
    if (!IsValid())
    {
        return *this;
    }

    return XmlElementWrapper(DYNAMIC_CAST(IElement*, m_pElement->GetNextSibling()));
}
