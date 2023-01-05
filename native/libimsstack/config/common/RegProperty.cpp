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

#include "RegProperty.h"

PUBLIC
RegProperty::RegProperty(IN const AString& strServiceId) :
        ImsProperty(ImsProperty::PKEY_REG),
        m_strServiceId(strServiceId)
{
}

PUBLIC
RegProperty::RegProperty(IN const RegProperty& other) :
        ImsProperty(other),
        m_strServiceId(other.m_strServiceId),
        m_objHeaders(other.m_objHeaders)
{
}

PUBLIC
RegProperty& RegProperty::operator=(IN const RegProperty& other)
{
    if (this != &other)
    {
        ImsProperty::operator=(other);

        m_strServiceId = other.m_strServiceId;
        m_objHeaders = other.m_objHeaders;
    }

    return (*this);
}

PROTECTED VIRTUAL IMS_BOOL RegProperty::Equals(IN const ImsProperty& objOther) const
{
    const RegProperty& objRegOther = DYNAMIC_CAST(const RegProperty&, objOther);

    if (m_nKey != objRegOther.m_nKey)
    {
        return IMS_FALSE;
    }

    if (!m_strServiceId.Equals(objRegOther.m_strServiceId))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}
