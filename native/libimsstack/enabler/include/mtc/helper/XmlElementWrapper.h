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

#ifndef XML_ELEMENT_WRAPPER_H_
#define XML_ELEMENT_WRAPPER_H_

#include "ImsTypeDef.h"

class AString;
class IElement;
class INodeList;

class XmlElementWrapper final
{
public:
    explicit XmlElementWrapper(IN IElement* pElement);
    ~XmlElementWrapper();

    inline IMS_BOOL IsValid() const { return m_pElement != IMS_NULL; }

    const AString& GetTagName() const;
    const AString& GetValue() const;

    XmlElementWrapper GetFirstChild(IN const AString& strTagName) const;
    XmlElementWrapper GetFirstChild() const;
    XmlElementWrapper GetNextSibling() const;

private:
    IElement* m_pElement;
};

#endif
