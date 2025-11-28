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
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "SdpInformation.h"
#include "offeranswer/SdpAvCodec.h"
#include "offeranswer/SdpMediaFormatParameter.h"
#include "offeranswer/SdpMediaParameter.h"

#include "SipDebug.h"
#include "base/Ims.h"
#include "media/IMediaState.h"
#include "media/MediaDescriptor.h"

__IMS_TRACE_TAG_IMS_CORE__;

// clang-format off
PUBLIC GLOBAL
const IMS_CHAR* MediaDescriptor::RESERVED_ATTRIBUTE[MediaDescriptor::MAX_RESERVED_ATTRIBUTE] =
{
    "des",
    "curr",
    "conf",
    "mid",
    "ice-pwd",
    "ice-ufrag",
    "candidate",
    "remote-candidates",
    "sendonly",
    "recvonly",
    "sendrecv",
    "inactive",
    "csup",
    "creq",
    "acap",
    "tcap",
    "pcfg",
    "acfg",
    "3gpp_sync_info",
    // StreamMedia
    "rtpmap",
    "fmtp",
    "dccp-service-code",
    "rtcp-mux",
    "rtp-fb",
    "ptime",
    "maxptime",
    "framesize",
    "framerate",
    "quality",
    // FramedMedia
    "setup",
    "connection",
    "accept-types",
    "accept-wrapped-types",
    "max-size",
    "path",
    // BasicReliableMedia
    "setup",
    "connection",
};
// clang-format on

PUBLIC
void MediaDescriptor::SetMid(IN IMS_SINT32 nMid)
{
    if (m_nMid != nMid)
    {
        IMS_TRACE_I("SetMid :: %d >> %d", m_nMid, nMid, 0);
        m_nMid = nMid;
    }
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::AddAttribute(IN const AString& strAttribute)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    if ((nState != IMediaState::MEDIA_STATE_INACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_INACTIVE_PROPOSAL) &&
            (nState != IMediaState::MEDIA_STATE_ACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_PROPOSAL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to add an attribute (%s) in the state (%d)", strAttribute.GetStr(),
                nState, 0);
        return IMS_FAILURE;
    }

    SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    SdpAttribute objAttribute;

    // Check a syntax of the attribute
    if (!objAttribute.Decode(strAttribute))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Decoding an attribute (%s) failed", strAttribute.GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    // Check if it is a reserved or not
    if (IsAttributeReserved(pMediaParam, objAttribute.GetAttributeName()))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Trying to set a reserved attribute (%s)", strAttribute.GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    // Check if it already exists in the session
    if (pMediaParam->Contains(objAttribute))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Attribute (%s) already exists", strAttribute.GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    if (!pMediaParam->AddAttribute(objAttribute))
    {
        Ims::SetLastError(ImsError::INVALID_OPERATION);

        IMS_TRACE_E(0, "Adding an attribute (%s) failed", strAttribute.GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL ImsList<AString> MediaDescriptor::GetAttributes() const
{
    const SdpMediaParameter* pMediaParam = m_piMediaState->GetPeerMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer media parameter", 0, 0, 0);
        return ImsList<AString>();
    }

    const ImsList<SdpAttribute>& objSdpAttributes = pMediaParam->GetAttributes();
    ImsList<AString> objAttributes;

    for (IMS_UINT32 i = 0; i < objSdpAttributes.GetSize(); ++i)
    {
        const SdpAttribute& objAttribute = objSdpAttributes.GetAt(i);

        objAttributes.Append(objAttribute.GetValue());
    }

    return objAttributes;
}

PRIVATE VIRTUAL ImsList<AString> MediaDescriptor::GetBandwidthInfo() const
{
    const SdpMediaParameter* pMediaParam = m_piMediaState->GetPeerMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer media parameter", 0, 0, 0);
        return ImsList<AString>();
    }

    const ImsList<SdpBandwidth>& objSdpBandwidths = pMediaParam->GetBandwidths();
    ImsList<AString> objBandwidths;

    for (IMS_UINT32 i = 0; i < objSdpBandwidths.GetSize(); ++i)
    {
        const SdpBandwidth& objBandwidth = objSdpBandwidths.GetAt(i);

        objBandwidths.Append(objBandwidth.GetValue());
    }

    return objBandwidths;
}

PRIVATE VIRTUAL AString MediaDescriptor::GetMediaDescription() const
{
    const SdpMediaParameter* pMediaParam = m_piMediaState->GetPeerMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer media parameter", 0, 0, 0);
        return AString::ConstNull();
    }

    // Check a session state
    if (m_piMediaState->GetMediaState() == IMediaState::MEDIA_STATE_INACTIVE)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);
        return AString::ConstNull();
    }

    return pMediaParam->GetMedia().GetValue();
}

