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
#include "AStringBuffer.h"
#include "IpAddress.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "Sdp.h"
#include "SdpConnection.h"
#include "SdpEncryptionKey.h"
#include "SdpInformation.h"
#include "SdpSessionDescription.h"
#include "SdpTime.h"
#include "SdpTimezone.h"
#include "SdpUri.h"

__IMS_TRACE_TAG_SDP__;

PUBLIC
SdpSessionDescription::SdpSessionDescription() :
        SdpDescription(),
        m_pUri(IMS_NULL),
        m_pConnection(IMS_NULL),
        m_pTimezone(IMS_NULL)
{
}

PUBLIC
SdpSessionDescription::SdpSessionDescription(IN const SdpSessionDescription& other) :
        SdpDescription(other),
        m_objVersion(other.m_objVersion),
        m_objOrigin(other.m_objOrigin),
        m_objSessionName(other.m_objSessionName),
        m_pUri(IMS_NULL),
        m_pConnection(IMS_NULL),
        m_pTimezone(IMS_NULL)
{
    if (other.m_pConnection != IMS_NULL)
    {
        m_pConnection = new SdpConnection(*(other.m_pConnection));
    }

    if (other.m_pUri != IMS_NULL)
    {
        m_pUri = new SdpUri(*(other.m_pUri));
    }

    if (other.m_pTimezone != IMS_NULL)
    {
        m_pTimezone = new SdpTimezone(*(other.m_pTimezone));
    }

    m_objTimeDescriptions = other.m_objTimeDescriptions;
}

PUBLIC VIRTUAL SdpSessionDescription::~SdpSessionDescription()
{
    if (m_pConnection != IMS_NULL)
    {
        delete m_pConnection;
    }

    if (m_pUri != IMS_NULL)
    {
        delete m_pUri;
    }

    if (m_pTimezone != IMS_NULL)
    {
        delete m_pTimezone;
    }

    m_objTimeDescriptions.Clear();
}

