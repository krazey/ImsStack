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
#include "AStringArray.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "SdpDescription.h"
#include "SdpEncryptionKey.h"
#include "SdpInformation.h"

__IMS_TRACE_TAG_SDP__;

PUBLIC
SdpDescription::SdpDescription() :
        m_pInformation(IMS_NULL),
        m_pEncryptionKey(IMS_NULL)
{
    for (IMS_SINT32 i = 0; i < Sdp::TYPE_MAX; ++i)
    {
        m_abLineContains[i] = IMS_FALSE;
    }
}

PUBLIC
SdpDescription::SdpDescription(IN const SdpDescription& other) :
        m_pInformation(IMS_NULL),
        m_pEncryptionKey(IMS_NULL)
{
    for (IMS_SINT32 i = 0; i < Sdp::TYPE_MAX; ++i)
    {
        m_abLineContains[i] = other.m_abLineContains[i];
    }

    if (other.m_pInformation != IMS_NULL)
    {
        m_pInformation = new SdpInformation(*(other.m_pInformation));
    }

    if (other.m_pEncryptionKey != IMS_NULL)
    {
        m_pEncryptionKey = new SdpEncryptionKey(*(other.m_pEncryptionKey));
    }

    m_objBandwidths = other.m_objBandwidths;
    m_objAttributes = other.m_objAttributes;
}

PUBLIC VIRTUAL SdpDescription::~SdpDescription()
{
    if (m_pInformation != IMS_NULL)
    {
        delete m_pInformation;
    }

    if (m_pEncryptionKey != IMS_NULL)
    {
        delete m_pEncryptionKey;
    }

    m_objAttributes.Clear();
    m_objBandwidths.Clear();
}

