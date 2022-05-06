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

#include "Sdp.h"
#include "offeranswer/SdpMediaFormat.h"
#include "offeranswer/SdpMediaFormatParameter.h"

PUBLIC
SdpMediaFormat::SdpMediaFormat(IN IMS_SINT32 nType /*= TYPE_OTHER*/) :
        m_nType(nType),
        m_strValue(AString::ConstNull())
{
}

PUBLIC
SdpMediaFormat::SdpMediaFormat(IN const SdpMediaFormat& other) :
        m_nType(other.m_nType),
        m_strValue(other.m_strValue)
{
    for (IMS_UINT32 i = 0; i < other.m_objExtraParameters.GetSize(); ++i)
    {
        const SdpMediaFormatParameter* pParameter = other.m_objExtraParameters.GetAt(i);

        if (pParameter != IMS_NULL)
        {
            m_objExtraParameters.Append(pParameter->Clone());
        }
    }
}

PUBLIC VIRTUAL SdpMediaFormat::~SdpMediaFormat()
{
    for (IMS_UINT32 i = 0; i < m_objExtraParameters.GetSize(); ++i)
    {
        SdpMediaFormatParameter* pParameter = m_objExtraParameters.GetAt(i);

        if (pParameter != IMS_NULL)
        {
            delete pParameter;
        }
    }

    m_objExtraParameters.Clear();
}

PUBLIC
SdpMediaFormat& SdpMediaFormat::operator=(IN const SdpMediaFormat& other)
{
    if (this != &other)
    {
        m_nType = other.m_nType;
        m_strValue = other.m_strValue;

        for (IMS_UINT32 i = 0; i < m_objExtraParameters.GetSize(); ++i)
        {
            SdpMediaFormatParameter* pParameter = m_objExtraParameters.GetAt(i);

            if (pParameter != IMS_NULL)
            {
                delete pParameter;
            }
        }

        m_objExtraParameters.Clear();

        for (IMS_UINT32 i = 0; i < other.m_objExtraParameters.GetSize(); ++i)
        {
            const SdpMediaFormatParameter* pParameter = other.m_objExtraParameters.GetAt(i);

            if (pParameter != IMS_NULL)
            {
                m_objExtraParameters.Append(pParameter->Clone());
            }
        }
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpMediaFormat::Equals(IN const SdpMediaFormat* pFormat) const
{
    if (pFormat == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_nType != pFormat->m_nType)
    {
        return IMS_FALSE;
    }

    if (!m_strValue.EqualsIgnoreCase(pFormat->m_strValue))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SdpMediaFormat::SetValue(IN const AString& strFormat)
{
    if (!Sdp::IsTokenString(strFormat))
    {
        return IMS_FALSE;
    }

    m_strValue = strFormat;

    return IMS_TRUE;
}

PUBLIC
void SdpMediaFormat::AddExtraParameter(IN SdpMediaFormatParameter* pParameter)
{
    if (pParameter != IMS_NULL)
    {
        m_objExtraParameters.Append(pParameter);
    }
}
