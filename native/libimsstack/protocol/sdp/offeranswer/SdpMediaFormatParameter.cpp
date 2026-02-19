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

#include "SdpAttribute.h"
#include "offeranswer/SdpMediaFormatParameter.h"

PUBLIC
SdpMediaFormatParameter::SdpMediaFormatParameter(
        IN IMS_SINT32 nAttribute, IN IMS_SINT32 nPayloadTypeNumber) :
        m_nAttribute(nAttribute),
        m_nPayloadTypeNumber(nPayloadTypeNumber)
{
}

PUBLIC
SdpMediaFormatParameter::SdpMediaFormatParameter(IN const SdpMediaFormatParameter& other) :
        m_nAttribute(other.m_nAttribute),
        m_nPayloadTypeNumber(other.m_nPayloadTypeNumber)
{
}

PUBLIC VIRTUAL SdpMediaFormatParameter::~SdpMediaFormatParameter() {}

PUBLIC
SdpMediaFormatParameter& SdpMediaFormatParameter::operator=(IN const SdpMediaFormatParameter& other)
{
    if (this != &other)
    {
        m_nAttribute = other.m_nAttribute;
        m_nPayloadTypeNumber = other.m_nPayloadTypeNumber;
    }

    return (*this);
}

PUBLIC VIRTUAL SdpMediaFormatParameter* SdpMediaFormatParameter::Clone() const
{
    return new SdpMediaFormatParameter(*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpMediaFormatParameter::Equals(
        IN const SdpMediaFormatParameter* pParameter) const
{
    if (pParameter == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_nAttribute != pParameter->m_nAttribute)
    {
        return IMS_FALSE;
    }

    if (m_nPayloadTypeNumber != pParameter->m_nPayloadTypeNumber)
    {
        // 4 Check if the existing payload type number is wildcard or not.
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SdpMediaFormatParameter::SetValue(IN const AString& /*strValue*/)
{
    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpMediaFormatParameter::ToSdp() const
{
    const IMS_CHAR* pszAttributeName = SdpAttribute::GetAttributeName(m_nAttribute);

    if (pszAttributeName == IMS_NULL)
    {
        return AString::ConstNull();
    }

    AString strPayloadTypeNumber;

    if (m_nPayloadTypeNumber == PT_WILDCARD)
    {
        strPayloadTypeNumber.Sprintf("a=%s:*", pszAttributeName);
    }
    else if (m_nPayloadTypeNumber == PT_NOT_SPECIFIED)
    {
        strPayloadTypeNumber.Sprintf("a=%s:", pszAttributeName);
    }
    else
    {
        strPayloadTypeNumber.Sprintf("a=%s:%d", pszAttributeName, m_nPayloadTypeNumber);
    }

    return strPayloadTypeNumber;
}
