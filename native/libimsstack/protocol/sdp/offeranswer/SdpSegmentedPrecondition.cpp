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

#include "offeranswer/SdpSegmentedPrecondition.h"

PUBLIC
SdpSegmentedPrecondition::SdpSegmentedPrecondition(IN IMS_SINT32 nType /*= TYPE_QOS*/) :
        SdpPrecondition(nType, SUBTYPE_SEGMENTED)
{
}

PUBLIC
SdpSegmentedPrecondition::SdpSegmentedPrecondition(IN const SdpSegmentedPrecondition& other) :
        SdpPrecondition(other),
        m_objLocalDetails(other.m_objLocalDetails),
        m_objRemoteDetails(other.m_objRemoteDetails)
{
}

PUBLIC VIRTUAL SdpSegmentedPrecondition::~SdpSegmentedPrecondition() {}

PUBLIC
SdpSegmentedPrecondition& SdpSegmentedPrecondition::operator=(
        IN const SdpSegmentedPrecondition& other)
{
    if (this != &other)
    {
        SdpPrecondition::operator=(other);

        m_objLocalDetails = other.m_objLocalDetails;
        m_objRemoteDetails = other.m_objRemoteDetails;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpSegmentedPrecondition::AddStatus(IN IMS_SINT32 nStatus,
        IN IMS_SINT32 nDirection, IN IMS_SINT32 nStrength /*= STRENGTH_NOTUSED*/)
{
    if (nStatus == STATUS_LOCAL)
    {
        DetailInfo objInfo(nStatus, nDirection, nStrength);

        m_objLocalDetails.Append(objInfo);
    }
    else if (nStatus == STATUS_REMOTE)
    {
        DetailInfo objInfo(nStatus, nDirection, nStrength);

        m_objRemoteDetails.Append(objInfo);
    }
    else
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SdpSegmentedPrecondition::IsPreconditionPresent() const
{
    if (!m_objLocalDetails.IsEmpty() || !m_objRemoteDetails.IsEmpty())
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL AString SdpSegmentedPrecondition::ToSdp(IN IMS_SINT32 nAttribute) const
{
    // a=<attribute>:<value>
    AString strPrefix = SdpPrecondition::ToSdp(nAttribute);

    if (strPrefix.IsNULL())
    {
        return AString::ConstNull();
    }

    AStringBuffer objBuffer(128);

    // 'local' status
    for (IMS_UINT32 i = 0; i < m_objLocalDetails.GetSize(); ++i)
    {
        const DetailInfo& objInfo = m_objLocalDetails.GetAt(i);

        // a=xxxx:qos
        objBuffer.Append(strPrefix);

        // SP
        objBuffer.Append(TextParser::CHAR_SP);

        // [strength-tag] SP status-type SP directiont-tag
        objBuffer.Append(objInfo.ToString());

        objBuffer.Append(TextParser::CHAR_CR);
        objBuffer.Append(TextParser::CHAR_LF);
    }

    // 'remote' status
    for (IMS_UINT32 i = 0; i < m_objRemoteDetails.GetSize(); ++i)
    {
        const DetailInfo& objInfo = m_objRemoteDetails.GetAt(i);

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
