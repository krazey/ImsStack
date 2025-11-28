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
#include "IpAddress.h"
#include "ServiceMemory.h"

#include "SdpConnection.h"
#include "SdpEncryptionKey.h"
#include "SdpInformation.h"
#include "SdpSessionDescription.h"
#include "SdpTimezone.h"
#include "SdpUri.h"
#include "offeranswer/SdpOfferAnswer.h"
#include "offeranswer/SdpSessionParameter.h"

PUBLIC
SdpSessionParameter::SdpSessionParameter() :
        SdpParameter(),
        m_pUri(IMS_NULL),
        m_pConnection(IMS_NULL),
        m_pPreviousConnection(IMS_NULL),
        m_pTimezone(IMS_NULL)
{
}

PUBLIC
SdpSessionParameter::SdpSessionParameter(IN const SdpSessionParameter& other) :
        SdpParameter(other),
        m_objVersion(other.m_objVersion),
        m_objOrigin(other.m_objOrigin),
        m_objSessionName(other.m_objSessionName),
        m_pUri(IMS_NULL),
        m_pConnection(IMS_NULL),
        m_pPreviousConnection(IMS_NULL),
        m_objTimeDescriptions(other.m_objTimeDescriptions),
        m_pTimezone(IMS_NULL)
{
    if (other.m_pUri != IMS_NULL)
    {
        m_pUri = new SdpUri(*(other.m_pUri));
    }

    if (other.m_pConnection != IMS_NULL)
    {
        m_pConnection = new SdpConnection(*(other.m_pConnection));
    }

    if (other.m_pPreviousConnection != IMS_NULL)
    {
        m_pPreviousConnection = new SdpConnection(*(other.m_pPreviousConnection));
    }

    if (other.m_pTimezone != IMS_NULL)
    {
        m_pTimezone = new SdpTimezone(*(other.m_pTimezone));
    }
}

PUBLIC
SdpSessionParameter::~SdpSessionParameter()
{
    ClearAllSessionParameters();
}

PUBLIC
SdpSessionParameter& SdpSessionParameter::operator=(IN const SdpSessionParameter& other)
{
    if (this != &other)
    {
        Clear();

        SdpParameter::operator=(other);

        m_objVersion = other.m_objVersion;
        m_objOrigin = other.m_objOrigin;
        m_objSessionName = other.m_objSessionName;

        if (other.m_pUri != IMS_NULL)
        {
            m_pUri = new SdpUri(*(other.m_pUri));
        }

        if (other.m_pConnection != IMS_NULL)
        {
            m_pConnection = new SdpConnection(*(other.m_pConnection));
        }

        if (other.m_pPreviousConnection != IMS_NULL)
        {
            m_pPreviousConnection = new SdpConnection(*(other.m_pPreviousConnection));
        }

        m_objTimeDescriptions = other.m_objTimeDescriptions;

        if (other.m_pTimezone != IMS_NULL)
        {
            m_pTimezone = new SdpTimezone(*(other.m_pTimezone));
        }
    }

    return (*this);
}

