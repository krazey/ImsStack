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
#include "ServiceTrace.h"
#include "TextParser.h"

#include "Sdp.h"
#include "SdpAttribute.h"
#include "offeranswer/SdpPrecondition.h"

__IMS_TRACE_TAG_SDP__;

PUBLIC GLOBAL const IMS_CHAR* SdpPrecondition::STR_DIRECTION_TAG[SdpPrecondition::DIRECTION_MAX] = {
        "none",
        "send",
        "recv",
        "sendrecv",
};

PUBLIC GLOBAL const IMS_CHAR* SdpPrecondition::STR_STATUS_TYPE[SdpPrecondition::STATUS_MAX] = {
        "e2e",
        "local",
        "remote",
};

PUBLIC GLOBAL const IMS_CHAR* SdpPrecondition::STR_STRENGTH_TAG[SdpPrecondition::STRENGTH_MAX] = {
        "mandatory",
        "optional",
        "none",
        "failure",
        "unknown",
        "",
};

PUBLIC GLOBAL const IMS_CHAR* SdpPrecondition::STR_TYPE[SdpPrecondition::TYPE_MAX] = {
        "qos",
        "",
};

PUBLIC
AString SdpPrecondition::DetailInfo::ToString() const
{
    AStringBuffer objBuffer(32);

    // strength-tag
    if ((m_nStrength > STRENGTH_INVALID) && (m_nStrength < STRENGTH_MAX))
    {
        if (m_nStrength != STRENGTH_NOTUSED)
        {
            objBuffer.Append(STR_STRENGTH_TAG[m_nStrength]);
            objBuffer.Append(TextParser::CHAR_SP);
        }
    }

    // status-type
    if ((m_nStatus > STATUS_INVALID) && (m_nStatus < STATUS_MAX))
    {
        objBuffer.Append(STR_STATUS_TYPE[m_nStatus]);
        objBuffer.Append(TextParser::CHAR_SP);
    }

    // direction-tag
    if ((m_nDirection > DIRECTION_INVALID) && (m_nDirection < DIRECTION_MAX))
    {
        objBuffer.Append(STR_DIRECTION_TAG[m_nDirection]);
    }

    return static_cast<const AStringBuffer&>(objBuffer).GetString();
}

PUBLIC
SdpPrecondition::SdpPrecondition(
        IN IMS_SINT32 nType /*= TYPE_QOS*/, IN IMS_SINT32 nSubType /*= SUBTYPE_E2E*/) :
        m_nType(nType),
        m_nSubType(nSubType)
{
}

PUBLIC
SdpPrecondition::SdpPrecondition(IN const SdpPrecondition& other) :
        m_nType(other.m_nType),
        m_nSubType(other.m_nSubType)
{
}

PUBLIC VIRTUAL SdpPrecondition::~SdpPrecondition() {}

PUBLIC
SdpPrecondition& SdpPrecondition::operator=(IN const SdpPrecondition& other)
{
    if (this != &other)
    {
        m_nType = other.m_nType;
        m_nSubType = other.m_nSubType;
    }

    return (*this);
}

PUBLIC VIRTUAL AString SdpPrecondition::ToSdp(IN IMS_SINT32 nAttribute) const
{
    // a=<attribute>:<value>

    if ((nAttribute != SdpAttribute::CURR) && (nAttribute != SdpAttribute::DES) &&
            (nAttribute != SdpAttribute::CONF))
    {
        return AString::ConstNull();
    }

    if ((GetType() <= TYPE_INVALID) || (GetType() >= TYPE_MAX))
    {
        return AString::ConstNull();
    }

    AStringBuffer objBuffer(16);

    objBuffer.Append(static_cast<const IMS_CHAR>(Sdp::LINE_A));
    objBuffer.Append(TextParser::CHAR_EQUAL);

    if (nAttribute == SdpAttribute::CURR)
    {
        objBuffer.Append("curr:");
    }
    else if (nAttribute == SdpAttribute::DES)
    {
        objBuffer.Append("des:");
    }
    else
    {
        objBuffer.Append("conf:");
    }

    // precondition-type
    objBuffer.Append(STR_TYPE[GetType()]);

    return static_cast<const AStringBuffer&>(objBuffer).GetString();
}

PUBLIC
IMS_BOOL SdpPrecondition::AddStatus(IN const AString& strStatusLine)
{
    IMS_SINT32 nType = TYPE_INVALID;
    IMS_SINT32 nSubType = SUBTYPE_SEGMENTED;
    DetailInfo objInfo;

    if (!ExtractProperties(strStatusLine, nType, nSubType, objInfo))
    {
        return IMS_FALSE;
    }

    if (nType != GetType())
    {
        IMS_TRACE_E(0, "Type(this=%d, status=%d) is not matched", GetType(), nType, 0);
        return IMS_FALSE;
    }

    if (nSubType != GetSubType())
    {
        IMS_TRACE_E(0, "SubType(this=%d, status=%d) is not matched", GetSubType(), nSubType, 0);
        return IMS_FALSE;
    }

    return AddStatus(objInfo.GetStatus(), objInfo.GetDirection(), objInfo.GetStrength());
}

