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

#include "SdpVersion.h"

PUBLIC
SdpVersion::SdpVersion() :
        SdpLine(),
        m_nVersion(SDP_VERSION)
{
}

PUBLIC
SdpVersion::SdpVersion(IN const SdpVersion& other) :
        SdpLine(other),
        m_nVersion(other.m_nVersion)
{
}

PUBLIC VIRTUAL SdpVersion::~SdpVersion() {}

PUBLIC
SdpVersion& SdpVersion::operator=(IN const SdpVersion& other)
{
    if (this != &other)
    {
        SdpLine::operator=(other);

        m_nVersion = other.m_nVersion;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpVersion::Decode(IN const AString& strValue)
{
    // v=<version>
    IMS_BOOL bOk = IMS_FALSE;

    m_nVersion = strValue.ToInt32(&bOk);

    if (!bOk)
    {
        // Invalid SDP version
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpVersion::Encode() const
{
    // v=<version>
    AString strLine;

    strLine.Sprintf("v=%u\r\n", m_nVersion);

    return strLine;
}

PUBLIC VIRTUAL AString SdpVersion::GetValue() const
{
    AString strValue;

    return strValue.SetNumber(m_nVersion);
}

PUBLIC
IMS_BOOL SdpVersion::SetVersion(IN IMS_SINT32 nVersion /*= SDP_VERSION*/)
{
    m_nVersion = nVersion;

    if (m_nVersion != SDP_VERSION)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}