PUBLIC VIRTUAL const AString& SdpSessionParameter::GetConnectionAddress() const
{
    if (m_pConnection == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return m_pConnection->GetAddress();
}

PUBLIC VIRTUAL AString SdpSessionParameter::ToSdp() const
{
    // SDP line order: v, o, s, i, u, e, p, c, b, t, r, z, k, a, m
    AStringBuffer objSdp(256);

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

    objSdp.Append(SdpParameter::ToSdp());

    return static_cast<const AStringBuffer&>(objSdp).GetString();
}

PUBLIC
IMS_SINT32 SdpSessionParameter::Compare(
        IN const SdpSessionParameter& objPeerParam, OUT SdpSessionParameter& objProposalParam)
{
    if (!ValidateDirection(&objPeerParam))
    {
        return SdpOfferAnswer::RESULT_FAILURE;
    }

    // Update the session direction
    objProposalParam.UpdateDirection(objPeerParam);

    return SdpOfferAnswer::RESULT_SUCCESS;
}

PUBLIC
IMS_BOOL SdpSessionParameter::Create(IN const SdpSessionDescription& objSessionDescription)
{
    // v-line
    m_objVersion = objSessionDescription.GetVersion();

    // o-line
    m_objOrigin = objSessionDescription.GetOrigin();

    // s-line information
    m_objSessionName = objSessionDescription.GetSessionName();

    // u-line
    if (m_pUri != IMS_NULL)
    {
        delete m_pUri;
        m_pUri = IMS_NULL;
    }

    if (objSessionDescription.Contains(Sdp::TYPE_U))
    {
        const SdpUri* pNewUri = objSessionDescription.GetUri();

        m_pUri = new SdpUri(*pNewUri);
    }

    // e-line, list type

    // p-line, list type

    // Store the previous connection information
    if (m_pPreviousConnection != IMS_NULL)
    {
        delete m_pPreviousConnection;
        m_pPreviousConnection = IMS_NULL;
    }

    if (m_pConnection != IMS_NULL)
    {
        m_pPreviousConnection = new SdpConnection(*m_pConnection);

        delete m_pConnection;
        m_pConnection = IMS_NULL;
    }

    // c-line information
    if (objSessionDescription.Contains(Sdp::TYPE_C))
    {
        const SdpConnection* pNewConnection = objSessionDescription.GetConnection();

        m_pConnection = new SdpConnection(*pNewConnection);
    }

    // t-line & r-line & z-line
    m_objTimeDescriptions.Clear();
    m_objTimeDescriptions = objSessionDescription.GetTimeDescriptions();

    if (m_pTimezone != IMS_NULL)
    {
        delete m_pTimezone;
        m_pTimezone = IMS_NULL;
    }

    if (objSessionDescription.Contains(Sdp::TYPE_Z))
    {
        const SdpTimezone* pNewTimezone = objSessionDescription.GetTimezone();

        m_pTimezone = new SdpTimezone(*pNewTimezone);
    }

    return SdpParameter::Create(objSessionDescription);
}

PUBLIC
IMS_BOOL SdpSessionParameter::IsSameVersion(IN const SdpSessionParameter* pSessionParam) const
{
    if (pSessionParam == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const AString& strMyVersion = m_objOrigin.GetSessionVersion();
    const AString& strOtherVersion = pSessionParam->m_objOrigin.GetSessionVersion();

    return strMyVersion.Equals(strOtherVersion);
}

PUBLIC
IMS_BOOL SdpSessionParameter::SetConnectionAddress(IN const AString& strAddress)
{
    IpAddress objIpAddr;

    if (strAddress.GetLength() > 0)
    {
        m_abLineContains[Sdp::TYPE_C] = IMS_TRUE;
    }
    else
    {
        m_abLineContains[Sdp::TYPE_C] = IMS_FALSE;
    }

    if (!objIpAddr.Parse(strAddress))
    {
        return IMS_FALSE;
    }

    if (objIpAddr.IsIPv6Address())
    {
        return m_pConnection->SetValue(Sdp::ADDR_TYPE_IP6, objIpAddr.ToString());
    }
    else
    {
        return m_pConnection->SetValue(Sdp::ADDR_TYPE_IP4, objIpAddr.ToString());
    }
}

PUBLIC
void SdpSessionParameter::UpdateProperties(IN const SdpSessionParameter& objSessionParam)
{
    // SDP version info.
    m_objVersion = objSessionParam.m_objVersion;
    m_abLineContains[Sdp::TYPE_V] = IMS_TRUE;

    // Update the origin field
    m_objOrigin = objSessionParam.m_objOrigin;
    m_abLineContains[Sdp::TYPE_O] = IMS_TRUE;

    // Update the session name field
    m_objSessionName = objSessionParam.m_objSessionName;
    m_abLineContains[Sdp::TYPE_S] = IMS_TRUE;

    // Update the uri field
    if (objSessionParam.Contains(Sdp::TYPE_U))
    {
        if (m_pUri != IMS_NULL)
        {
            delete m_pUri;
        }

        m_pUri = new SdpUri(*(objSessionParam.m_pUri));

        m_abLineContains[Sdp::TYPE_U] = IMS_TRUE;
    }

    // Update the session level connection if present
    if (objSessionParam.Contains(Sdp::TYPE_C))
    {
        if (m_pConnection != IMS_NULL)
        {
            delete m_pConnection;
        }

        m_pConnection = new SdpConnection(*(objSessionParam.m_pConnection));

        m_abLineContains[Sdp::TYPE_C] = IMS_TRUE;
    }

    // Update the session level previous connection if present
    if (objSessionParam.m_pPreviousConnection)
    {
        if (m_pPreviousConnection != IMS_NULL)
        {
            delete m_pPreviousConnection;
        }

        m_pPreviousConnection = new SdpConnection(*(objSessionParam.m_pPreviousConnection));
    }

    // Update the time information
    m_objTimeDescriptions = objSessionParam.m_objTimeDescriptions;
    m_abLineContains[Sdp::TYPE_T] = IMS_TRUE;

    if (objSessionParam.Contains(Sdp::TYPE_Z))
    {
        if (m_pTimezone != IMS_NULL)
        {
            delete m_pTimezone;
        }

        m_pTimezone = new SdpTimezone(*(objSessionParam.m_pTimezone));

        m_abLineContains[Sdp::TYPE_Z] = IMS_TRUE;
    }

    SdpParameter::UpdateProperties(objSessionParam);
}

PRIVATE VIRTUAL void SdpSessionParameter::Clear()
{
    SdpParameter::Clear();
    ClearAllSessionParameters();
}

PRIVATE
void SdpSessionParameter::ClearAllSessionParameters()
{
    if (m_pUri != IMS_NULL)
    {
        delete m_pUri;
        m_pUri = IMS_NULL;
    }

    if (m_pConnection != IMS_NULL)
    {
        delete m_pConnection;
        m_pConnection = IMS_NULL;
    }

    if (m_pPreviousConnection != IMS_NULL)
    {
        delete m_pPreviousConnection;
        m_pPreviousConnection = IMS_NULL;
    }

    m_objTimeDescriptions.Clear();

    if (m_pTimezone != IMS_NULL)
    {
        delete m_pTimezone;
        m_pTimezone = IMS_NULL;
    }
}
