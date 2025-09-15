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
#ifndef ATTR_H_
#define ATTR_H_

#include "Node.h"

class IElement;

class Attr : public Node
{
public:
    Attr(IN xmlAttrPtr pstAttr, IN IElement* piOwnerElement);
    ~Attr() override;

    Attr(IN const Attr&) = delete;
    Attr& operator=(IN const Attr&) = delete;

    // Node
    void SetNextSibling(IN INode* piNode) override;
    void SetPreviousSibling(IN INode* piNode) override;
    void SetParent(IN INode* piNode) override;
    IMS_RESULT SetChildren(IN INode* piNode) override;

    // Attr
    virtual const AString& GetName() const;
    virtual IElement* GetOwnerElement() const;
    virtual IMS_BOOL GetSpecified() const;
    virtual const AString& GetValue() const;
    virtual IMS_BOOL IsId() const;
    virtual void SetValue(IN const AString& strValue);

private:
    IElement* m_piOwnerElement;
    IMS_BOOL m_bSpecified;
};

#endif