PRIVATE VIRTUAL AString MediaDescriptor::GetMediaTitle() const
{
    const SdpMediaParameter* pMediaParam = m_piMediaState->GetPeerMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer media parameter", 0, 0, 0);
        return AString::ConstNull();
    }

    const SdpInformation* pInformation = pMediaParam->GetInformation();

    if (pInformation == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return pInformation->GetValue();
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::RemoveAttribute(IN const AString& strAttribute)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    if ((nState != IMediaState::MEDIA_STATE_INACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_INACTIVE_PROPOSAL) &&
            (nState != IMediaState::MEDIA_STATE_ACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_PROPOSAL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to remove an attribute (%s) in the state (%d)",
                strAttribute.GetStr(), nState, 0);
        return IMS_FAILURE;
    }

    SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    SdpAttribute objAttribute;

    // Check a syntax of the attribute
    if (!objAttribute.Decode(strAttribute))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Decoding an attribute (%s) failed", strAttribute.GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    // Check if it is a reserved or not
    if (IsAttributeReserved(pMediaParam, objAttribute.GetAttributeName()))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(
                0, "Trying to set a reserved attribute (%s) failed", strAttribute.GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    // Check if it already exists in the session
    if (!pMediaParam->Contains(objAttribute))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Attribute (%s) already exists", strAttribute.GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    pMediaParam->RemoveAttribute(objAttribute);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::SetBandwidthInfo(
        IN const ImsList<AString>& strBandwidthInfos)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    if ((nState != IMediaState::MEDIA_STATE_INACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_INACTIVE_PROPOSAL) &&
            (nState != IMediaState::MEDIA_STATE_ACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_PROPOSAL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set bandwidth in the state (%d)", nState, 0, 0);
        return IMS_FAILURE;
    }

    SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    ImsList<SdpBandwidth> objBandwidths;
    SdpBandwidth objBandwidth;

    for (IMS_UINT32 i = 0; i < strBandwidthInfos.GetSize(); ++i)
    {
        const AString& strBandwidth = strBandwidthInfos.GetAt(i);

        if (!objBandwidth.Decode(strBandwidth))
        {
            Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

            IMS_TRACE_E(0, "Trying to set bandwidth in the state (%d)", nState, 0, 0);
            return IMS_FAILURE;
        }

        objBandwidths.Append(objBandwidth);
    }

    pMediaParam->SetBandwidths(objBandwidths);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::SetMediaTitle(IN const AString& strTitle)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    // Check a media state
    if (nState != IMediaState::MEDIA_STATE_INACTIVE)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(
                0, "Trying to set a title (%s) in the state (%d)", strTitle.GetStr(), nState, 0);
        return IMS_FAILURE;
    }

    SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    SdpInformation objMediaTitle;

    if (!objMediaTitle.Decode(strTitle))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Decoding a media title (%s) failed", strTitle.GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    pMediaParam->SetInformation(objMediaTitle);

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::AddAttribute(IN IMS_SINT32 nType,
        IN const AString& strAttrValue, IN const AString& strType /*= AString::ConstNull()*/)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    if ((nState != IMediaState::MEDIA_STATE_INACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_INACTIVE_PROPOSAL) &&
            (nState != IMediaState::MEDIA_STATE_ACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_PROPOSAL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set an attribute (%d, %s) in the state (%d)", nType,
                strAttrValue.GetStr(), nState);
        return IMS_FAILURE;
    }

    // Special attributes for SDP negotiation
    //    recvonly, sendrecv, sendonly, setup, connection, mid
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY) || (nType == SdpAttribute::SETUP) ||
            (nType == SdpAttribute::CONNECTION))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Not allowed attribute (%d, %s)", nType, strAttrValue.GetStr(), 0);
        return IMS_FAILURE;
    }

    SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (nType == SdpAttribute::MID)
    {
        pMediaParam->SetAttributeMid(strAttrValue);

        Ims::SetLastError(ImsError::NO_ERROR);
        return IMS_SUCCESS;
    }

    SdpAttribute objAttribute;

    // Check a syntax of the attribute
    if (!objAttribute.SetValue(nType, strAttrValue, strType))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Setting an attribute (%s) failed", strAttrValue.GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    // Check if it already exists in the session
    if (pMediaParam->Contains(objAttribute))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Attribute (%s) already exists", objAttribute.GetValue().GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    if (!pMediaParam->AddAttribute(objAttribute))
    {
        Ims::SetLastError(ImsError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Adding an attribute (%s) failed", objAttribute.GetValue().GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::AddAttributeInt(IN IMS_SINT32 nType,
        IN IMS_SINT32 nAttrValue, IN const AString& strType /*= AString::ConstNull()*/)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    if ((nState != IMediaState::MEDIA_STATE_INACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_INACTIVE_PROPOSAL) &&
            (nState != IMediaState::MEDIA_STATE_ACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_PROPOSAL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set an attribute (%d, %d) in the state (%d)", nType, nAttrValue,
                nState);
        return IMS_FAILURE;
    }

    // Special attributes for SDP negotiation
    //    recvonly, sendrecv, sendonly, setup, connection, mid
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Not allowed attribute (%d, %d)", nType, nAttrValue, 0);
        return IMS_FAILURE;
    }
    else if ((nType == SdpAttribute::SETUP) || (nType == SdpAttribute::CONNECTION))
    {
        SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

        if (pMediaParam == IMS_NULL)
        {
            Ims::SetLastError(ImsError::ILLEGAL_STATE);

            IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
            return IMS_FAILURE;
        }

        if (nType == SdpAttribute::SETUP)
        {
            pMediaParam->SetAttributeSetup(nAttrValue);
        }
        else if (nType == SdpAttribute::CONNECTION)
        {
            pMediaParam->SetAttributeConnection(nAttrValue);
        }

        return IMS_SUCCESS;
    }

    AString strAttrValue;

    strAttrValue.SetNumber(nAttrValue);

    return AddAttribute(nType, strAttrValue, strType);
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::AddBandwidth(IN IMS_SINT32 nType,
        IN IMS_SINT32 nBandwidth, IN const AString& strType /*= AString::ConstNull()*/)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    if ((nState != IMediaState::MEDIA_STATE_INACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_INACTIVE_PROPOSAL) &&
            (nState != IMediaState::MEDIA_STATE_ACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_PROPOSAL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to add a bandwidth (%d, %d) in the state (%d)", nType, nBandwidth,
                nState);
        return IMS_FAILURE;
    }

    SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    SdpBandwidth objBandwidth;

    // Check a syntax of the attribute
    if (!objBandwidth.SetValue(nType, nBandwidth, strType))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(
                0, "Decoding a bandwidth (%d, %d, %s) failed", nType, nBandwidth, strType.GetStr());
        return IMS_FAILURE;
    }

    if (!pMediaParam->AddBandwidth(objBandwidth))
    {
        Ims::SetLastError(ImsError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Adding a bandwidth (%s) failed", objBandwidth.GetValue().GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL const AString& MediaDescriptor::GetAttribute(
        IN IMS_SINT32 nType, IN const AString& strType /*= AString::ConstNull()*/) const
{
    const SdpMediaParameter* pMediaParam = m_piMediaState->GetPeerMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer media parameter", 0, 0, 0);
        return AString::ConstNull();
    }

    // Special attributes for SDP negotiation
    //    mid, recvonly, sendrecv, sendonly, setup, connection, mid
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY) || (nType == SdpAttribute::SETUP) ||
            (nType == SdpAttribute::CONNECTION))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Not allowed attribute (%d)", nType, 0, 0);
        return AString::ConstNull();
    }
    else if (nType == SdpAttribute::MID)
    {
        return pMediaParam->GetAttributeMid();
    }

    const SdpAttribute* pAttribute = IMS_NULL;

    if (nType != SdpAttribute::ATTRIBUTE_OTHER)
    {
        pAttribute = pMediaParam->GetAttribute(nType);
    }
    else
    {
        pAttribute = pMediaParam->GetAttribute(strType);
    }

    if (pAttribute == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NOT_FOUND);
        return AString::ConstNull();
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pAttribute->GetAttributeValue();
}

