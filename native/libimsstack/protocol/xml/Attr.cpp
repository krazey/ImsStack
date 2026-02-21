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
#include "AString.h"

#include "Attr.h"
#include "IElement.h"

PUBLIC
Attr::Attr(IN xmlAttrPtr pstAttr, IN IElement* piOwnerElement) :
        Node(pstAttr),
        m_piOwnerElement(piOwnerElement),
        m_bSpecified(IMS_FALSE)
{
}

PUBLIC VIRTUAL Attr::~Attr() {}

PUBLIC VIRTUAL void Attr::SetNextSibling(IN INode* piNode)
{
    (void)piNode;
}

PUBLIC VIRTUAL void Attr::SetPreviousSibling(IN INode* piNode)
{
    (void)piNode;
}

PUBLIC VIRTUAL void Attr::SetParent(IN INode* piNode)
{
    (void)piNode;
}

PUBLIC VIRTUAL IMS_RESULT Attr::SetChildren(IN INode* piNode)
{
    (void)piNode;
    return IMS_FAILURE;
}

PUBLIC VIRTUAL const AString& Attr::GetName() const
{
    return GetLocalName();
}

PUBLIC VIRTUAL IElement* Attr::GetOwnerElement() const
{
    return m_piOwnerElement;
}

PUBLIC VIRTUAL IMS_BOOL Attr::GetSpecified() const
{
    return m_bSpecified;
}

PUBLIC VIRTUAL const AString& Attr::GetValue() const
{
    return GetNodeValue();
}

PUBLIC VIRTUAL IMS_BOOL Attr::IsId() const
{
    return m_bSpecified;
}

PUBLIC VIRTUAL void Attr::SetValue(IN const AString& strValue)
{
    SetNodeValue(strValue);
}
