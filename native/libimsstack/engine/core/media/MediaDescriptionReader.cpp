/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include "ServiceTrace.h"

#include "SdpInformation.h"
#include "offeranswer/SdpMediaFormat.h"
#include "offeranswer/SdpMediaParameter.h"
#include "offeranswer/SdpPrecondition.h"

#include "media/MediaDescriptionReader.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
MediaDescriptionReader::MediaDescriptionReader(
        IN const SdpMediaDescription& objSmd, IN const AString& strSessionLevelRemoteAddress) :
        m_objSmd(objSmd),
        m_objMediaFormats(ImsList<SdpMediaFormat*>()),
        m_pCurrentStatus(IMS_NULL),
        m_pDesiredStatus(IMS_NULL),
        m_pConfirmedStatus(IMS_NULL),
        m_strSessionLevelRemoteAddress(strSessionLevelRemoteAddress)
{
    CreateMediaFormats();
    m_pCurrentStatus = CreatePrecondition(SdpAttribute::CURR);
    m_pDesiredStatus = CreatePrecondition(SdpAttribute::DES);
    m_pConfirmedStatus = CreatePrecondition(SdpAttribute::CONF);
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::AddAttribute(IN const AString& /*strAttribute*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL ImsList<AString> MediaDescriptionReader::GetAttributes() const
{
    const ImsList<SdpAttribute>& objAttributes = m_objSmd.GetAttributes();
    ImsList<AString> objAttrs;

    for (IMS_UINT32 i = 0; i < objAttributes.GetSize(); ++i)
    {
        objAttrs.Append(objAttributes.GetAt(i).GetValue());
    }

    return objAttrs;
}

PUBLIC VIRTUAL ImsList<AString> MediaDescriptionReader::GetBandwidthInfo() const
{
    const ImsList<SdpBandwidth>& objBandwidths = m_objSmd.GetBandwidths();
    ImsList<AString> objBandwidthInfos;

    for (IMS_UINT32 i = 0; i < objBandwidths.GetSize(); ++i)
    {
        objBandwidthInfos.Append(objBandwidths.GetAt(i).GetValue());
    }

    return objBandwidthInfos;
}

PUBLIC VIRTUAL AString MediaDescriptionReader::GetMediaDescription() const
{
    return m_objSmd.GetMedia().GetValue();
}

PUBLIC VIRTUAL AString MediaDescriptionReader::GetMediaTitle() const
{
    const SdpInformation* pInformation = m_objSmd.GetInformation();
    return (pInformation != IMS_NULL) ? pInformation->GetValue() : AString::ConstNull();
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::RemoveAttribute(
        IN const AString& /*strAttribute*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::SetBandwidthInfo(
        IN const ImsList<AString>& /*strBandwidthInfos*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::SetMediaTitle(IN const AString& /*strTitle*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::AddAttribute(IN IMS_SINT32 /*nType*/,
        IN const AString& /*strAttrValue*/, IN const AString& /*strType = AString::ConstNull()*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::AddAttributeInt(IN IMS_SINT32 /*nType*/,
        IN IMS_SINT32 /*nAttrValue*/, IN const AString& /*strType = AString::ConstNull()*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::AddBandwidth(IN IMS_SINT32 /*nType*/,
        IN IMS_SINT32 /*nBandwidth*/, IN const AString& /*strType = AString::ConstNull()*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL const AString& MediaDescriptionReader::GetAttribute(
        IN IMS_SINT32 nType, IN const AString& strType /*= AString::ConstNull()*/) const
{
    // Special attributes for SDP negotiation
    //    recvonly, sendrecv, sendonly, setup, connection
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY) || (nType == SdpAttribute::SETUP) ||
            (nType == SdpAttribute::CONNECTION))
    {
        IMS_TRACE_E(0, "Not allowed attribute (%d)", nType, 0, 0);
        return AString::ConstNull();
    }

    const SdpAttribute* pAttribute;

    if (nType != SdpAttribute::ATTRIBUTE_OTHER)
    {
        pAttribute = m_objSmd.GetAttribute(nType);
    }
    else
    {
        pAttribute = m_objSmd.GetAttribute(strType);
    }

    return (pAttribute != IMS_NULL) ? pAttribute->GetAttributeValue() : AString::ConstNull();
}

PUBLIC VIRTUAL ImsList<AString> MediaDescriptionReader::GetAttributes(
        IN IMS_SINT32 nType, IN const AString& strType /*= AString::ConstNull()*/) const
{
    // Special attributes for SDP negotiation
    //    recvonly, sendrecv, sendonly, setup, connection
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY) || (nType == SdpAttribute::SETUP) ||
            (nType == SdpAttribute::CONNECTION))
    {
        IMS_TRACE_E(0, "Not allowed attribute (%d)", nType, 0, 0);
        return ImsList<AString>();
    }

    ImsList<AString> objAttributeValues;
    const ImsList<SdpAttribute>& objAttributes = m_objSmd.GetAttributes();

    if (nType != SdpAttribute::ATTRIBUTE_OTHER)
    {
        for (IMS_UINT32 i = 0; i < objAttributes.GetSize(); ++i)
        {
            const SdpAttribute& objAttribute = objAttributes.GetAt(i);

            if (objAttribute.GetAttribute() == nType)
            {
                objAttributeValues.Append(objAttribute.GetAttributeValue());
            }
        }
    }
    else
    {
        for (IMS_UINT32 i = 0; i < objAttributes.GetSize(); ++i)
        {
            const SdpAttribute& objAttribute = objAttributes.GetAt(i);

            if (objAttribute.GetAttributeName().Equals(strType))
            {
                objAttributeValues.Append(objAttribute.GetAttributeValue());
            }
        }
    }

    return objAttributeValues;
}

