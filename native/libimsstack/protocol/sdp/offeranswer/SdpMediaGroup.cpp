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

#include "offeranswer/SdpMediaGroup.h"
#include "offeranswer/SdpMediaParameter.h"

PUBLIC
SdpMediaGroup::SdpMediaGroup() :
        m_nType(GROUP_OTHER),
        m_strType(AString::ConstNull())
{
}

PUBLIC
SdpMediaGroup::SdpMediaGroup(IN const SdpMediaGroup& other) :
        m_nType(other.m_nType),
        m_strType(other.m_strType),
        m_objMediaStreamIndexes(other.m_objMediaStreamIndexes)
{
}

PUBLIC
SdpMediaGroup::~SdpMediaGroup() {}

PUBLIC
SdpMediaGroup& SdpMediaGroup::operator=(IN const SdpMediaGroup& other)
{
    if (this != &other)
    {
        m_nType = other.m_nType;
        m_strType = other.m_strType;
        m_objMediaStreamIndexes = other.m_objMediaStreamIndexes;
    }

    return (*this);
}

PUBLIC
IMS_BOOL SdpMediaGroup::Create(
        IN const AString& strGroupAttribute, IN const ImsList<SdpMediaParameter*>& objMediaParams)
{
    ImsList<AString> objTokens = strGroupAttribute.Split(TextParser::CHAR_SP);

    if (objTokens.GetSize() < 1)
    {
        return IMS_FALSE;
    }

    // semantics : LS (Lip Synchronization), FID (Flow Identification)
    m_strType = objTokens.GetAt(0);

    if (m_strType.Equals("LS"))
    {
        m_nType = LS;
    }
    else if (m_strType.Equals("FID"))
    {
        m_nType = FID;
    }
    else if (m_strType.Equals("SRF"))
    {
        m_nType = SRF;
    }
    else
    {
        m_nType = GROUP_OTHER;
    }

    for (IMS_UINT32 i = 1; i < objTokens.GetSize(); ++i)
    {
        const AString& strIdTag = objTokens.GetAt(i);

        m_objMids.AddElement(strIdTag);

        for (IMS_UINT32 j = 0; j < objMediaParams.GetSize(); ++j)
        {
            const SdpMediaParameter* pMediaParam = objMediaParams.GetAt(j);

            if (strIdTag.Equals(pMediaParam->GetAttributeMid()))
            {
                m_objMediaStreamIndexes.Append(pMediaParam->GetMid());
                break;
            }
        }
    }

    if ((objTokens.GetSize() - 1) != m_objMediaStreamIndexes.GetSize())
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SdpMediaGroup::RemoveMediaStream(IN IMS_SINT32 nMid)
{
    for (IMS_UINT32 i = 0; i < m_objMediaStreamIndexes.GetSize(); ++i)
    {
        if (nMid == m_objMediaStreamIndexes.GetAt(i))
        {
            m_objMediaStreamIndexes.RemoveAt(i);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
AString SdpMediaGroup::ToSdp() const
{
    AString strGroupAttribute;

    strGroupAttribute.Append("a=group:");
    strGroupAttribute.Append(m_strType);

    for (IMS_SINT32 i = 0; i < m_objMids.GetCount(); ++i)
    {
        strGroupAttribute.Append(TextParser::CHAR_SP);
        strGroupAttribute.Append(m_objMids.GetElementAt(i));
    }

    strGroupAttribute.Append(TextParser::CHAR_CR);
    strGroupAttribute.Append(TextParser::CHAR_LF);

    return strGroupAttribute;
}