PUBLIC
SdpDescription& SdpDescription::operator=(IN const SdpDescription& other)
{
    if (this != &other)
    {
        for (IMS_SINT32 i = 0; i < Sdp::TYPE_MAX; ++i)
        {
            m_abLineContains[i] = other.m_abLineContains[i];
        }

        if (m_pInformation != IMS_NULL)
        {
            delete m_pInformation;
            m_pInformation = IMS_NULL;
        }

        if (other.m_pInformation != IMS_NULL)
        {
            m_pInformation = new SdpInformation(*(other.m_pInformation));
        }

        if (m_pEncryptionKey != IMS_NULL)
        {
            delete m_pEncryptionKey;
            m_pEncryptionKey = IMS_NULL;
        }

        if (other.m_pEncryptionKey != IMS_NULL)
        {
            m_pEncryptionKey = new SdpEncryptionKey(*(other.m_pEncryptionKey));
        }

        m_objBandwidths.Clear();
        m_objBandwidths = other.m_objBandwidths;

        m_objAttributes.Clear();
        m_objAttributes = other.m_objAttributes;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpDescription::Decode(IN const AStringArray& objLines,
        IN IMS_SINT32 nStartLine /*= 0*/, IN IMS_SINT32 nEndLine /*= -1*/)
{
    if (nStartLine < 0)
    {
        return IMS_FALSE;
    }

    if (nEndLine == -1)
    {
        nEndLine = objLines.GetCount();
    }

    AString strLineBody;

    for (IMS_SINT32 i = nStartLine; i < nEndLine; ++i)
    {
        const AString& strLine = objLines.GetElementAt(i);

        if ((strLine[0] != Sdp::LINE_I) && (strLine[0] != Sdp::LINE_B) &&
                (strLine[0] != Sdp::LINE_K) && (strLine[0] != Sdp::LINE_A))
        {
            continue;
        }

        strLineBody = strLine.GetSubStr(2);

        switch (strLine[0])
        {
            case Sdp::LINE_I:
                if (!m_abLineContains[Sdp::TYPE_I])
                {
                    m_pInformation = new SdpInformation();

                    if (!m_pInformation->Decode(strLineBody))
                    {
                        delete m_pInformation;
                        m_pInformation = IMS_NULL;

                        IMS_TRACE_E(
                                0, "SDP decoding failed: i-line (%s)", strLineBody.GetStr(), 0, 0);
                        return IMS_FALSE;
                    }

                    m_abLineContains[Sdp::TYPE_I] = IMS_TRUE;
                }
                else
                {
                    // Invalid SDP
                    return IMS_FALSE;
                }
                break;

            case Sdp::LINE_B:
            {
                SdpBandwidth objBandwidth;

                if (!objBandwidth.Decode(strLineBody))
                {
                    IMS_TRACE_E(0, "SDP decoding failed: b-line (%s)", strLineBody.GetStr(), 0, 0);
                    return IMS_FALSE;
                }

                if (!m_objBandwidths.Append(objBandwidth))
                {
                    return IMS_FALSE;
                }

                m_abLineContains[Sdp::TYPE_B] = IMS_TRUE;
            }
            break;

            case Sdp::LINE_K:
                if (!m_abLineContains[Sdp::TYPE_K])
                {
                    m_pEncryptionKey = new SdpEncryptionKey();

                    if (!m_pEncryptionKey->Decode(strLineBody))
                    {
                        delete m_pEncryptionKey;
                        m_pEncryptionKey = IMS_NULL;

                        IMS_TRACE_E(
                                0, "SDP decoding failed: k-line (%s)", strLineBody.GetStr(), 0, 0);
                        return IMS_FALSE;
                    }

                    m_abLineContains[Sdp::TYPE_K] = IMS_TRUE;
                }
                else
                {
                    // Invalid SDP
                    return IMS_FALSE;
                }
                break;

            case Sdp::LINE_A:
            {
                SdpAttribute objAttribute;

                if (!objAttribute.Decode(strLineBody))
                {
                    IMS_TRACE_E(0, "SDP decoding failed: a-line (%s)", strLineBody.GetStr(), 0, 0);

                    // If the attribute is not parsed, then ignore it and continue the parsing.
                    if (strLineBody.StartsWith("rtpmap") || strLineBody.StartsWith("fmtp"))
                    {
                        return IMS_FALSE;
                    }
                    break;
                }

                if (!m_objAttributes.Append(objAttribute))
                {
                    return IMS_FALSE;
                }

                m_abLineContains[Sdp::TYPE_A] = IMS_TRUE;
            }
            break;
        }
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpDescription::Encode() const
{
    return AString::ConstNull();
}

PUBLIC
IMS_BOOL SdpDescription::AddAttribute(IN const SdpAttribute& objAttribute)
{
    if (!m_objAttributes.Append(objAttribute))
    {
        return IMS_FALSE;
    }

    if (!m_abLineContains[Sdp::TYPE_A])
    {
        m_abLineContains[Sdp::TYPE_A] = IMS_TRUE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SdpDescription::AddBandwidth(IN const SdpBandwidth& objBandwidth)
{
    if (!m_objBandwidths.Append(objBandwidth))
    {
        return IMS_FALSE;
    }

    if (!m_abLineContains[Sdp::TYPE_B])
    {
        m_abLineContains[Sdp::TYPE_B] = IMS_TRUE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SdpDescription::Contains(IN IMS_SINT32 nLine) const
{
    if ((nLine < Sdp::TYPE_V) || (nLine >= Sdp::TYPE_MAX))
    {
        return IMS_FALSE;
    }

    return m_abLineContains[nLine];
}

PUBLIC
IMS_BOOL SdpDescription::Contains(IN const SdpAttribute& objAttribute)
{
    for (IMS_UINT32 i = 0; i < m_objAttributes.GetSize(); ++i)
    {
        const SdpAttribute& objExAttribute = m_objAttributes.GetAt(i);

        if (objExAttribute.Equals(&objAttribute))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL SdpDescription::Contains(IN const SdpBandwidth& objBandwidth)
{
    for (IMS_UINT32 i = 0; i < m_objBandwidths.GetSize(); ++i)
    {
        const SdpBandwidth& objExBandwidth = m_objBandwidths.GetAt(i);

        if (objExBandwidth.Equals(&objBandwidth))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
const SdpAttribute* SdpDescription::GetAttribute(IN IMS_SINT32 nAttribute) const
{
    for (IMS_UINT32 i = 0; i < m_objAttributes.GetSize(); ++i)
    {
        const SdpAttribute& objAttribute = m_objAttributes.GetAt(i);

        if (objAttribute.GetAttribute() == nAttribute)
        {
            return &objAttribute;
        }
    }

    return IMS_NULL;
}

PUBLIC
const SdpAttribute* SdpDescription::GetAttribute(IN const AString& strAttribute) const
{
    for (IMS_UINT32 i = 0; i < m_objAttributes.GetSize(); ++i)
    {
        const SdpAttribute& objAttribute = m_objAttributes.GetAt(i);

        if (objAttribute.GetAttributeName().Equals(strAttribute))
        {
            return &objAttribute;
        }
    }

    return IMS_NULL;
}

PUBLIC
ImsList<SdpAttribute> SdpDescription::GetAttributes(IN IMS_SINT32 nAttribute) const
{
    ImsList<SdpAttribute> objCollectedAttributes;

    for (IMS_UINT32 i = 0; i < m_objAttributes.GetSize(); ++i)
    {
        const SdpAttribute& objAttribute = m_objAttributes.GetAt(i);

        if (objAttribute.GetAttribute() == nAttribute)
        {
            objCollectedAttributes.Append(objAttribute);
        }
    }

    return objCollectedAttributes;
}

PUBLIC
ImsList<SdpAttribute> SdpDescription::GetAttributes(IN const AString& strAttribute) const
{
    ImsList<SdpAttribute> objCollectedAttributes;

    for (IMS_UINT32 i = 0; i < m_objAttributes.GetSize(); ++i)
    {
        const SdpAttribute& objAttribute = m_objAttributes.GetAt(i);

        if (objAttribute.GetAttributeName().Equals(strAttribute))
        {
            objCollectedAttributes.Append(objAttribute);
        }
    }

    return objCollectedAttributes;
}

PUBLIC
IMS_SINT32 SdpDescription::GetDirection() const
{
    for (IMS_UINT32 i = 0; i < m_objAttributes.GetSize(); ++i)
    {
        IMS_SINT32 nAttribute = m_objAttributes.GetAt(i).GetAttribute();

        if (nAttribute == SdpAttribute::INACTIVE)
        {
            return Sdp::DIRECTION_INACTIVE;
        }
        else if (nAttribute == SdpAttribute::RECVONLY)
        {
            return Sdp::DIRECTION_RECVONLY;
        }
        else if (nAttribute == SdpAttribute::SENDONLY)
        {
            return Sdp::DIRECTION_SENDONLY;
        }
        else if (nAttribute == SdpAttribute::SENDRECV)
        {
            return Sdp::DIRECTION_SENDRECV;
        }
    }

    // Default value : sendrecv
    return Sdp::DIRECTION_NONE;
}

PUBLIC
void SdpDescription::RemoveAttributes(IN IMS_SINT32 nAttribute)
{
    for (IMS_UINT32 i = 0; i < m_objAttributes.GetSize();)
    {
        const SdpAttribute& objAttribute = m_objAttributes.GetAt(i);

        if (objAttribute.GetAttribute() == nAttribute)
        {
            m_objAttributes.RemoveAt(i);
        }
        else
        {
            ++i;
        }
    }

    if (m_objAttributes.IsEmpty())
    {
        m_abLineContains[Sdp::TYPE_A] = IMS_FALSE;
    }
}

PUBLIC
void SdpDescription::RemoveAttribute(IN const SdpAttribute& objAttribute)
{
    for (IMS_UINT32 i = 0; i < m_objAttributes.GetSize(); ++i)
    {
        const SdpAttribute& objExAttribute = m_objAttributes.GetAt(i);

        if (objExAttribute.Equals(&objAttribute))
        {
            m_objAttributes.RemoveAt(i);
            break;
        }
    }

    if (m_objAttributes.IsEmpty())
    {
        m_abLineContains[Sdp::TYPE_A] = IMS_FALSE;
    }
}

PUBLIC
void SdpDescription::SetAttributes(IN const ImsList<SdpAttribute>& objAttributes)
{
    m_objAttributes.Clear();
    m_objAttributes = objAttributes;

    if (m_objAttributes.IsEmpty())
    {
        m_abLineContains[Sdp::TYPE_A] = IMS_FALSE;
    }
    else
    {
        m_abLineContains[Sdp::TYPE_A] = IMS_TRUE;
    }
}

PUBLIC
void SdpDescription::SetBandwidths(IN const ImsList<SdpBandwidth>& objBandwidths)
{
    m_objBandwidths.Clear();
    m_objBandwidths = objBandwidths;

    if (m_objBandwidths.IsEmpty())
    {
        m_abLineContains[Sdp::TYPE_B] = IMS_FALSE;
    }
    else
    {
        m_abLineContains[Sdp::TYPE_B] = IMS_TRUE;
    }
}

PUBLIC
void SdpDescription::SetEncryptionKey(IN const SdpEncryptionKey& objEncryptionKey)
{
    if (m_pEncryptionKey != IMS_NULL)
    {
        delete m_pEncryptionKey;
    }

    m_pEncryptionKey = new SdpEncryptionKey(objEncryptionKey);

    if (m_pEncryptionKey != IMS_NULL)
    {
        m_abLineContains[Sdp::TYPE_K] = IMS_TRUE;
    }
    else
    {
        m_abLineContains[Sdp::TYPE_K] = IMS_FALSE;
    }
}

PUBLIC
void SdpDescription::SetInformation(IN const SdpInformation& objInformation)
{
    if (m_pInformation != IMS_NULL)
    {
        delete m_pInformation;
    }

    m_pInformation = new SdpInformation(objInformation);

    if (m_pInformation != IMS_NULL)
    {
        m_abLineContains[Sdp::TYPE_I] = IMS_TRUE;
    }
    else
    {
        m_abLineContains[Sdp::TYPE_I] = IMS_FALSE;
    }
}
