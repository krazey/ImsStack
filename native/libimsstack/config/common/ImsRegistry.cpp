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
#include "ServiceMemory.h"

#include "ImsRegistry.h"

PUBLIC
ImsRegistry::ImsRegistry() :
        m_objProperties(ImsList<AStringArray>())
{
}

PUBLIC
ImsRegistry::ImsRegistry(IN const ImsRegistry& other) :
        m_objProperties(other.m_objProperties)
{
}

PUBLIC
ImsRegistry& ImsRegistry::operator=(IN const ImsRegistry& other)
{
    if (this != &other)
    {
        m_objProperties = other.m_objProperties;
    }

    return (*this);
}

PUBLIC
IMS_BOOL ImsRegistry::Add(IN const AStringArray& objProperty)
{
    return m_objProperties.Append(objProperty);
}

PUBLIC
const AStringArray& ImsRegistry::GetAt(IN IMS_SINT32 i) const
{
    if (i < 0 || i >= GetCount())
    {
        return AStringArray::ConstNull();
    }

    if (m_objProperties.IsEmpty())
    {
        return AStringArray::ConstNull();
    }

    return m_objProperties.GetAt(i);
}

PUBLIC
IMS_SINT32 ImsRegistry::GetCount() const
{
    return static_cast<IMS_SINT32>(m_objProperties.GetSize());
}