PUBLIC
SdpSessionDescription& SdpSessionDescription::operator=(IN const SdpSessionDescription& other)
{
    if (this != &other)
    {
        SdpDescription::operator=(other);

        m_objVersion = other.m_objVersion;
        m_objOrigin = other.m_objOrigin;
        m_objSessionName = other.m_objSessionName;

        if (m_pConnection != IMS_NULL)
        {
            delete m_pConnection;
            m_pConnection = IMS_NULL;
        }

        if (other.m_pConnection != IMS_NULL)
        {
            m_pConnection = new SdpConnection(*(other.m_pConnection));
        }

        if (m_pUri != IMS_NULL)
        {
            delete m_pUri;
            m_pUri = IMS_NULL;
        }

        if (other.m_pUri != IMS_NULL)
        {
            m_pUri = new SdpUri(*(other.m_pUri));
        }

        if (m_pTimezone != IMS_NULL)
        {
            delete m_pTimezone;
            m_pTimezone = IMS_NULL;
        }

        if (other.m_pTimezone != IMS_NULL)
        {
            m_pTimezone = new SdpTimezone(*(other.m_pTimezone));
        }

        m_objTimeDescriptions.Clear();
        m_objTimeDescriptions = other.m_objTimeDescriptions;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpSessionDescription::Decode(IN const AStringArray& objLines,
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

        strLineBody = strLine.GetSubStr(2);

        switch (strLine[0])
        {
            case Sdp::LINE_V:
                if (!m_abLineContains[Sdp::TYPE_V])
                {
                    if (!m_objVersion.Decode(strLineBody))
                    {
                        IMS_TRACE_E(
                                0, "SDP decoding failed: v-line (%s)", strLineBody.GetStr(), 0, 0);
                        return IMS_FALSE;
                    }

                    m_abLineContains[Sdp::TYPE_V] = IMS_TRUE;
                }
                else
                {
                    // Invalid SDP
                    return IMS_FALSE;
                }
                break;

            case Sdp::LINE_O:
                if (!m_abLineContains[Sdp::TYPE_O])
                {
                    if (!m_objOrigin.Decode(strLineBody))
                    {
                        IMS_TRACE_E(
                                0, "SDP decoding failed: o-line (%s)", strLineBody.GetStr(), 0, 0);
                        return IMS_FALSE;
                    }

                    m_abLineContains[Sdp::TYPE_O] = IMS_TRUE;
                }
                else
                {
                    // Invalid SDP
                    return IMS_FALSE;
                }
                break;

            case Sdp::LINE_S:
                if (!m_abLineContains[Sdp::TYPE_S])
                {
                    if (!m_objSessionName.Decode(strLineBody))
                    {
                        IMS_TRACE_E(
                                0, "SDP decoding failed: s-line (%s)", strLineBody.GetStr(), 0, 0);
                        return IMS_FALSE;
                    }

                    m_abLineContains[Sdp::TYPE_S] = IMS_TRUE;
                }
                else
                {
                    // Invalid SDP
                    return IMS_FALSE;
                }
                break;

            case Sdp::LINE_C:
                if (!m_abLineContains[Sdp::TYPE_C])
                {
                    m_pConnection = new SdpConnection();

                    if (!m_pConnection->Decode(strLineBody))
                    {
                        delete m_pConnection;
                        m_pConnection = IMS_NULL;

                        IMS_TRACE_E(
                                0, "SDP decoding failed: c-line (%s)", strLineBody.GetStr(), 0, 0);
                        return IMS_FALSE;
                    }

                    m_abLineContains[Sdp::TYPE_C] = IMS_TRUE;
                }
                else
                {
                    // Invalid SDP
                    return IMS_FALSE;
                }
                break;

            case Sdp::LINE_U:
                if (!m_abLineContains[Sdp::TYPE_U])
                {
                    m_pUri = new SdpUri();

                    if (!m_pUri->Decode(strLineBody))
                    {
                        delete m_pUri;
                        m_pUri = IMS_NULL;

                        IMS_TRACE_E(
                                0, "SDP decoding failed: u-line (%s)", strLineBody.GetStr(), 0, 0);
                        return IMS_FALSE;
                    }

                    m_abLineContains[Sdp::TYPE_U] = IMS_TRUE;
                }
                else
                {
                    // Invalid SDP
                    return IMS_FALSE;
                }
                break;

            case Sdp::LINE_T:
            {
                SdpTime* pTime = new SdpTime();

                if (!pTime->Decode(strLineBody))
                {
                    delete pTime;
                    IMS_TRACE_E(0, "SDP decoding failed: t-line (%s)", strLineBody.GetStr(), 0, 0);
                    return IMS_FALSE;
                }

                // Find 'r' lines
                IMS_SINT32 nTimeEndLine = i + 1;

                for (IMS_SINT32 j = i + 1; j < nEndLine; ++j)
                {
                    const AString& strRLine = objLines.GetElementAt(j);

                    if (strRLine[0] != Sdp::LINE_R)
                    {
                        break;
                    }

                    ++nTimeEndLine;
                }

                SdpTimeDescription objTimeDescription(pTime);

                if (nTimeEndLine > (i + 1))
                {
                    if (!objTimeDescription.Decode(objLines, i + 1, nTimeEndLine))
                    {
                        return IMS_FALSE;
                    }
                }

                if (!m_objTimeDescriptions.Append(objTimeDescription))
                {
                    return IMS_FALSE;
                }

                m_abLineContains[Sdp::TYPE_T] = IMS_TRUE;

                if (nTimeEndLine > (i + 1))
                {
                    m_abLineContains[Sdp::TYPE_R] = IMS_TRUE;
                }
            }
            break;

            case Sdp::LINE_Z:
                if (!m_abLineContains[Sdp::TYPE_Z])
                {
                    m_pTimezone = new SdpTimezone();

                    if (!m_pTimezone->Decode(strLineBody))
                    {
                        delete m_pTimezone;
                        m_pTimezone = IMS_NULL;

                        IMS_TRACE_E(
                                0, "SDP decoding failed: z-line (%s)", strLineBody.GetStr(), 0, 0);
                        return IMS_FALSE;
                    }

                    m_abLineContains[Sdp::TYPE_Z] = IMS_TRUE;
                }
                else
                {
                    // Invalid SDP
                    return IMS_FALSE;
                }
                break;
        }
    }

    if (!m_abLineContains[Sdp::TYPE_V] || !m_abLineContains[Sdp::TYPE_O] ||
            !m_abLineContains[Sdp::TYPE_S] || !m_abLineContains[Sdp::TYPE_T])
    {
        // mandatory SDP line is missing
        IMS_TRACE_E(0, "Check the validity: mandatory lines (v,o,s,t) are missing", 0, 0, 0);
        return IMS_FALSE;
    }

    return SdpDescription::Decode(objLines, nStartLine, nEndLine);
}

PUBLIC VIRTUAL AString SdpSessionDescription::Encode() const
{
    // SDP line order: v, o, s, i, u, e, p, c, b, t, r, z, k, a, m
    AStringBuffer objSdp(512);

    if (m_abLineContains[Sdp::TYPE_V])
    {
        objSdp.Append(m_objVersion.Encode());
    }

    if (m_abLineContains[Sdp::TYPE_O])
    {
        objSdp.Append(m_objOrigin.Encode());
    }

    if (m_abLineContains[Sdp::TYPE_S])
    {
        objSdp.Append(m_objSessionName.Encode());
    }

    if (m_abLineContains[Sdp::TYPE_I])
    {
        objSdp.Append(GetInformation()->Encode());
    }

    if (m_abLineContains[Sdp::TYPE_U])
    {
        objSdp.Append(m_pUri->Encode());
    }

    // Skip 'e' & 'p'

    if (m_abLineContains[Sdp::TYPE_C])
    {
        objSdp.Append(m_pConnection->Encode());
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

    if (m_abLineContains[Sdp::TYPE_T])
    {
        for (IMS_UINT32 i = 0; i < m_objTimeDescriptions.GetSize(); ++i)
        {
            const SdpTimeDescription& objTimeDescription = m_objTimeDescriptions.GetAt(i);

            objSdp.Append(objTimeDescription.Encode());
        }
    }

    if (m_abLineContains[Sdp::TYPE_Z])
    {
        objSdp.Append(m_pTimezone->Encode());
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
IMS_BOOL SdpSessionDescription::CreateMandatoryLines(
        IN const AString& strUserId, IN const IpAddress& objLocalAddress)
{
    // Mandatory SDP lines: v, o, s, t
    AString strAddress = objLocalAddress.ToString();

    // v-line
    m_abLineContains[Sdp::TYPE_V] = IMS_TRUE;

    // o-line
    if (!m_objOrigin.SetValue(strUserId, strAddress))
    {
        return IMS_FALSE;
    }

    m_abLineContains[Sdp::TYPE_O] = IMS_TRUE;

    // s-line
    m_abLineContains[Sdp::TYPE_S] = IMS_TRUE;

    // t-line
    SdpTimeDescription objTimeDescription(new SdpTime());

    if (!m_objTimeDescriptions.Append(objTimeDescription))
    {
        return IMS_FALSE;
    }

    m_abLineContains[Sdp::TYPE_T] = IMS_TRUE;

    // c-line : It's an optional line, but we will create a connection line.
    m_pConnection = new SdpConnection();

    if (m_pConnection == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nAddrType =
            (objLocalAddress.IsIPv6Address()) ? Sdp::ADDR_TYPE_IP6 : Sdp::ADDR_TYPE_IP4;

    if (!m_pConnection->SetValue(nAddrType, strAddress))
    {
        delete m_pConnection;
        m_pConnection = IMS_NULL;

        return IMS_FALSE;
    }

    m_abLineContains[Sdp::TYPE_C] = IMS_TRUE;

    return IMS_TRUE;
}

PUBLIC
void SdpSessionDescription::SetSessionName(IN const SdpSessionName& objSessionName)
{
    m_objSessionName = objSessionName;

    m_abLineContains[Sdp::TYPE_S] = IMS_TRUE;
}

PUBLIC
void SdpSessionDescription::SetConnection(IN const SdpConnection& objConnection)
{
    if (m_pConnection != IMS_NULL)
    {
        delete m_pConnection;
    }

    m_pConnection = new SdpConnection(objConnection);

    if (m_pConnection != IMS_NULL)
    {
        m_abLineContains[Sdp::TYPE_C] = IMS_TRUE;
    }
    else
    {
        m_abLineContains[Sdp::TYPE_C] = IMS_FALSE;
    }
}
