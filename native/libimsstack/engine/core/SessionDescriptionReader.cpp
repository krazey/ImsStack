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

#include "SdpConnection.h"
#include "SdpInformation.h"
#include "SessionDescriptionReader.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC VIRTUAL IMS_RESULT SessionDescriptionReader::AddAttribute(IN const AString& /*strAttribute*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL ImsList<AString> SessionDescriptionReader::GetAttributes() const
{
    const ImsList<SdpAttribute>& objAttributes = m_objSsd.GetAttributes();
    ImsList<AString> objAttrs;

    for (IMS_UINT32 i = 0; i < objAttributes.GetSize(); ++i)
    {
        objAttrs.Append(objAttributes.GetAt(i).GetValue());
    }

    return objAttrs;
}

PUBLIC VIRTUAL AString SessionDescriptionReader::GetProtocolVersion() const
{
    return m_objSsd.GetVersion().GetValue();
}

PUBLIC VIRTUAL const AString& SessionDescriptionReader::GetSessionId() const
{
    return m_objSsd.GetOrigin().GetSessionId();
}

PUBLIC VIRTUAL AString SessionDescriptionReader::GetSessionInfo() const
{
    const SdpInformation* pInformation = m_objSsd.GetInformation();
    return (pInformation != IMS_NULL) ? pInformation->GetValue() : AString::ConstNull();
}

PUBLIC VIRTUAL AString SessionDescriptionReader::GetSessionName() const
{
    return m_objSsd.GetSessionName().GetValue();
}

PUBLIC VIRTUAL IMS_RESULT SessionDescriptionReader::RemoveAttribute(
        IN const AString& /*strAttribute*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT SessionDescriptionReader::SetSessionInfo(IN const AString& /*strInfo*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT SessionDescriptionReader::SetSessionName(IN const AString& /*strName*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT SessionDescriptionReader::AddAttribute(IN IMS_SINT32 /*nType*/,
        IN const AString& /*strAttrValue*/, IN const AString& /*strType = AString::ConstNull()*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT SessionDescriptionReader::AddAttributeInt(IN IMS_SINT32 /*nType*/,
        IN IMS_SINT32 /*nAttrValue*/, IN const AString& /*strType = AString::ConstNull()*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT SessionDescriptionReader::AddBandwidth(IN IMS_SINT32 /*nType*/,
        IN IMS_SINT32 /*nBandwidth*/, IN const AString& /*strType = AString::ConstNull()*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL const AString& SessionDescriptionReader::GetAttribute(
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
        pAttribute = m_objSsd.GetAttribute(nType);
    }
    else
    {
        pAttribute = m_objSsd.GetAttribute(strType);
    }

    return (pAttribute != IMS_NULL) ? pAttribute->GetAttributeValue() : AString::ConstNull();
}

PUBLIC VIRTUAL IMS_SINT32 SessionDescriptionReader::GetAttributeInt(
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
        const SdpAttribute* pSetup = m_objSsd.GetAttribute(nType);
        const AString& strSetup =
                (pSetup != IMS_NULL) ? pSetup->GetAttributeValue() : AString::ConstNull();
        Sdp::ParseAttributeSetup(strSetup, nAttrSetup);
        return nAttrSetup;
    }
    else if (nType == SdpAttribute::CONNECTION)
    {
        IMS_SINT32 nAttrConnection = Sdp::CONNECTION_NONE;
        const SdpAttribute* pConnection = m_objSsd.GetAttribute(nType);
        const AString& strConnection =
                (pConnection != IMS_NULL) ? pConnection->GetAttributeValue() : AString::ConstNull();
        Sdp::ParseAttributeConnection(strConnection, nAttrConnection);
        return nAttrConnection;
    }

    const SdpAttribute* pAttribute;

    if (nType != SdpAttribute::ATTRIBUTE_OTHER)
    {
        pAttribute = m_objSsd.GetAttribute(nType);
    }
    else
    {
        pAttribute = m_objSsd.GetAttribute(strType);
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
        IMS_TRACE_E(0, "Converting the attribute(%d[%s]:%s) to integer format failed", nType,
                strType.GetStr(), strAttrValue.GetStr());
        return INVALID_VALUE;
    }

    return nAttrValue;
}

PUBLIC VIRTUAL IMS_SINT32 SessionDescriptionReader::GetBandwidth(
        IN IMS_SINT32 nType, IN const AString& strType /*= AString::ConstNull()*/) const
{
    const ImsList<SdpBandwidth>& objBandwidths = m_objSsd.GetBandwidths();
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

PUBLIC VIRTUAL IMS_SINT32 SessionDescriptionReader::GetDirection() const
{
    return m_objSsd.GetDirection();
}

PUBLIC VIRTUAL const AString& SessionDescriptionReader::GetSessionVersion() const
{
    return m_objSsd.GetOrigin().GetSessionVersion();
}

PUBLIC VIRTUAL const AString& SessionDescriptionReader::GetUsername() const
{
    return m_objSsd.GetOrigin().GetUsername();
}

PUBLIC VIRTUAL IMS_RESULT SessionDescriptionReader::RemoveAttribute(
        IN const SdpAttribute& /*objAttribute*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT SessionDescriptionReader::RemoveAttribute(IN IMS_SINT32 /*nType*/,
        IN const AString& /*strAttrValue = AString::ConstNull()*/,
        IN const AString& /*strType = AString::ConstNull()*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT SessionDescriptionReader::RemoveAllBandwidths()
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT SessionDescriptionReader::SetConnectionAddress(
        IN const AString& /*strAddress*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT SessionDescriptionReader::SetDirection(IN IMS_SINT32 /*nDirection*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IMS_RESULT SessionDescriptionReader::SetOriginAddress(
        IN const AString& /*strAddress*/)
{
    IMS_TRACE_D("Not implemented for this descriptor.", 0, 0, 0);
    return IMS_FAILURE;
}

PUBLIC VIRTUAL IpAddress SessionDescriptionReader::GetLocalAddress() const
{
    return IpAddress::NONE;
}

PUBLIC VIRTUAL IpAddress SessionDescriptionReader::GetRemoteAddress() const
{
    IpAddress objAddress;

    if (!objAddress.Parse(GetRemoteAddressAsString()))
    {
        IMS_TRACE_D("Remote connection address may be FQDN or null", 0, 0, 0);
        return IpAddress::NONE;
    }

    return objAddress;
}

PUBLIC VIRTUAL const AString& SessionDescriptionReader::GetRemoteAddressAsString() const
{
    const SdpConnection* pConnection = m_objSsd.GetConnection();
    return (pConnection != IMS_NULL) ? pConnection->GetAddress() : AString::ConstNull();
}