PRIVATE VIRTUAL ImsList<AString> MediaDescriptor::GetAttributes(
        IN IMS_SINT32 nType, IN const AString& strType /*= AString::ConstNull()*/) const
{
    const SdpMediaParameter* pMediaParam = m_piMediaState->GetPeerMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer media parameter", 0, 0, 0);
        return ImsList<AString>();
    }

    // Special attributes for SDP negotiation
    //    mid, recvonly, sendrecv, sendonly, setup, connection, mid
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY) || (nType == SdpAttribute::SETUP) ||
            (nType == SdpAttribute::CONNECTION))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Not allowed attribute (%d)", nType, 0, 0);
        return ImsList<AString>();
    }
    else if (nType == SdpAttribute::MID)
    {
        ImsList<AString> objAttributeValues;

        objAttributeValues.Append(pMediaParam->GetAttributeMid());

        return objAttributeValues;
    }

    ImsList<AString> objAttributeValues;
    const ImsList<SdpAttribute>& objAttributes = pMediaParam->GetAttributes();

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

    Ims::SetLastError(ImsError::NO_ERROR);

    return objAttributeValues;
}

PRIVATE VIRTUAL IMS_SINT32 MediaDescriptor::GetAttributeInt(
        IN IMS_SINT32 nType, IN const AString& strType /*= AString::ConstNull()*/) const
{
    const SdpMediaParameter* pMediaParam = m_piMediaState->GetPeerMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer media parameter", 0, 0, 0);
        return INVALID_VALUE;
    }

    // Special attributes for SDP negotiation
    //    recvonly, sendrecv, sendonly, setup, connection
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Not allowed attribute (%d)", nType, 0, 0);
        return INVALID_VALUE;
    }
    else if (nType == SdpAttribute::SETUP)
    {
        return pMediaParam->GetAttributeSetup();
    }
    else if (nType == SdpAttribute::CONNECTION)
    {
        return pMediaParam->GetAttributeConnection();
    }

    const SdpAttribute* pAttribute = IMS_NULL;

    if (nType != SdpAttribute::ATTRIBUTE_OTHER)
    {
        pAttribute = pMediaParam->GetAttribute(nType);
    }
    else
    {
        pAttribute = pMediaParam->GetAttribute(strType);
    }

    if (pAttribute == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NOT_FOUND);
        return INVALID_VALUE;
    }

    const AString& strAttrValue = pAttribute->GetAttributeValue();

    if (strAttrValue.IsNULL())
    {
        return INVALID_VALUE;
    }

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
                    Ims::SetLastError(ImsError::NO_ERROR);

                    return nAttrValue;
                }
            }
        }

        IMS_TRACE_E(0, "Converting the attribute (integer format: %d, %s) failed", nType,
                strType.GetStr(), 0);

        Ims::SetLastError(ImsError::INVALID_OPERATION);
        return INVALID_VALUE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return nAttrValue;
}

