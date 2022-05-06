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
#include "AStringBuffer.h"
#include "ServiceMemory.h"
#include "TextParser.h"

#include "offeranswer/SdpE2EPrecondition.h"

PUBLIC
SdpE2EPrecondition::SdpE2EPrecondition(IN IMS_SINT32 nType /*= TYPE_QOS*/) :
        SdpPrecondition(nType, SUBTYPE_E2E)
{
}

PUBLIC
SdpE2EPrecondition::SdpE2EPrecondition(IN const SdpE2EPrecondition& other) :
        SdpPrecondition(other),
        m_objE2EDetails(other.m_objE2EDetails)
{
}

PUBLIC VIRTUAL SdpE2EPrecondition::~SdpE2EPrecondition() {}

PUBLIC
SdpE2EPrecondition& SdpE2EPrecondition::operator=(IN const SdpE2EPrecondition& other)
{
    if (this != &other)
    {
        SdpPrecondition::operator=(other);

        m_objE2EDetails = other.m_objE2EDetails;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpE2EPrecondition::AddStatus(IN IMS_SINT32 nStatus,
        IN IMS_SINT32 nDirection, IN IMS_SINT32 nStrength /* = STRENGTH_NOTUSED */)
{
    if (nStatus == STATUS_E2E)
    {
        DetailInfo objInfo(nStatus, nDirection, nStrength);

        m_objE2EDetails.Append(objInfo);
    }
    else
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpE2EPrecondition::ToSdp(IN IMS_SINT32 nAttribute) const
{
    // a=<attribute>:<value>
    AString strPrefix = SdpPrecondition::ToSdp(nAttribute);

    if (strPrefix.IsNULL())
    {
        return AString::ConstNull();
    }

    AStringBuffer objBuffer(128);

    for (IMS_UINT32 i = 0; i < m_objE2EDetails.GetSize(); ++i)
    {
        const DetailInfo& objInfo = m_objE2EDetails.GetAt(i);

        // a=xxxx:qos
        objBuffer.Append(strPrefix);

        // SP
        objBuffer.Append(TextParser::CHAR_SP);

        // [strength-tag] SP status-type SP directiont-tag
        objBuffer.Append(objInfo.ToString());

        objBuffer.Append(TextParser::CHAR_CR);
        objBuffer.Append(TextParser::CHAR_LF);
    }

    return static_cast<const AStringBuffer&>(objBuffer).GetString();
}
