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

#include "Sdp.h"
#include "SdpEncryptionKey.h"
#include "SdpInformation.h"
#include "SdpMediaDescription.h"

__IMS_TRACE_TAG_SDP__;

PUBLIC
SdpMediaDescription::SdpMediaDescription() :
        SdpDescription()
{
}

PUBLIC
SdpMediaDescription::SdpMediaDescription(IN const SdpMediaDescription& other) :
        SdpDescription(other),
        m_objMedia(other.m_objMedia),
        m_objConnections(other.m_objConnections)
{
}

PUBLIC VIRTUAL SdpMediaDescription::~SdpMediaDescription()
{
    m_objConnections.Clear();
}

PUBLIC
SdpMediaDescription& SdpMediaDescription::operator=(IN const SdpMediaDescription& other)
{
    if (this != &other)
    {
        SdpDescription::operator=(other);

        m_objMedia = other.m_objMedia;

        m_objConnections.Clear();
        m_objConnections = other.m_objConnections;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpMediaDescription::Decode(IN const AStringArray& objLines,
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

        if ((strLine[0] != Sdp::LINE_M) && (strLine[0] != Sdp::LINE_C))
        {
            continue;
        }

        strLineBody = strLine.GetSubStr(2);

        switch (strLine[0])
        {
            case Sdp::LINE_M:
                if (!m_abLineContains[Sdp::TYPE_M])
                {
                    if (!m_objMedia.Decode(strLineBody))
                    {
                        IMS_TRACE_E(
                                0, "SDP decoding failed: m-line (%s)", strLineBody.GetStr(), 0, 0);
                        return IMS_FALSE;
                    }

                    m_abLineContains[Sdp::TYPE_M] = IMS_TRUE;
                }
                else
                {
                    // Invalid SDP
                    return IMS_FALSE;
                }
                break;

            case Sdp::LINE_C:
            {
                SdpConnection objConnection;

                if (!objConnection.Decode(strLineBody))
                {
                    IMS_TRACE_E(0, "SDP decoding failed: c-line (%s)", strLineBody.GetStr(), 0, 0);
                    return IMS_FALSE;
                }

                if (!m_objConnections.Append(objConnection))
                    return IMS_FALSE;

                m_abLineContains[Sdp::TYPE_C] = IMS_TRUE;
            }
            break;
        }
    }

    return SdpDescription::Decode(objLines, nStartLine, nEndLine);
}

PUBLIC VIRTUAL AString SdpMediaDescription::Encode() const
{
    // SDP order: m, i, c, b, k, *(a)
    if (!m_abLineContains[Sdp::TYPE_M])
    {
        return AString::ConstNull();
    }

    AStringBuffer objSdp(512);

    objSdp.Append(m_objMedia.Encode());

    if (m_abLineContains[Sdp::TYPE_I])
    {
        objSdp.Append(GetInformation()->Encode());
    }

    if (m_abLineContains[Sdp::TYPE_C])
    {
        for (IMS_UINT32 i = 0; i < m_objConnections.GetSize(); ++i)
        {
            const SdpConnection& objConnection = m_objConnections.GetAt(i);

            objSdp.Append(objConnection.Encode());
        }
    }

    if (m_abLineContains[Sdp::TYPE_B])
    {
        const ImsList<SdpBandwidth>& objBLines = GetBandwidths();

        for (IMS_UINT32 i = 0; i < objBLines.GetSize(); ++i)
        {
            const SdpBandwidth& objBandwidth = objBLines.GetAt(i);

            objSdp.Append(objBandwidth.Encode());
        }
    }

    if (m_abLineContains[Sdp::TYPE_K])
    {
        objSdp.Append(GetEncryptionKey()->Encode());
    }

    if (m_abLineContains[Sdp::TYPE_A])
    {
        const ImsList<SdpAttribute>& objALines = GetAttributes();

        for (IMS_UINT32 i = 0; i < objALines.GetSize(); ++i)
        {
            const SdpAttribute& objAttribute = objALines.GetAt(i);

            objSdp.Append(objAttribute.Encode());
        }
    }

    return static_cast<const AStringBuffer&>(objSdp).GetString();
}

PUBLIC
const SdpConnection* SdpMediaDescription::GetConnection() const
{
    if (m_objConnections.GetSize() == 0)
    {
        return IMS_NULL;
    }

    const SdpConnection& objConnection = m_objConnections.GetAt(0);

    return &objConnection;
}

PUBLIC
void SdpMediaDescription::RemoveConnections()
{
    m_objConnections.Clear();

    m_abLineContains[Sdp::TYPE_C] = IMS_FALSE;
}

PUBLIC
void SdpMediaDescription::SetConnections(IN const ImsList<SdpConnection>& objConnections)
{
    m_objConnections.Clear();
    m_objConnections = objConnections;

    if (m_objConnections.IsEmpty())
    {
        m_abLineContains[Sdp::TYPE_C] = IMS_FALSE;
    }
    else
    {
        m_abLineContains[Sdp::TYPE_C] = IMS_TRUE;
    }
}