PUBLIC VIRTUAL IMS_SINT32 MediaDescriptionReader::GetAttributeInt(
        IN IMS_SINT32 nType, IN const AString& strType /*= AString::ConstNull()*/) const
{
    // Special attributes for SDP negotiation
    //    recvonly, sendrecv, sendonly, setup, connection
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY))
    {
        IMS_TRACE_E(0, "Not allowed attribute (%d)", nType, 0, 0);
        return INVALID_VALUE;
    }
    else if (nType == SdpAttribute::SETUP)
    {
        IMS_SINT32 nAttrSetup = Sdp::SETUP_NONE;
        const SdpAttribute* pSetup = m_objSmd.GetAttribute(nType);
        const AString& strSetup =
                (pSetup != IMS_NULL) ? pSetup->GetAttributeValue() : AString::ConstNull();
        Sdp::ParseAttributeSetup(strSetup, nAttrSetup);
        return nAttrSetup;
    }
    else if (nType == SdpAttribute::CONNECTION)
    {
        IMS_SINT32 nAttrConnection = Sdp::CONNECTION_NONE;
        const SdpAttribute* pConnection = m_objSmd.GetAttribute(nType);
        const AString& strConnection =
                (pConnection != IMS_NULL) ? pConnection->GetAttributeValue() : AString::ConstNull();
        Sdp::ParseAttributeConnection(strConnection, nAttrConnection);
        return nAttrConnection;
    }

    const SdpAttribute* pAttribute;

    if (nType != SdpAttribute::ATTRIBUTE_OTHER)
    {
        pAttribute = m_objSmd.GetAttribute(nType);
    }
    else
    {
        pAttribute = m_objSmd.GetAttribute(strType);
    }

    if (pAttribute == IMS_NULL)
    {
        return INVALID_VALUE;
    }

    const AString& strAttrValue = pAttribute->GetAttributeValue();
    IMS_BOOL bOk = IMS_FALSE;
    IMS_SINT32 nAttrValue = strAttrValue.ToInt32(&bOk);

    if (!bOk)
    {
        // Do the specific operation for framerate attribute
        if (nType == SdpAttribute::FRAMERATE)
        {
            // Check if it contains the fraction part or not
            IMS_SINT32 nIndex = strAttrValue.GetIndexOf('.');

            if (nIndex > 0)
            {
                AString strIntegerPart = strAttrValue.GetSubStr(0, nIndex);

                bOk = IMS_FALSE;
                nAttrValue = strIntegerPart.ToInt32(&bOk);

                if (bOk)
                {
                    return nAttrValue;
                }
            }
        }

        IMS_TRACE_E(0, "Converting the attribute(%d[%s]:%s) to integer format failed", nType,
                strType.GetStr(), strAttrValue.GetStr());
        return INVALID_VALUE;
    }

    return nAttrValue;
}

PUBLIC VIRTUAL IMS_SINT32 MediaDescriptionReader::GetBandwidth(
        IN IMS_SINT32 nType, IN const AString& strType /*= AString::ConstNull()*/) const
{
    const ImsList<SdpBandwidth>& objBandwidths = m_objSmd.GetBandwidths();
    const SdpBandwidth* pBandwidth = IMS_NULL;

    if (nType != SdpBandwidth::TYPE_OTHER)
    {
        for (IMS_UINT32 i = 0; i < objBandwidths.GetSize(); ++i)
        {
            const SdpBandwidth& objBandwidth = objBandwidths.GetAt(i);

            if (objBandwidth.GetType() == nType)
            {
                pBandwidth = &objBandwidth;
                break;
            }
        }
    }
    else
    {
        for (IMS_UINT32 i = 0; i < objBandwidths.GetSize(); ++i)
        {
            const SdpBandwidth& objBandwidth = objBandwidths.GetAt(i);

            if (objBandwidth.GetType() != SdpBandwidth::TYPE_OTHER)
            {
                continue;
            }

            if (strType.EqualsIgnoreCase(objBandwidth.GetTypeName()))
            {
                pBandwidth = &objBandwidth;
                break;
            }
        }
    }

    return (pBandwidth != IMS_NULL) ? pBandwidth->GetBandwidth() : INVALID_VALUE;
}

PUBLIC VIRTUAL IMS_SINT32 MediaDescriptionReader::GetDirection() const
{
    return m_objSmd.GetDirection();
}

PUBLIC VIRTUAL const SdpMedia* MediaDescriptionReader::GetMediaDescriptionEx() const
{
    return &(m_objSmd.GetMedia());
}