PUBLIC
IMS_BOOL SdpPrecondition::ExtractProperties(IN const AString& strStatusLine, OUT IMS_SINT32& nType,
        OUT IMS_SINT32& nSubType, OUT SdpPrecondition::DetailInfo& objDetailInfo)
{
    if (strStatusLine.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    ImsList<AString> objTokens = strStatusLine.Split(TextParser::CHAR_SP);

    if ((objTokens.GetSize() != 3) && (objTokens.GetSize() != 4))
    {
        IMS_TRACE_E(0, "Invalid status (%s)", strStatusLine.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    // precondition-type
    if (objTokens.GetAt(0).EqualsIgnoreCase(STR_TYPE[TYPE_QOS]))
    {
        nType = TYPE_QOS;
    }
    else
    {
        nType = TYPE_OTHER;
    }

    IMS_SINT32 nIndexOfStatusType = 1;
    IMS_SINT32 nIndexOfDirectionTag = 2;

    if (objTokens.GetSize() == 4)
    {
        // "desired-status" attribute

        // strength-tag
        const AString& strStrengthTag = objTokens.GetAt(1);

        if (strStrengthTag.EqualsIgnoreCase(STR_STRENGTH_TAG[STRENGTH_MANDATORY]))
        {
            objDetailInfo.SetStrength(STRENGTH_MANDATORY);
        }
        else if (strStrengthTag.EqualsIgnoreCase(STR_STRENGTH_TAG[STRENGTH_OPTIONAL]))
        {
            objDetailInfo.SetStrength(STRENGTH_OPTIONAL);
        }
        else if (strStrengthTag.EqualsIgnoreCase(STR_STRENGTH_TAG[STRENGTH_NONE]))
        {
            objDetailInfo.SetStrength(STRENGTH_NONE);
        }
        else if (strStrengthTag.EqualsIgnoreCase(STR_STRENGTH_TAG[STRENGTH_FAILURE]))
        {
            objDetailInfo.SetStrength(STRENGTH_FAILURE);
        }
        else if (strStrengthTag.EqualsIgnoreCase(STR_STRENGTH_TAG[STRENGTH_UNKNOWN]))
        {
            objDetailInfo.SetStrength(STRENGTH_UNKNOWN);
        }
        else
        {
            IMS_TRACE_E(0, "Unknown strength-tag (%s) found", strStrengthTag.GetStr(), 0, 0);
            return IMS_FALSE;
        }

        nIndexOfStatusType = 2;
        nIndexOfDirectionTag = 3;
    }

    // status-type
    const AString& strStatusType = objTokens.GetAt(nIndexOfStatusType);

    if (strStatusType.EqualsIgnoreCase(STR_STATUS_TYPE[STATUS_E2E]))
    {
        objDetailInfo.SetStatus(STATUS_E2E);
        nSubType = SUBTYPE_E2E;
    }
    else if (strStatusType.EqualsIgnoreCase(STR_STATUS_TYPE[STATUS_LOCAL]))
    {
        objDetailInfo.SetStatus(STATUS_LOCAL);
        nSubType = SUBTYPE_SEGMENTED;
    }
    else if (strStatusType.EqualsIgnoreCase(STR_STATUS_TYPE[STATUS_REMOTE]))
    {
        objDetailInfo.SetStatus(STATUS_REMOTE);
        nSubType = SUBTYPE_SEGMENTED;
    }
    else
    {
        IMS_TRACE_E(0, "Unknown status-type (%s) found", strStatusType.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    // direction-tag
    const AString& strDirectionTag = objTokens.GetAt(nIndexOfDirectionTag);

    if (strDirectionTag.EqualsIgnoreCase(STR_DIRECTION_TAG[DIRECTION_NONE]))
    {
        objDetailInfo.SetDirection(DIRECTION_NONE);
    }
    else if (strDirectionTag.EqualsIgnoreCase(STR_DIRECTION_TAG[DIRECTION_SEND]))
    {
        objDetailInfo.SetDirection(DIRECTION_SEND);
    }
    else if (strDirectionTag.EqualsIgnoreCase(STR_DIRECTION_TAG[DIRECTION_RECV]))
    {
        objDetailInfo.SetDirection(DIRECTION_RECV);
    }
    else if (strDirectionTag.EqualsIgnoreCase(STR_DIRECTION_TAG[DIRECTION_SENDRECV]))
    {
        objDetailInfo.SetDirection(DIRECTION_SENDRECV);
    }
    else
    {
        IMS_TRACE_E(0, "Unknown direction-tag (%s) found", strDirectionTag.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}