PRIVATE VIRTUAL IMS_SINT32 MediaDescriptor::GetBandwidth(
        IN IMS_SINT32 nType, IN const AString& strType /*= AString::ConstNull()*/) const
{
    const SdpMediaParameter* pMediaParam = m_piMediaState->GetPeerMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer media parameter", 0, 0, 0);
        return INVALID_VALUE;
    }

    const SdpBandwidth* pBandwidth = IMS_NULL;

    if (nType != SdpBandwidth::TYPE_OTHER)
    {
        pBandwidth = pMediaParam->GetBandwidth(nType);
    }
    else
    {
        pBandwidth = pMediaParam->GetBandwidth(strType);
    }

    if (pBandwidth == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NOT_FOUND);
        return INVALID_VALUE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return pBandwidth->GetBandwidth();
}

PRIVATE VIRTUAL IMS_SINT32 MediaDescriptor::GetDirection() const
{
    const SdpMediaParameter* pMediaParam = m_piMediaState->GetPeerMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer media parameter", 0, 0, 0);
        return Sdp::DIRECTION_NONE;
    }

    return pMediaParam->GetDirection();
}

PRIVATE VIRTUAL const SdpMedia* MediaDescriptor::GetMediaDescriptionEx() const
{
    const SdpMediaParameter* pMediaParam = m_piMediaState->GetPeerMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer media parameter", 0, 0, 0);
        return IMS_NULL;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return &(pMediaParam->GetMedia());
}