PUBLIC VIRTUAL const ImsList<SdpMediaFormat*>& MediaDescriptionReader::GetMediaFormats() const
{
    return m_objMediaFormats;
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::RemoveAttribute(
        IN const SdpAttribute& /*objAttribute*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::RemoveAttribute(IN IMS_SINT32 /*nType*/,
        IN const AString& /*strAttrValue = AString::ConstNull()*/,
        IN const AString& /*strType = AString::ConstNull()*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::RemoveMediaFormat(
        IN IMS_SINT32 /*nType*/, IN const AString& /*strValue*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::SetConnectionAddress(
        IN const AString& /*strAddress*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::SetDirection(IN IMS_SINT32 /*nDirection*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::SetMediaDescription(IN IMS_SINT32 /*nType*/,
        IN IMS_SINT32 /*nPort*/, IN IMS_SINT32 /*nTransportProtocol*/,
        IN const AStringArray& /*objFormats*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::SetMediaFormat(
        IN const SdpMediaFormat* /*pMediaFormat*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::SetMediaFormat(IN IMS_SINT32 /*nType*/,
        IN const AString& /*strValue*/, IN const AString& /*strAnyMap*/,
        IN const AString& /*strFmtp*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::SetPort(IN IMS_SINT32 /*nPort*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL const SdpMedia* MediaDescriptionReader::GetMediaDescriptionExAsLocal() const
{
    return IMS_NULL;
}

PUBLIC VIRTUAL IpAddress MediaDescriptionReader::GetLocalAddress() const
{
    return IpAddress::NONE;
}

PUBLIC VIRTUAL IMS_SINT32 MediaDescriptionReader::GetLocalPort() const
{
    return 0;
}

PUBLIC VIRTUAL IpAddress MediaDescriptionReader::GetRemoteAddress() const
{
    IpAddress objAddress;

    if (!objAddress.Parse(GetRemoteAddressAsString()))
    {
        IMS_TRACE_D("Remote connection address may be FQDN or null", 0, 0, 0);
        return IpAddress::NONE;
    }

    return objAddress;
}

PUBLIC VIRTUAL const AString& MediaDescriptionReader::GetRemoteAddressAsString() const
{
    const SdpConnection* pConnection = m_objSmd.GetConnection();
    return (pConnection != IMS_NULL) ? pConnection->GetAddress() : m_strSessionLevelRemoteAddress;
}

PUBLIC VIRTUAL IMS_SINT32 MediaDescriptionReader::GetRemotePort() const
{
    return m_objSmd.GetMedia().GetPort();
}

// IMS_SDP_PRECONDITION
PUBLIC VIRTUAL const SdpPrecondition* MediaDescriptionReader::GetPrecondition(
        IN IMS_SINT32 nAttribute, IN IMS_SINT32 nType /*= SdpPrecondition::TYPE_QOS*/) const
{
    if (nType != SdpPrecondition::TYPE_QOS)
    {
        IMS_TRACE_E(
                0, "Type (%d) is not supported; It only supports 'qos' precondition", nType, 0, 0);
        return IMS_NULL;
    }

    switch (nAttribute)
    {
        case SdpAttribute::CURR:
            return m_pCurrentStatus;
        case SdpAttribute::DES:
            return m_pDesiredStatus;
        case SdpAttribute::CONF:
            return m_pConfirmedStatus;
        default:
            return IMS_NULL;
    }
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::RemovePrecondition(
        IN IMS_SINT32 /*nAttribute*/, IN IMS_SINT32 /*nType = SdpPrecondition::TYPE_QOS*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT MediaDescriptionReader::SetPrecondition(
        IN IMS_SINT32 /*nAttribute*/, IN const SdpPrecondition* /*pPrecondition*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PRIVATE
void MediaDescriptionReader::CreateMediaFormats()
{
    SdpMediaParameter::ExtractMediaFormat(m_objSmd, m_objMediaFormats);

    // Remove "rtpmap" & "fmtp" attributes
    m_objSmd.RemoveAttributes(SdpAttribute::RTPMAP);
    m_objSmd.RemoveAttributes(SdpAttribute::FMTP);

    // Remove "rtcp-fb" attribute
    m_objSmd.RemoveAttributes(SdpAttribute::RTCP_FB);
}

PRIVATE
SdpPrecondition* MediaDescriptionReader::CreatePrecondition(IN IMS_SINT32 nAttribute)
{
    SdpPrecondition* pPrecondition = IMS_NULL;
    ImsList<SdpAttribute> objAttrs = m_objSmd.GetAttributes(nAttribute);
    ImsList<SdpAttribute> objQoSAttrs;

    if (!objAttrs.IsEmpty())
    {
        pPrecondition = SdpMediaParameter::CreatePrecondition(objAttrs, objQoSAttrs);

        if (pPrecondition != IMS_NULL)
        {
            for (IMS_UINT32 i = 0; i < objQoSAttrs.GetSize(); ++i)
            {
                m_objSmd.RemoveAttribute(objQoSAttrs.GetAt(i));
            }
        }
    }
    return pPrecondition;
}