PRIVATE VIRTUAL const ImsList<SdpMediaFormat*>& MediaDescriptor::GetMediaFormats() const
{
    const SdpMediaParameter* pMediaParam = m_piMediaState->GetPeerMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        static ImsList<SdpMediaFormat*> objMediaFormats;

        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer media parameter", 0, 0, 0);
        return objMediaFormats;
    }

    return pMediaParam->GetMediaFormats();
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::RemoveAttribute(IN const SdpAttribute& objAttribute)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    if ((nState != IMediaState::MEDIA_STATE_INACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_INACTIVE_PROPOSAL) &&
            (nState != IMediaState::MEDIA_STATE_ACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_PROPOSAL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to remove an attribute (%s) in the state (%d)",
                objAttribute.GetAttributeValue().GetStr(), nState, 0);
        return IMS_FAILURE;
    }

    SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Special attributes for SDP negotiation
    //    mid, recvonly, sendrecv, sendonly, setup, connection, mid
    IMS_SINT32 nType = objAttribute.GetAttribute();

    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        IMS_TRACE_E(0, "Not allowed attribute (%d)", nType, 0, 0);
        return IMS_FAILURE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    if (nType == SdpAttribute::SETUP)
    {
        pMediaParam->SetAttributeSetup(Sdp::SETUP_NONE);
        return IMS_SUCCESS;
    }
    else if (nType == SdpAttribute::CONNECTION)
    {
        pMediaParam->SetAttributeConnection(Sdp::CONNECTION_NONE);
        return IMS_SUCCESS;
    }
    else if (nType == SdpAttribute::MID)
    {
        pMediaParam->SetAttributeMid(AString::ConstNull());
        return IMS_SUCCESS;
    }

    // Check if it already exists in the session
    if (!pMediaParam->Contains(objAttribute))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Attribute (%s) does not exist", objAttribute.GetValue().GetStr(), 0, 0);
        return IMS_FAILURE;
    }

    pMediaParam->RemoveAttribute(objAttribute);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::RemoveAttribute(IN IMS_SINT32 nType,
        IN const AString& strAttrValue /*= AString::ConstNull()*/,
        IN const AString& strType /*= AString::ConstNull()*/)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    if ((nState != IMediaState::MEDIA_STATE_INACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_INACTIVE_PROPOSAL) &&
            (nState != IMediaState::MEDIA_STATE_ACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_PROPOSAL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to remove an attribute (%d, %s) in the state (%d)", nType,
                strAttrValue.GetStr(), nState);
        return IMS_FAILURE;
    }

    SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Remove all the attributes if the type is ATTRIBUTE_ALL
    if (nType == SdpAttribute::ATTRIBUTE_ALL)
    {
        Ims::SetLastError(ImsError::NO_ERROR);

        pMediaParam->RemoveAttributes();
        return IMS_SUCCESS;
    }

    // Special attributes for SDP negotiation
    //    mid, recvonly, sendrecv, sendonly, setup, connection, mid
    if ((nType == SdpAttribute::RECVONLY) || (nType == SdpAttribute::SENDRECV) ||
            (nType == SdpAttribute::SENDONLY))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        IMS_TRACE_E(0, "Not allowed attribute (%d)", nType, 0, 0);
        return IMS_FAILURE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    if (nType == SdpAttribute::SETUP)
    {
        pMediaParam->SetAttributeSetup(Sdp::SETUP_NONE);
        return IMS_SUCCESS;
    }
    else if (nType == SdpAttribute::CONNECTION)
    {
        pMediaParam->SetAttributeConnection(Sdp::CONNECTION_NONE);
        return IMS_SUCCESS;
    }
    else if (nType == SdpAttribute::MID)
    {
        pMediaParam->SetAttributeMid(AString::ConstNull());
        return IMS_SUCCESS;
    }

    if (!strAttrValue.IsNULL())
    {
        SdpAttribute objAttribute;

        objAttribute.SetValue(nType, strAttrValue, strType);

        if (!pMediaParam->Contains(objAttribute))
        {
            Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

            IMS_TRACE_E(0, "Attribute (%s) does not exist", objAttribute.GetValue().GetStr(), 0, 0);
            return IMS_FAILURE;
        }

        pMediaParam->RemoveAttribute(objAttribute);

        return IMS_SUCCESS;
    }

    // Check if it already exists in the session
    pMediaParam->RemoveAttribute(nType);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::RemoveMediaFormat(
        IN IMS_SINT32 nType, IN const AString& strValue)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    if ((nState != IMediaState::MEDIA_STATE_INACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_INACTIVE_PROPOSAL) &&
            (nState != IMediaState::MEDIA_STATE_ACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_PROPOSAL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to remove a media format (%d, %s) in the state (%d)", nType,
                strValue.GetStr(), nState);
        return IMS_FAILURE;
    }

    SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    pMediaParam->RemoveMediaFormat(nType, strValue);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::SetConnectionAddress(IN const AString& strAddress)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    if ((nState != IMediaState::MEDIA_STATE_INACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_INACTIVE_PROPOSAL) &&
            (nState != IMediaState::MEDIA_STATE_ACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_PROPOSAL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set a media connection address (%s) in the state (%d)",
                SipDebug::GetIp(strAddress), nState, 0);
        return IMS_FAILURE;
    }

    SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (!pMediaParam->SetConnectionAddress(strAddress))
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);

        IMS_TRACE_E(0, "Setting a connection address (%s) in c-line of media-level",
                SipDebug::GetIp(strAddress), 0, 0);
        return IMS_FAILURE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::SetDirection(IN IMS_SINT32 nDirection)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    if ((nState != IMediaState::MEDIA_STATE_INACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_INACTIVE_PROPOSAL) &&
            (nState != IMediaState::MEDIA_STATE_ACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_PROPOSAL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(
                0, "Trying to set a media direction (%d) in the state (%d)", nDirection, nState, 0);
        return IMS_FAILURE;
    }

    SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    pMediaParam->SetDirection(nDirection);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::SetMediaDescription(IN IMS_SINT32 nType,
        IN IMS_SINT32 nPort, IN IMS_SINT32 nTransportProtocol, IN const AStringArray& objFormats)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    if ((nState != IMediaState::MEDIA_STATE_INACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_INACTIVE_PROPOSAL) &&
            (nState != IMediaState::MEDIA_STATE_ACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_PROPOSAL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(
                0, "Trying to set a media description (%d) in the state (%d)", nType, nState, 0);
        return IMS_FAILURE;
    }

    SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (!pMediaParam->SetMedia(nType, nPort, nTransportProtocol, objFormats))
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::SetMediaFormat(IN const SdpMediaFormat* pMediaFormat)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    if (pMediaFormat == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if ((nState != IMediaState::MEDIA_STATE_INACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_INACTIVE_PROPOSAL) &&
            (nState != IMediaState::MEDIA_STATE_ACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_PROPOSAL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set a media format (%d, %s) in the state (%d)",
                pMediaFormat->GetType(), pMediaFormat->GetValue().GetStr(), nState);
        return IMS_FAILURE;
    }

    SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    SdpMediaFormat* pTmpMediaFormat =
            pMediaParam->GetMediaFormat(pMediaFormat->GetType(), pMediaFormat->GetValue());

    if (pTmpMediaFormat == IMS_NULL)
    {
        IMS_TRACE_E(0, "Format (%d, %s) does not exist", pMediaFormat->GetType(),
                pMediaFormat->GetValue().GetStr(), 0);
        return IMS_FAILURE;
    }

    switch (pTmpMediaFormat->GetType())
    {
        case SdpMediaFormat::TYPE_RTP:
        {
            const SdpAvCodec* pAvCodec = DYNAMIC_CAST(SdpAvCodec*, pMediaFormat);
            SdpAvCodec* pTmpAvCodec = DYNAMIC_CAST(SdpAvCodec*, pTmpMediaFormat);

            if ((pAvCodec == IMS_NULL) || (pTmpAvCodec == IMS_NULL))
            {
                IMS_TRACE_E(0, "AVCodec is null", 0, 0, 0);
                return IMS_FAILURE;
            }

            AString strRtpMap;
            AString strFmtp = AString::ConstNull();

            // rtpmap
            strRtpMap.Sprintf("%s %s/%d", pAvCodec->GetValue().GetStr(),
                    pAvCodec->GetName().GetStr(), pAvCodec->GetClockRate());

            if (pAvCodec->GetEncodingParameters().GetLength() > 0)
            {
                strRtpMap.Append(TextParser::CHAR_SLASH);
                strRtpMap.Append(pAvCodec->GetEncodingParameters());
            }

            // fmtp
            if (!pAvCodec->GetFormatSpecificParameter().IsNULL())
            {
                strFmtp.Sprintf("%s %s", pAvCodec->GetValue().GetStr(),
                        pAvCodec->GetFormatSpecificParameter().GetStr());
            }

            if (!pTmpAvCodec->SetParameters(strRtpMap, strFmtp))
            {
                IMS_TRACE_E(0, "Setting RTPMAP (%s) & FMTP (%s) failed", strRtpMap.GetStr(),
                        strFmtp.GetStr(), 0);
                return IMS_FAILURE;
            }

            // Media format parameter :: rtcp-fb / framesize / ...
            const ImsList<SdpMediaFormatParameter*>& objExtraParameters =
                    pMediaFormat->GetExtraParameters();

            for (IMS_UINT32 i = 0; i < objExtraParameters.GetSize(); ++i)
            {
                const SdpMediaFormatParameter* pParameter = objExtraParameters.GetAt(i);

                if (pParameter == IMS_NULL)
                {
                    continue;
                }

                pTmpAvCodec->AddExtraParameter(pParameter->Clone());
            }
            break;
        }
        default:
            IMS_TRACE_E(0, "RTP format only supports", 0, 0, 0);
            return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::SetMediaFormat(IN IMS_SINT32 nType,
        IN const AString& strValue, IN const AString& strAnyMap, IN const AString& strFmtp)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    if ((nState != IMediaState::MEDIA_STATE_INACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_INACTIVE_PROPOSAL) &&
            (nState != IMediaState::MEDIA_STATE_ACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_PROPOSAL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set a media format (%d, %s) in the state (%d)", nType,
                strValue.GetStr(), nState);
        return IMS_FAILURE;
    }

    SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    SdpMediaFormat* pMediaFormat = pMediaParam->GetMediaFormat(nType, strValue);

    if (pMediaFormat == IMS_NULL)
    {
        IMS_TRACE_E(0, "Format (%d, %s) does not exist", nType, strValue.GetStr(), 0);
        return IMS_FAILURE;
    }

    AString strAttrAnyMap;
    AString strAttrFmtp = AString::ConstNull();

    if (strAnyMap.GetLength() > 0)
    {
        strAttrAnyMap = strValue + TextParser::CHAR_SP + strAnyMap;
    }

    if (!strFmtp.IsNULL())
    {
        strAttrFmtp = strValue + TextParser::CHAR_SP + strFmtp;
    }

    if (!pMediaFormat->SetParameters(strAttrAnyMap, strAttrFmtp))
    {
        IMS_TRACE_E(0, "Setting AnyMAP (%s) & FMTP (%s) failed", strAnyMap.GetStr(),
                strFmtp.GetStr(), 0);
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::SetPort(IN IMS_SINT32 nPort)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    if ((nState != IMediaState::MEDIA_STATE_INACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_INACTIVE_PROPOSAL) &&
            (nState != IMediaState::MEDIA_STATE_ACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_PROPOSAL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to set a media port (%d) in the state (%d)", nPort, nState, 0);
        return IMS_FAILURE;
    }

    SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    SdpMedia& objMedia = const_cast<SdpMedia&>(pMediaParam->GetMedia());

    objMedia.SetPort(nPort);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL const SdpMedia* MediaDescriptor::GetMediaDescriptionExAsLocal() const
{
    const SdpMediaParameter* pMediaParam = m_piMediaState->GetMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No media parameter", 0, 0, 0);
        return IMS_NULL;
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    return &(pMediaParam->GetMedia());
}

PRIVATE VIRTUAL IpAddress MediaDescriptor::GetLocalAddress() const
{
    const SdpMediaParameter* pMediaParam = m_piMediaState->GetMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        IMS_TRACE_E(0, "No media parameter", 0, 0, 0);
        return IpAddress::NONE;
    }

    IpAddress objAddress;

    // Frist, media-level connection address will be checked
    if (pMediaParam->IsConnectionPresent())
    {
        if (!objAddress.Parse(pMediaParam->GetConnectionAddress()))
        {
            IMS_TRACE_E(0, "Parsing IpAddress failed", 0, 0, 0);
            return IpAddress::NONE;
        }
    }
    else
    {
        if (!objAddress.Parse(m_piMediaState->GetConnectionAddress()))
        {
            IMS_TRACE_E(0, "Parsing IpAddress failed", 0, 0, 0);
            return IpAddress::NONE;
        }
    }

    return objAddress;
}

PRIVATE VIRTUAL IMS_SINT32 MediaDescriptor::GetLocalPort() const
{
    const SdpMediaParameter* pMediaParam = m_piMediaState->GetMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        IMS_TRACE_E(0, "No media parameter", 0, 0, 0);
        return 0;
    }

    return pMediaParam->GetMedia().GetPort();
}

PRIVATE VIRTUAL IpAddress MediaDescriptor::GetRemoteAddress() const
{
    IpAddress objAddress;

    if (!objAddress.Parse(GetRemoteAddressAsString()))
    {
        IMS_TRACE_D("Remote connection address may be FQDN or null", 0, 0, 0);
        return IpAddress::NONE;
    }

    return objAddress;
}

PRIVATE VIRTUAL const AString& MediaDescriptor::GetRemoteAddressAsString() const
{
    const SdpMediaParameter* pMediaParam = m_piMediaState->GetPeerMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        IMS_TRACE_E(0, "No peer media parameter", 0, 0, 0);
        return AString::ConstNull();
    }

    // Frist, media-level connection address will be checked
    if (pMediaParam->IsConnectionPresent())
    {
        return pMediaParam->GetConnectionAddress();
    }

    return m_piMediaState->GetPeerConnectionAddress();
}

PRIVATE VIRTUAL IMS_SINT32 MediaDescriptor::GetRemotePort() const
{
    const SdpMediaParameter* pMediaParam = m_piMediaState->GetPeerMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        IMS_TRACE_E(0, "No peer media parameter", 0, 0, 0);
        return 0;
    }

    return pMediaParam->GetMedia().GetPort();
}

// IMS_SDP_PRECONDITION
PRIVATE VIRTUAL const SdpPrecondition* MediaDescriptor::GetPrecondition(
        IN IMS_SINT32 nAttribute, IN IMS_SINT32 nType /*= SdpPrecondition::TYPE_QOS*/) const
{
    const SdpMediaParameter* pMediaParam = m_piMediaState->GetPeerMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No peer media parameter", 0, 0, 0);
        return IMS_NULL;
    }

    return pMediaParam->GetPrecondition(nAttribute, nType);
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::RemovePrecondition(
        IN IMS_SINT32 nAttribute, IN IMS_SINT32 nType /*= SdpPrecondition::TYPE_QOS*/)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    if ((nState != IMediaState::MEDIA_STATE_INACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_INACTIVE_PROPOSAL) &&
            (nState != IMediaState::MEDIA_STATE_ACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_PROPOSAL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "Trying to remove a precondition (%d, %d) in the state (%d)", nAttribute,
                nType, nState);
        return IMS_FAILURE;
    }

    SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    pMediaParam->RemovePrecondition(nAttribute, nType);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT MediaDescriptor::SetPrecondition(
        IN IMS_SINT32 nAttribute, IN const SdpPrecondition* pPrecondition)
{
    // Check a media state
    IMS_SINT32 nState = m_piMediaState->GetMediaState();

    if ((nState != IMediaState::MEDIA_STATE_INACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_INACTIVE_PROPOSAL) &&
            (nState != IMediaState::MEDIA_STATE_ACTIVE) &&
            (nState != IMediaState::MEDIA_STATE_PROPOSAL))
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(
                0, "Trying to set a precondition (%d) in the state (%d)", nAttribute, nState, 0);
        return IMS_FAILURE;
    }

    SdpMediaParameter* pMediaParam = m_piMediaState->GetProposalMediaParameter(m_nMid);

    if (pMediaParam == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_STATE);

        IMS_TRACE_E(0, "No proposal media parameter", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (!pMediaParam->SetPrecondition(nAttribute, const_cast<SdpPrecondition*>(pPrecondition)))
    {
        Ims::SetLastError(ImsError::GENERAL_ERROR);

        IMS_TRACE_E(0, "Setting a precondition (%d) failed", nAttribute, 0, 0);
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PRIVATE GLOBAL IMS_BOOL MediaDescriptor::IsAttributeReserved(
        IN const SdpMediaParameter* pMediaParam, IN const AString& strAttribute)
{
    if (pMediaParam == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (strAttribute.IsNULL() || strAttribute.IsEmpty())
    {
        return IMS_FALSE;
    }

    IMS_SINT32 i;

    for (i = START_COMMON; i <= END_COMMON; ++i)
    {
        if (strAttribute.EqualsIgnoreCase(RESERVED_ATTRIBUTE[i]))
        {
            return IMS_TRUE;
        }
    }

    IMS_SINT32 nTransportProtocol = pMediaParam->GetMedia().GetTransportProtocol();

    // StreamMedia attributes
    if ((nTransportProtocol == SdpMedia::TRANSPORT_RTP_AVP) ||
            (nTransportProtocol == SdpMedia::TRANSPORT_RTP_AVPF) ||
            (nTransportProtocol == SdpMedia::TRANSPORT_RTP_SAVP) ||
            (nTransportProtocol == SdpMedia::TRANSPORT_RTP_SAVPF))
    {
        for (i = START_STREAM_MEDIA; i <= END_STREAM_MEDIA; ++i)
        {
            if (strAttribute.EqualsIgnoreCase(RESERVED_ATTRIBUTE[i]))
            {
                return IMS_TRUE;
            }
        }
    }
    // FramedMedia attributes
    else if ((nTransportProtocol == SdpMedia::TRANSPORT_TCP_MSRP) ||
            (nTransportProtocol == SdpMedia::TRANSPORT_TCP_TLS_MSRP))
    {
        for (i = START_FRAMED_MEDIA; i <= END_FRAMED_MEDIA; ++i)
        {
            if (strAttribute.EqualsIgnoreCase(RESERVED_ATTRIBUTE[i]))
            {
                return IMS_TRUE;
            }
        }
    }
    // BasicReliableMedia attributes
    else if (nTransportProtocol == SdpMedia::TRANSPORT_TCP)
    {
        for (i = START_BASIC_RELIABLE_MEDIA; i <= END_BASIC_RELIABLE_MEDIA; ++i)
        {
            if (strAttribute.EqualsIgnoreCase(RESERVED_ATTRIBUTE[i]))
            {
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}
