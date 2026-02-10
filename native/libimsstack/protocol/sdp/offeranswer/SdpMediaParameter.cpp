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
#include "ImsMap.h"
#include "IpAddress.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "SdpEncryptionKey.h"
#include "SdpInformation.h"
#include "SdpMediaDescription.h"
#include "offeranswer/SdpAvCodec.h"
#include "offeranswer/SdpE2EPrecondition.h"
#include "offeranswer/SdpFramesize.h"
#include "offeranswer/SdpMediaParameter.h"
#include "offeranswer/SdpOfferAnswer.h"
#include "offeranswer/SdpProfile.h"
#include "offeranswer/SdpRtcpFeedback.h"
#include "offeranswer/SdpSegmentedPrecondition.h"

#define __IMS_MEDIA_FORMAT_PREFERENCE_ORDER_BY_REMOTE_ENDPOINT__

__IMS_TRACE_TAG_SDP__;

PUBLIC
SdpMediaParameter::SdpMediaParameter(IN IMS_SINT32 nMid) :
        SdpParameter(),
        m_nMid(nMid),
        m_strAttrMid(AString::ConstNull()),
        m_pCurrentStatus(IMS_NULL),
        m_pDesiredStatus(IMS_NULL),
        m_pConfirmedStatus(IMS_NULL)
{
    for (IMS_SINT32 i = 0; i < ATTR_MAX; ++i)
    {
        m_abAttributeContains[i] = IMS_FALSE;
    }
}

PUBLIC
SdpMediaParameter::SdpMediaParameter(IN const SdpMediaParameter& other) :
        SdpParameter(other),
        m_nMid(other.m_nMid),
        m_objMedia(other.m_objMedia),
        m_objConnections(other.m_objConnections),
        m_objPrevConnections(other.m_objPrevConnections),
        m_strAttrMid(other.m_strAttrMid),
        m_pCurrentStatus(IMS_NULL),
        m_pDesiredStatus(IMS_NULL),
        m_pConfirmedStatus(IMS_NULL)
{
    CopyMediaFormat(other.m_objMediaFormats, m_objMediaFormats);

    for (IMS_SINT32 i = 0; i < ATTR_MAX; ++i)
    {
        m_abAttributeContains[i] = other.m_abAttributeContains[i];
    }

#if defined(__IMS_SDP_PRECONDITION__)
    {
        CopyPrecondition(other, *this);
    }
#endif
}

PUBLIC
SdpMediaParameter::~SdpMediaParameter()
{
    m_objConnections.Clear();
    m_objPrevConnections.Clear();

    ClearMediaFormat(m_objMediaFormats);

#if defined(__IMS_SDP_PRECONDITION__)
    {
        ClearPrecondition(this);
    }
#endif
}

PUBLIC
SdpMediaParameter& SdpMediaParameter::operator=(IN const SdpMediaParameter& other)
{
    if (this != &other)
    {
        Clear();

        SdpParameter::operator=(other);

        m_objMedia = other.m_objMedia;
        m_objConnections = other.m_objConnections;
        m_objPrevConnections = other.m_objPrevConnections;

        CopyMediaFormat(other.m_objMediaFormats, m_objMediaFormats);

        m_strAttrMid = other.m_strAttrMid;

        for (IMS_SINT32 i = 0; i < ATTR_MAX; ++i)
        {
            m_abAttributeContains[i] = other.m_abAttributeContains[i];
        }

#if defined(__IMS_SDP_PRECONDITION__)
        {
            CopyPrecondition(other, *this);
        }
#endif
    }

    return (*this);
}

PUBLIC VIRTUAL const AString& SdpMediaParameter::GetConnectionAddress() const
{
    if (m_objConnections.IsEmpty())
    {
        return AString::ConstNull();
    }

    const SdpConnection& objConnection = m_objConnections.GetAt(0);

    return objConnection.GetAddress();
}

PUBLIC VIRTUAL AString SdpMediaParameter::ToSdp() const
{
    // SDP order: m, i, c, b, k, *(a)
    AStringBuffer objSdp(512);

    objSdp.Append(m_objMedia.Encode());

    if (m_objMedia.GetPort() == 0)
    {
        // If the media type or codec does not support,
        // we don't need to set the other lines (b, a, ...).
        // So, if the m-line is not supported by the endpoint, m-line only will be set.

        // Because of 'file-transfer-id', all SDP will be formed.
        // return strSDP;
    }

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

    // Attribute: rtpmap & fmtp if present
    for (IMS_UINT32 i = 0; i < m_objMediaFormats.GetSize(); ++i)
    {
        const SdpMediaFormat* pMediaFormat = m_objMediaFormats.GetAt(i);

        if (pMediaFormat->HasAttribute())
        {
            objSdp.Append(pMediaFormat->ToSdp());
        }
    }

    // Extra parameters : rtcp-fb (wildcard), framesize (non-standard)
    for (IMS_UINT32 i = 0; i < m_objMediaFormats.GetSize(); ++i)
    {
        const SdpMediaFormat* pMediaFormat = m_objMediaFormats.GetAt(i);

        if (pMediaFormat->HasAttribute())
        {
            const ImsList<SdpMediaFormatParameter*>& objExtraParameters =
                    pMediaFormat->GetExtraParameters();

            for (IMS_UINT32 j = 0; j < objExtraParameters.GetSize(); ++j)
            {
                const SdpMediaFormatParameter* pParameter = objExtraParameters.GetAt(j);

                if (pParameter == IMS_NULL)
                {
                    continue;
                }

                IMS_SINT32 nPayloadType = pParameter->GetPayloadTypeNumber();

                if ((nPayloadType == SdpMediaFormatParameter::PT_WILDCARD) ||
                        (nPayloadType == SdpMediaFormatParameter::PT_NOT_SPECIFIED))
                {
                    objSdp.Append(pParameter->ToSdp());
                }
            }
            // Wildcard attribute will be included all the media format.
            // so, just add wildcard attribute one time.
            break;
        }
    }

    if (m_abAttributeContains[ATTR_MID])
    {
        SdpAttribute objAttr;

        objAttr.SetValue(SdpAttribute::MID, m_strAttrMid);
        objSdp.Append(objAttr.Encode());
    }

#if defined(__IMS_SDP_PRECONDITION__)
    {
        if (m_abAttributeContains[ATTR_QOS_CURR])
        {
            if (m_pCurrentStatus != IMS_NULL)
            {
                objSdp.Append(m_pCurrentStatus->ToSdp(SdpAttribute::CURR));
            }
        }

        if (m_abAttributeContains[ATTR_QOS_DES])
        {
            if (m_pDesiredStatus != IMS_NULL)
            {
                objSdp.Append(m_pDesiredStatus->ToSdp(SdpAttribute::DES));
            }
        }

        if (m_abAttributeContains[ATTR_QOS_CONF])
        {
            if (m_pConfirmedStatus != IMS_NULL)
            {
                objSdp.Append(m_pConfirmedStatus->ToSdp(SdpAttribute::CONF));
            }
        }
    }
#endif

    objSdp.Append(SdpParameter::ToSdp());

    return static_cast<const AStringBuffer&>(objSdp).GetString();
}

PUBLIC
IMS_SINT32 SdpMediaParameter::Compare(IN IMS_BOOL bInitialOffer, IN IMS_BOOL bIsOffer,
        IN const SdpMediaParameter* pPeerParam, OUT SdpMediaParameter*& pNegotiatedPeerParam,
        OUT SdpMediaParameter*& pProposalParam)
{
    ImsList<SdpMediaFormat*> objAcceptedMediaFormats;
    ImsList<SdpMediaFormat*> objPeerMediaFormats;
    IMS_BOOL bAtLeastOneCodecMatched = IMS_FALSE;

    // Copy the m-line from the peer view
    pProposalParam->m_objMedia = pPeerParam->m_objMedia;

    if (pPeerParam->m_objMedia.GetPort() == 0)
    {
        // It is a negotiated stream and which has been rejected by the peer,
        // so set the flag to TRUE so that the stream will be added to negotiated session as it is.
        bAtLeastOneCodecMatched = IMS_TRUE;
    }
    else
    {
        // Check the media type & transport protocol
        if (pPeerParam->m_objMedia.GetType() != m_objMedia.GetType())
        {
            return SdpOfferAnswer::RESULT_NOT_FOUND;
        }

        if (pPeerParam->m_objMedia.GetTransportProtocol() != m_objMedia.GetTransportProtocol())
        {
            // SDPCapNego
            const SdpAttribute* pAttrTcap = GetAttribute(SdpAttribute::TCAP);

            if (!bIsOffer && (pAttrTcap != IMS_NULL))
            {
                IMS_SINT32 nLocalTransportProtocol = m_objMedia.GetTransportProtocol();
                IMS_SINT32 nPeerTransportProtocol = pPeerParam->m_objMedia.GetTransportProtocol();

                if (((nLocalTransportProtocol == SdpMedia::TRANSPORT_RTP_AVP) ||
                            (nLocalTransportProtocol == SdpMedia::TRANSPORT_RTP_SAVP)) &&
                        ((nPeerTransportProtocol == SdpMedia::TRANSPORT_RTP_AVPF) ||
                                (nPeerTransportProtocol == SdpMedia::TRANSPORT_RTP_SAVP) ||
                                (nPeerTransportProtocol == SdpMedia::TRANSPORT_RTP_SAVPF) ||
                                (nPeerTransportProtocol == SdpMedia::TRANSPORT_UDP_TLS_RTP_SAVP)))
                {
                    // Overwrite the transport protocol according to the SDP answer
                    // Add if "tcap" attribute is provided in the previous SDP offer
                    IMS_TRACE_D("SDPCapNego :: Media transport protocol will be changed"
                                "(%d >> %d)",
                            nLocalTransportProtocol, nPeerTransportProtocol, 0);

                    m_objMedia.SetTransportProtocol(nPeerTransportProtocol);
                }
                // Both values can be same.
                // cppcheck-suppress knownConditionTrueFalse
                else if (nLocalTransportProtocol != nPeerTransportProtocol)
                {
                    // Overwrite the transport protocol according to the SDP answer
                    // Add if "tcap" attribute is provided in the previous SDP offer
                    IMS_TRACE_D("SDPCapNego(2) :: Media transport protocol will be changed"
                                "(%d >> %d)",
                            nLocalTransportProtocol, nPeerTransportProtocol, 0);

                    m_objMedia.SetTransportProtocol(nPeerTransportProtocol);
                }
            }

            if (pPeerParam->m_objMedia.GetTransportProtocol() != m_objMedia.GetTransportProtocol())
            {
                return SdpOfferAnswer::RESULT_NOT_FOUND;
            }
        }

        const ImsList<SdpMediaFormat*>* pOfferedMediaFormats = IMS_NULL;
        const ImsList<SdpMediaFormat*>* pAnsweredMediaFormats = IMS_NULL;

// MEDIA_FORMAT_PREFERENCE_ORDER_BY_REMOTE_ENDPOINT
#ifdef __IMS_MEDIA_FORMAT_PREFERENCE_ORDER_BY_REMOTE_ENDPOINT__
        pOfferedMediaFormats = &(pPeerParam->m_objMediaFormats);
        pAnsweredMediaFormats = &m_objMediaFormats;
#else
        if (bIsOffer)
        {
            pOfferedMediaFormats = &(pPeerParam->m_objMediaFormats);
            pAnsweredMediaFormats = &m_objMediaFormats;
        }
        else
        {
            pOfferedMediaFormats = &m_objMediaFormats;
            pAnsweredMediaFormats = &(pPeerParam->m_objMediaFormats);
        }
#endif

        for (IMS_UINT32 i = 0; i < pOfferedMediaFormats->GetSize(); ++i)
        {
            IMS_BOOL bMediaFormatFound = IMS_FALSE;
            SdpMediaFormat* pOfferedMediaFormat = pOfferedMediaFormats->GetAt(i);

            for (IMS_UINT32 j = 0; j < pAnsweredMediaFormats->GetSize(); ++j)
            {
                SdpMediaFormat* pLocalMediaFormat;
                SdpMediaFormat* pPeerMediaFormat;
                SdpMediaFormat* pAnsweredMediaFormat = pAnsweredMediaFormats->GetAt(j);

                if (bIsOffer)
                {
                    pLocalMediaFormat = pAnsweredMediaFormat;
                    pPeerMediaFormat = pOfferedMediaFormat;

                    // Check the payload type and if it is duplicate format,
                    // then skip the media format
                    if (IsSameAvCodecPresent(objPeerMediaFormats, pPeerMediaFormat))
                    {
                        break;
                    }
                }
                else
                {
                    // MEDIA_FORMAT_PREFERENCE_ORDER_BY_REMOTE_ENDPOINT
#ifdef __IMS_MEDIA_FORMAT_PREFERENCE_ORDER_BY_REMOTE_ENDPOINT__
                    pLocalMediaFormat = pAnsweredMediaFormat;
                    pPeerMediaFormat = pOfferedMediaFormat;

                    // Check the payload type and if it is duplicate format,
                    // then skip the media format
                    if (IsSameAvCodecPresent(objPeerMediaFormats, pPeerMediaFormat))
                    {
                        break;
                    }
#else
                    pLocalMediaFormat = pOfferedMediaFormat;
                    pPeerMediaFormat = pAnsweredMediaFormat;
#endif

                    // Check the payload type and if it is duplicate format,
                    // then skip the media format
                    if (IsSameAvCodecPresent(objAcceptedMediaFormats, pLocalMediaFormat))
                    {
                        continue;
                    }
                }

                if (pOfferedMediaFormat->Equals(pAnsweredMediaFormat))
                {
                    if (bIsOffer && !bInitialOffer)
                    {
                        const AString& strOfferedPayloadType = pOfferedMediaFormat->GetValue();
                        const AString& strAnsweredPayloadType = pAnsweredMediaFormat->GetValue();

                        // Payload type should be retained during the session.
                        if (!strOfferedPayloadType.Equals(strAnsweredPayloadType))
                        {
                            IMS_TRACE_D("Payload type (O:%s, A:%s) is not matched "
                                        "during the session; skipped...",
                                    strOfferedPayloadType.GetStr(), strAnsweredPayloadType.GetStr(),
                                    0);

                            // Check the payload type and if it is duplicate format,
                            // then skip the media format
                            if (!IsSameAvCodecPresent(objPeerMediaFormats, pPeerMediaFormat))
                            {
                                ImsList<SdpMediaFormat*> objTempMediaFormats;

                                // Update the peer media formats
                                objTempMediaFormats.Append(pPeerMediaFormat);
                                CopyMediaFormat(objTempMediaFormats, objPeerMediaFormats);

                                IMS_TRACE_D("Payload type (%s) is added during an active call "
                                            "as preferred codec...",
                                        pPeerMediaFormat->GetValue().GetStr(), 0, 0);
                            }

                            continue;
                        }
                    }

                    bAtLeastOneCodecMatched = IMS_TRUE;

                    ImsList<SdpMediaFormat*> objTempMediaFormats;

                    // Update the peer media formats
                    objTempMediaFormats.Append(pPeerMediaFormat);
                    CopyMediaFormat(objTempMediaFormats, objPeerMediaFormats);

                    // Update the local media formats
                    objTempMediaFormats.Clear();
                    objTempMediaFormats.Append(pLocalMediaFormat);
                    CopyMediaFormat(objTempMediaFormats, objAcceptedMediaFormats);

                    bMediaFormatFound = IMS_TRUE;
                }
            }

            // If the re-offer has a preferred new media formats
            // before the previous negotiated media formats,
            // then add a new media format with the preference order.
            if (bIsOffer && !bInitialOffer && !bMediaFormatFound &&
                    !IsSameAvCodecPresent(objPeerMediaFormats, pOfferedMediaFormat) &&
                    !IsSameNonAvCodecPresent(objPeerMediaFormats, pOfferedMediaFormat))
            {
                if (!bAtLeastOneCodecMatched)
                {
                    IMS_TRACE_D("New media format is detected...", 0, 0, 0);
                    bAtLeastOneCodecMatched = IMS_TRUE;
                }

                ImsList<SdpMediaFormat*> objTempMediaFormats;

                // Update the peer media formats
                objTempMediaFormats.Append(pOfferedMediaFormat);
                CopyMediaFormat(objTempMediaFormats, objPeerMediaFormats);

                // Update the local media formats
                CopyMediaFormat(objTempMediaFormats, objAcceptedMediaFormats);

                IMS_TRACE_D("1:Media format(%s, %d) is added during an active call",
                        pOfferedMediaFormat->GetValue().GetStr(), objAcceptedMediaFormats.GetSize(),
                        0);
            }
        }

        if (bIsOffer)
        {
            // In case of a new offer during active call, new media format can be added,
            // so if new media format is present, then add it to the accepted media formats.
            for (IMS_UINT32 j = 0; j < pPeerParam->m_objMediaFormats.GetSize(); ++j)
            {
                SdpMediaFormat* pPeerMediaFormat = pPeerParam->m_objMediaFormats.GetAt(j);

                // Check the payload type and if it is duplicate format,
                // then skip the media format
                if (IsSameAvCodecPresent(objPeerMediaFormats, pPeerMediaFormat))
                {
                    continue;
                }

                if (IsSameNonAvCodecPresent(objPeerMediaFormats, pPeerMediaFormat))
                {
                    continue;
                }

                if (!bAtLeastOneCodecMatched)
                {
                    IMS_TRACE_D("New media format only exists", 0, 0, 0);
                    bAtLeastOneCodecMatched = IMS_TRUE;
                }

                ImsList<SdpMediaFormat*> objTempMediaFormats;

                // Update the peer media formats
                objTempMediaFormats.Append(pPeerMediaFormat);
                CopyMediaFormat(objTempMediaFormats, objPeerMediaFormats);

                // Update the local media formats
                CopyMediaFormat(objTempMediaFormats, objAcceptedMediaFormats);

                IMS_TRACE_D("2:Media format(%s, %d) is added during an active call",
                        pPeerMediaFormat->GetValue().GetStr(), objAcceptedMediaFormats.GetSize(),
                        0);
            }

            if (!bInitialOffer && !bAtLeastOneCodecMatched)
            {
                IMS_TRACE_D("All media formats are not matched during the session;"
                            " it accepts all the peer media formats",
                        0, 0, 0);

                CopyMediaFormat(objPeerMediaFormats, objAcceptedMediaFormats);

                if (!objAcceptedMediaFormats.IsEmpty())
                {
                    bAtLeastOneCodecMatched = IMS_TRUE;
                }
            }
        }
    }

    if (bAtLeastOneCodecMatched)
    {
        if (!ValidateDirection(pPeerParam))
        {
            ClearMediaFormat(objAcceptedMediaFormats);
            ClearMediaFormat(objPeerMediaFormats);
            return SdpOfferAnswer::RESULT_FAILURE;
        }

        // Update the media direction
        pProposalParam->UpdateDirection(*pPeerParam);

        if (pPeerParam->m_objMedia.GetPort() != 0)
        {
            // Now, remove the existing codec list from the negotiated media stream
            // and update it with the negotiated codec list constructed.
            ClearMediaFormat(pProposalParam->m_objMediaFormats);
            CopyMediaFormat(objAcceptedMediaFormats, pProposalParam->m_objMediaFormats);
        }

        pNegotiatedPeerParam = new SdpMediaParameter(*pProposalParam);

        if (pNegotiatedPeerParam != IMS_NULL)
        {
            // Update the negotiated media formats
            ClearMediaFormat(pNegotiatedPeerParam->m_objMediaFormats);

            if (pPeerParam->m_objMedia.GetPort() != 0)
            {
                CopyMediaFormat(objPeerMediaFormats, pNegotiatedPeerParam->m_objMediaFormats);
            }
            else
            {
                CopyMediaFormat(
                        pPeerParam->m_objMediaFormats, pNegotiatedPeerParam->m_objMediaFormats);
            }
        }

        ClearMediaFormat(objAcceptedMediaFormats);
        ClearMediaFormat(objPeerMediaFormats);

        // Set the proposed media identifier to the media identifier of the local current view
        pProposalParam->m_nMid = m_nMid;

        return SdpOfferAnswer::RESULT_SUCCESS;
    }

    ClearMediaFormat(objAcceptedMediaFormats);
    ClearMediaFormat(objPeerMediaFormats);

    return SdpOfferAnswer::RESULT_NOT_FOUND;
}

PUBLIC
IMS_BOOL SdpMediaParameter::Create(IN const SdpMediaDescription& objMediaDescription)
{
    Clear();

    // m-line
    m_objMedia = objMediaDescription.GetMedia();

    // c-line (media connection information)
    m_objPrevConnections.Clear();
    m_objPrevConnections = m_objConnections;

    m_objConnections.Clear();
    if (objMediaDescription.Contains(Sdp::TYPE_C))
    {
        m_objConnections = objMediaDescription.GetConnections();
    }

    // Codecs (in case of RTP/AVP or RTP/SAVP) & format-specific parameters
    // a=rtpmap:xxxx , a=fmtp:xxxx
    ClearMediaFormat(m_objMediaFormats);

    if (!ExtractMediaFormat(objMediaDescription, m_objMediaFormats))
    {
        return IMS_FALSE;
    }

    if (!SdpParameter::Create(objMediaDescription))
    {
        return IMS_FALSE;
    }

    // MID (a=mid:xxxx)
    const SdpAttribute* pMid = GetAttribute(SdpAttribute::MID);

    if (pMid == IMS_NULL)
    {
        m_abAttributeContains[ATTR_MID] = IMS_FALSE;
        m_strAttrMid = AString::ConstNull();
    }
    else
    {
        m_abAttributeContains[ATTR_MID] = IMS_TRUE;
        m_strAttrMid = pMid->GetAttributeValue();

        RemoveAttribute(SdpAttribute::MID);
    }

#if defined(__IMS_SDP_PRECONDITION__)
    {
        // QoS attributes (a=curr:xxxx, a=des:xxxx, a=conf:xxxx)
        ImsList<SdpAttribute> objAttrs = GetAttributes(SdpAttribute::CURR);

        if (!objAttrs.IsEmpty())
        {
            ImsList<SdpAttribute> objQoSAttrs;

            m_pCurrentStatus = CreatePrecondition(objAttrs, objQoSAttrs);

            if (m_pCurrentStatus != IMS_NULL)
            {
                m_abAttributeContains[ATTR_QOS_CURR] = IMS_TRUE;

                for (IMS_UINT32 i = 0; i < objQoSAttrs.GetSize(); ++i)
                {
                    RemoveAttribute(objQoSAttrs.GetAt(i));
                }
            }
        }

        objAttrs = GetAttributes(SdpAttribute::DES);

        if (!objAttrs.IsEmpty())
        {
            ImsList<SdpAttribute> objQoSAttrs;

            m_pDesiredStatus = CreatePrecondition(objAttrs, objQoSAttrs);

            if (m_pDesiredStatus != IMS_NULL)
            {
                m_abAttributeContains[ATTR_QOS_DES] = IMS_TRUE;

                for (IMS_UINT32 i = 0; i < objQoSAttrs.GetSize(); ++i)
                {
                    RemoveAttribute(objQoSAttrs.GetAt(i));
                }
            }
        }

        objAttrs = GetAttributes(SdpAttribute::CONF);

        if (!objAttrs.IsEmpty())
        {
            ImsList<SdpAttribute> objQoSAttrs;

            m_pConfirmedStatus = CreatePrecondition(objAttrs, objQoSAttrs);

            if (m_pConfirmedStatus != IMS_NULL)
            {
                m_abAttributeContains[ATTR_QOS_CONF] = IMS_TRUE;

                for (IMS_UINT32 i = 0; i < objQoSAttrs.GetSize(); ++i)
                {
                    RemoveAttribute(objQoSAttrs.GetAt(i));
                }
            }
        }
    }
#endif

    // Remove "rtpmap" & "fmtp" attributes
    RemoveAttributes(SdpAttribute::RTPMAP);
    RemoveAttributes(SdpAttribute::FMTP);

    // Remove "rtcp-fb" attribute
    RemoveAttributes(SdpAttribute::RTCP_FB);
    // Remove "framesize" attribute
    // RemoveAttributes(SdpAttribute::FRAMESIZE);

    if (GetAttributes().IsEmpty())
    {
        m_abLineContains[Sdp::TYPE_A] = IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
const SdpMediaFormat* SdpMediaParameter::GetMediaFormat(
        IN IMS_SINT32 nType, IN const AString& strValue) const
{
    for (IMS_UINT32 i = 0; i < m_objMediaFormats.GetSize(); ++i)
    {
        const SdpMediaFormat* pFormat = m_objMediaFormats.GetAt(i);

        if (pFormat->GetType() != nType)
        {
            continue;
        }

        if (pFormat->GetValue().Equals(strValue))
        {
            return pFormat;
        }
    }

    return IMS_NULL;
}

PUBLIC
SdpMediaFormat* SdpMediaParameter::GetMediaFormat(IN IMS_SINT32 nType, IN const AString& strValue)
{
    for (IMS_UINT32 i = 0; i < m_objMediaFormats.GetSize(); ++i)
    {
        SdpMediaFormat* pFormat = m_objMediaFormats.GetAt(i);

        if (pFormat->GetType() != nType)
        {
            continue;
        }

        if (pFormat->GetValue().Equals(strValue))
        {
            return pFormat;
        }
    }

    return IMS_NULL;
}

PUBLIC
IMS_BOOL SdpMediaParameter::IsQosPreconditionPresent() const
{
    return ((m_abAttributeContains[ATTR_QOS_CONF]) || (m_abAttributeContains[ATTR_QOS_CURR]) ||
            (m_abAttributeContains[ATTR_QOS_DES]));
}

PUBLIC
void SdpMediaParameter::MarkRejectedOrRemoved()
{
    IMS_TRACE_I("SdpMediaParameter - Rejected / Removed (%s)", m_objMedia.Encode().GetStr(), 0, 0);

    m_objMedia.SetPort(0);
    m_objMedia.SetNumOfPort(0);
}

PUBLIC
void SdpMediaParameter::RemoveConnections()
{
    m_objConnections.Clear();
    // objPrevConnections.Clear();
}

PUBLIC
void SdpMediaParameter::RemoveMediaFormat(IN IMS_SINT32 nType, IN const AString& strValue)
{
    for (IMS_UINT32 i = 0; i < m_objMediaFormats.GetSize(); ++i)
    {
        const SdpMediaFormat* pFormat = m_objMediaFormats.GetAt(i);

        if (pFormat->GetType() != nType)
        {
            continue;
        }

        if (pFormat->GetValue().Equals(strValue))
        {
            // Remove the media format from the m-line (SdpMedia)
            AStringArray objFormats = m_objMedia.GetFormats();

            objFormats.RemoveElement(strValue, IMS_FALSE);

            m_objMedia.SetFormats(objFormats);

            // Delete the media format itself
            delete pFormat;

            m_objMediaFormats.RemoveAt(i);
            return;
        }
    }
}

PUBLIC
void SdpMediaParameter::SetAttributeMid(IN const AString& strAttrMid)
{
    m_strAttrMid = strAttrMid;

    if (m_strAttrMid.IsNULL())
    {
        m_abAttributeContains[ATTR_MID] = IMS_FALSE;
    }
    else
    {
        m_abAttributeContains[ATTR_MID] = IMS_TRUE;
    }
}

PUBLIC
IMS_BOOL SdpMediaParameter::SetConnectionAddress(IN const AString& strAddress)
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

    m_objConnections.Clear();

    if (!objIpAddr.Parse(strAddress))
    {
        return IMS_FALSE;
    }

    SdpConnection objConnection;

    if (objIpAddr.IsIPv6Address())
    {
        if (!objConnection.SetValue(Sdp::ADDR_TYPE_IP6, objIpAddr.ToString()))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        if (!objConnection.SetValue(Sdp::ADDR_TYPE_IP4, objIpAddr.ToString()))
        {
            return IMS_FALSE;
        }
    }

    m_objConnections.Append(objConnection);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SdpMediaParameter::SetMedia(IN IMS_SINT32 nType, IN IMS_SINT32 nPort,
        IN IMS_SINT32 nTransportProtocol, IN const AStringArray& objFormats)
{
    ClearMediaFormat(m_objMediaFormats);

    // If the media type is not an audio/video,
    // then set the media formats automatically using the format list.
    if ((nTransportProtocol == SdpMedia::TRANSPORT_RTP_AVP) ||
            (nTransportProtocol == SdpMedia::TRANSPORT_RTP_AVPF) ||
            (nTransportProtocol == SdpMedia::TRANSPORT_RTP_SAVP) ||
            (nTransportProtocol == SdpMedia::TRANSPORT_RTP_SAVPF))
    {
        for (IMS_SINT32 i = 0; i < objFormats.GetCount(); ++i)
        {
            SdpAvCodec* pCodec = new SdpAvCodec();

            if (pCodec == IMS_NULL)
            {
                return IMS_FALSE;
            }

            pCodec->SetValue(objFormats.GetElementAt(i));

            if (!m_objMediaFormats.Append(pCodec))
            {
                delete pCodec;
                return IMS_FALSE;
            }
        }
    }
    else
    {
        // Set the media format
        IMS_SINT32 nFormatType;

        if ((nTransportProtocol == SdpMedia::TRANSPORT_TCP_MSRP) ||
                (nTransportProtocol == SdpMedia::TRANSPORT_TCP_TLS_MSRP))
        {
            nFormatType = SdpMediaFormat::TYPE_MSRP;
        }
        else if (nTransportProtocol == SdpMedia::TRANSPORT_UDP)
        {
            nFormatType = SdpMediaFormat::TYPE_UDP;
        }
        else if (nTransportProtocol == SdpMedia::TRANSPORT_TCP)
        {
            nFormatType = SdpMediaFormat::TYPE_TCP;
        }
        else
        {
            nFormatType = SdpMediaFormat::TYPE_OTHER;
        }

        for (IMS_SINT32 i = 0; i < objFormats.GetCount(); ++i)
        {
            SdpMediaFormat* pFormat = new SdpMediaFormat(nFormatType);

            if (pFormat == IMS_NULL)
            {
                return IMS_FALSE;
            }

            pFormat->SetValue(objFormats.GetElementAt(i));

            if (!m_objMediaFormats.Append(pFormat))
            {
                delete pFormat;
                return IMS_FALSE;
            }
        }
    }

    if (!m_objMedia.SetValue(nType, nPort, nTransportProtocol, objFormats))
    {
        IMS_TRACE_E(0, "Setting SDP media failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
void SdpMediaParameter::SetMid(IN IMS_SINT32 nMid)
{
    if (m_nMid != nMid)
    {
        IMS_TRACE_I("SetMid :: %d >> %d", m_nMid, nMid, 0);
        m_nMid = nMid;
    }
}

PUBLIC
void SdpMediaParameter::UpdateProperties(IN const SdpMediaParameter& objMediaParam)
{
    if (m_objMedia.GetPort() != 0)
    {
        AStringArray objFormats;

        for (IMS_UINT32 i = 0; i < m_objMediaFormats.GetSize(); ++i)
        {
            const SdpMediaFormat* pMediaFormat = m_objMediaFormats.GetAt(i);
            objFormats.AddElement(pMediaFormat->GetValue());
        }

        m_objMedia.SetValue(objMediaParam.m_objMedia.GetType(), objMediaParam.m_objMedia.GetPort(),
                objMediaParam.m_objMedia.GetTransportProtocol(), objFormats,
                objMediaParam.m_objMedia.GetTypeEx());
    }

    // Set the packet time attribute
    // nPacketTime = objMediaStream.nPacketTime;

    // Set the connecton
    m_objConnections.Clear();

    if (objMediaParam.Contains(Sdp::TYPE_C))
    {
        m_objConnections = objMediaParam.m_objConnections;

        m_abLineContains[Sdp::TYPE_C] = IMS_TRUE;
    }

    // Set the previous connection
    m_objPrevConnections.Clear();
    m_objPrevConnections = objMediaParam.m_objPrevConnections;

    SdpParameter::UpdateProperties(objMediaParam);

    for (IMS_SINT32 i = 0; i < ATTR_MAX; ++i)
    {
        m_abAttributeContains[i] = objMediaParam.m_abAttributeContains[i];
    }

    // MID (a=mid:xxxx)
    if (m_abAttributeContains[ATTR_MID])
    {
        m_strAttrMid = objMediaParam.m_strAttrMid;
    }

#if defined(__IMS_SDP_PRECONDITION__)
    {
        // QoS attributes (a=curr:xxxx, a=des:xxxx, a=conf:xxxx)
        ClearPrecondition(this);
        CopyPrecondition(objMediaParam, *this);
    }
#endif

    // Remove "rtpmap" & "fmtp" attributes
}

PUBLIC
SdpPrecondition* SdpMediaParameter::GetPrecondition(
        IN IMS_SINT32 nAttribute, IN IMS_SINT32 nType /* = SdpPrecondition::TYPE_QOS */) const
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

PUBLIC
void SdpMediaParameter::RemovePrecondition(
        IN IMS_SINT32 nAttribute, IN IMS_SINT32 nType /* = SdpPrecondition::TYPE_QOS */)
{
    if (nType != SdpPrecondition::TYPE_QOS)
    {
        IMS_TRACE_E(
                0, "Type (%d) is not supported; It only supports 'qos' precondition", nType, 0, 0);
        return;
    }

    switch (nAttribute)
    {
        case SdpAttribute::CURR:
            if (m_pCurrentStatus != IMS_NULL)
            {
                delete m_pCurrentStatus;
                m_pCurrentStatus = IMS_NULL;
            }

            m_abAttributeContains[ATTR_QOS_CURR] = IMS_FALSE;
            break;

        case SdpAttribute::DES:
            if (m_pDesiredStatus != IMS_NULL)
            {
                delete m_pDesiredStatus;
                m_pDesiredStatus = IMS_NULL;
            }

            m_abAttributeContains[ATTR_QOS_DES] = IMS_FALSE;
            break;

        case SdpAttribute::CONF:
            if (m_pConfirmedStatus != IMS_NULL)
            {
                delete m_pConfirmedStatus;
                m_pConfirmedStatus = IMS_NULL;
            }

            m_abAttributeContains[ATTR_QOS_CONF] = IMS_FALSE;
            break;

        default:
            break;
    }
}

PUBLIC
IMS_BOOL SdpMediaParameter::SetPrecondition(
        IN IMS_SINT32 nAttribute, IN SdpPrecondition* pPrecondition)
{
    if ((nAttribute != SdpAttribute::CURR) && (nAttribute != SdpAttribute::DES) &&
            (nAttribute != SdpAttribute::CONF))
    {
        IMS_TRACE_D("Illegal argument :: attribute (%d)", nAttribute, 0, 0);
        return IMS_FALSE;
    }

    if (pPrecondition == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (pPrecondition->GetType() != SdpPrecondition::TYPE_QOS)
    {
        IMS_TRACE_D("Non-qos precondition (%d) is not allowed", pPrecondition->GetType(), 0, 0);
        return IMS_FALSE;
    }

    SdpPrecondition* pNewPrecondition = IMS_NULL;

    if (pPrecondition->GetSubType() == SdpPrecondition::SUBTYPE_E2E)
    {
        const SdpE2EPrecondition* pE2E = DYNAMIC_CAST(SdpE2EPrecondition*, pPrecondition);

        if (pE2E == IMS_NULL)
        {
            return IMS_FALSE;
        }

        pNewPrecondition = new SdpE2EPrecondition(*pE2E);
    }
    else if (pPrecondition->GetSubType() == SdpPrecondition::SUBTYPE_SEGMENTED)
    {
        const SdpSegmentedPrecondition* pSegmented =
                DYNAMIC_CAST(SdpSegmentedPrecondition*, pPrecondition);

        if (pSegmented == IMS_NULL)
        {
            return IMS_FALSE;
        }

        pNewPrecondition = new SdpSegmentedPrecondition(*pSegmented);
    }
    else
    {
        IMS_TRACE_E(0, "Invalid subtype (%d)", pPrecondition->GetSubType(), 0, 0);
        return IMS_FALSE;
    }

    if (pNewPrecondition == IMS_NULL)
    {
        return IMS_FALSE;
    }

    switch (nAttribute)
    {
        case SdpAttribute::CURR:
            if (m_pCurrentStatus != IMS_NULL)
            {
                delete m_pCurrentStatus;
            }

            m_pCurrentStatus = pNewPrecondition;

            m_abAttributeContains[ATTR_QOS_CURR] = IMS_TRUE;
            break;

        case SdpAttribute::DES:
            if (m_pDesiredStatus != IMS_NULL)
            {
                delete m_pDesiredStatus;
            }

            m_pDesiredStatus = pNewPrecondition;

            m_abAttributeContains[ATTR_QOS_DES] = IMS_TRUE;
            break;

        case SdpAttribute::CONF:
            if (m_pConfirmedStatus != IMS_NULL)
            {
                delete m_pConfirmedStatus;
            }

            m_pConfirmedStatus = pNewPrecondition;

            m_abAttributeContains[ATTR_QOS_CONF] = IMS_TRUE;
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL SdpMediaParameter::ExtractMediaFormat(
        IN const SdpMediaDescription& objMediaDesc, OUT ImsList<SdpMediaFormat*>& objOutFormats)
{
    const SdpMedia& objMedia = objMediaDesc.GetMedia();
    const AStringArray& objFormats = objMedia.GetFormats();
    IMS_SINT32 nTransportProtocol = objMedia.GetTransportProtocol();

    if ((nTransportProtocol == SdpMedia::TRANSPORT_RTP_AVP) ||
            (nTransportProtocol == SdpMedia::TRANSPORT_RTP_AVPF) ||
            (nTransportProtocol == SdpMedia::TRANSPORT_RTP_SAVP) ||
            (nTransportProtocol == SdpMedia::TRANSPORT_RTP_SAVPF))
    {
        ImsList<SdpAttribute> objRtpmaps = objMediaDesc.GetAttributes(SdpAttribute::RTPMAP);
        ImsList<SdpAttribute> objFmtps = objMediaDesc.GetAttributes(SdpAttribute::FMTP);

        // Additional payload specific parameters - framesize / rtcp-fb / ...
        ImsList<SdpAttribute> objRtcpFbs = objMediaDesc.GetAttributes(SdpAttribute::RTCP_FB);

        // Convert the payload type to an integer value
        ImsList<IMS_SINT32> objPts;
        ImsList<IMS_SINT32> objPts4Rtpmap;
        ImsList<IMS_SINT32> objPts4Fmtp;
        // rtcp-fb attribute
        ImsList<IMS_SINT32> objPts4RtcpFb;

        for (IMS_SINT32 i = 0; i < objFormats.GetCount(); ++i)
        {
            objPts.Append(objFormats.GetElementAt(i).ToInt32());
        }

        IMS_UINT32 nRtpmap;
        IMS_UINT32 nFmtp;
        // rtcp-fb attribute
        ImsList<IMS_UINT32> objRtcpFbIndices;
        ImsList<IMS_UINT32> objWildcardRtcpFbIndices;

        for (nRtpmap = 0; nRtpmap < objRtpmaps.GetSize(); ++nRtpmap)
        {
            const SdpAttribute& objAttribute = objRtpmaps.GetAt(nRtpmap);

            objPts4Rtpmap.Append(
                    Sdp::GetPayloadTypeFromAttribute(objAttribute.GetAttributeValue()));
        }

        for (nFmtp = 0; nFmtp < objFmtps.GetSize(); ++nFmtp)
        {
            const SdpAttribute& objAttribute = objFmtps.GetAt(nFmtp);

            objPts4Fmtp.Append(Sdp::GetPayloadTypeFromAttribute(objAttribute.GetAttributeValue()));
        }

        // rtcp-fb attribute
        for (IMS_UINT32 nRtcpFb = 0; nRtcpFb < objRtcpFbs.GetSize(); ++nRtcpFb)
        {
            const SdpAttribute& objAttribute = objRtcpFbs.GetAt(nRtcpFb);

            if (objAttribute.GetAttributeValue().StartsWith(TextParser::CHAR_ASTERISK))
            {
                objWildcardRtcpFbIndices.Append(nRtcpFb);
                objPts4RtcpFb.Append(SdpMediaFormatParameter::PT_WILDCARD);
            }
            else
            {
                objPts4RtcpFb.Append(
                        Sdp::GetPayloadTypeFromAttribute(objAttribute.GetAttributeValue()));
            }
        }

#if defined(__SDP_CORRECT_FMTP_FOR_DUPLICATE_PAYLOAD_TYPES__)
        // 4 workaround solution for the multiple fmtp attributes -
        //  if it is required for test purpose, please enable below line.
        CorrectFmtps(objPts4Fmtp, objFmtps);
#endif

        // Check the payload type and collect the attribute according to the payload type
        for (IMS_UINT32 j = 0; j < objPts.GetSize(); ++j)
        {
            IMS_SINT32 nPayloadType = objPts.GetAt(j);

            for (nRtpmap = 0; nRtpmap < objPts4Rtpmap.GetSize(); ++nRtpmap)
            {
                if (nPayloadType == objPts4Rtpmap.GetAt(nRtpmap))
                {
                    break;
                }
            }

            for (nFmtp = 0; nFmtp < objPts4Fmtp.GetSize(); ++nFmtp)
            {
                if (nPayloadType == objPts4Fmtp.GetAt(nFmtp))
                {
                    break;
                }
            }

            // rtcp-fb attribute
            objRtcpFbIndices.Clear();

            for (IMS_UINT32 nRtcpFb = 0; nRtcpFb < objPts4RtcpFb.GetSize(); ++nRtcpFb)
            {
                if (nPayloadType == objPts4RtcpFb.GetAt(nRtcpFb))
                {
                    objRtcpFbIndices.Append(nRtcpFb);
                }
            }

            if (nRtpmap == objPts4Rtpmap.GetSize())
            {
                if (SdpAvCodec::IsDynamicPayloadType(nPayloadType))
                {
                    continue;
                }

                AString strRtpmap;

                if (!SdpAvCodec::GetDefaultRtpmap(nPayloadType, strRtpmap))
                {
                    continue;
                }

                SdpAvCodec* pCodec = new SdpAvCodec();

                if (pCodec == IMS_NULL)
                {
                    return IMS_FALSE;
                }

                if (nFmtp != objPts4Fmtp.GetSize())
                {
                    const SdpAttribute& objFmtp = objFmtps.GetAt(nFmtp);

                    if (!pCodec->SetParameters(strRtpmap, objFmtp.GetAttributeValue()))
                    {
                        delete pCodec;

                        IMS_TRACE_E(0, "Setting parameters (%s, %s) failed", strRtpmap.GetStr(),
                                objFmtp.GetAttributeValue().GetStr(), 0);
                        return IMS_FALSE;
                    }
                }
                else
                {
                    if (!pCodec->SetParameters(strRtpmap, AString::ConstNull()))
                    {
                        delete pCodec;

                        IMS_TRACE_E(0, "Setting parameters (%s) failed", strRtpmap.GetStr(), 0, 0);
                        return IMS_FALSE;
                    }
                }

                // rtcp-fb attribute
                if (!objRtcpFbIndices.IsEmpty())
                {
                    for (IMS_UINT32 kk = 0; kk < objRtcpFbIndices.GetSize(); ++kk)
                    {
                        const SdpAttribute& objRtcpFb =
                                objRtcpFbs.GetAt(objRtcpFbIndices.GetAt(kk));
                        pCodec->AddExtraParameter(
                                SdpRtcpFeedback::Decode(objRtcpFb.GetAttributeValue()));
                    }
                }

                if (!objWildcardRtcpFbIndices.IsEmpty())
                {
                    for (IMS_UINT32 kk = 0; kk < objWildcardRtcpFbIndices.GetSize(); ++kk)
                    {
                        const SdpAttribute& objRtcpFb =
                                objRtcpFbs.GetAt(objWildcardRtcpFbIndices.GetAt(kk));
                        pCodec->AddExtraParameter(
                                SdpRtcpFeedback::Decode(objRtcpFb.GetAttributeValue()));
                    }
                }

                if (!objOutFormats.Append(pCodec))
                {
                    delete pCodec;
                    return IMS_FALSE;
                }

                continue;
            }

            SdpAvCodec* pCodec = new SdpAvCodec();

            if (pCodec == IMS_NULL)
            {
                return IMS_FALSE;
            }

            const SdpAttribute& objRtpmap = objRtpmaps.GetAt(nRtpmap);

            if (nFmtp != objPts4Fmtp.GetSize())
            {
                const SdpAttribute& objFmtp = objFmtps.GetAt(nFmtp);

                if (!pCodec->SetParameters(
                            objRtpmap.GetAttributeValue(), objFmtp.GetAttributeValue()))
                {
                    delete pCodec;

                    IMS_TRACE_E(0, "Setting parameters (%s, %s) failed",
                            objRtpmap.GetAttributeValue().GetStr(),
                            objFmtp.GetAttributeValue().GetStr(), 0);
                    return IMS_FALSE;
                }
            }
            else
            {
                if (!pCodec->SetParameters(objRtpmap.GetAttributeValue(), AString::ConstNull()))
                {
                    delete pCodec;

                    IMS_TRACE_E(0, "Setting parameters (%s) failed",
                            objRtpmap.GetAttributeValue().GetStr(), 0, 0);
                    return IMS_FALSE;
                }
            }

            // rtcp-fb attribute
            if (!objRtcpFbIndices.IsEmpty())
            {
                for (IMS_UINT32 kk = 0; kk < objRtcpFbIndices.GetSize(); ++kk)
                {
                    const SdpAttribute& objRtcpFb = objRtcpFbs.GetAt(objRtcpFbIndices.GetAt(kk));
                    pCodec->AddExtraParameter(
                            SdpRtcpFeedback::Decode(objRtcpFb.GetAttributeValue()));
                }
            }

            if (!objWildcardRtcpFbIndices.IsEmpty())
            {
                for (IMS_UINT32 kk = 0; kk < objWildcardRtcpFbIndices.GetSize(); ++kk)
                {
                    const SdpAttribute& objRtcpFb =
                            objRtcpFbs.GetAt(objWildcardRtcpFbIndices.GetAt(kk));
                    pCodec->AddExtraParameter(
                            SdpRtcpFeedback::Decode(objRtcpFb.GetAttributeValue()));
                }
            }

            if (!objOutFormats.Append(pCodec))
            {
                delete pCodec;
                return IMS_FALSE;
            }
        }
    }
    else
    {
        IMS_SINT32 nFormatType;

        if ((nTransportProtocol == SdpMedia::TRANSPORT_TCP_MSRP) ||
                (nTransportProtocol == SdpMedia::TRANSPORT_TCP_TLS_MSRP))
        {
            nFormatType = SdpMediaFormat::TYPE_MSRP;
        }
        else if (nTransportProtocol == SdpMedia::TRANSPORT_UDP)
        {
            nFormatType = SdpMediaFormat::TYPE_UDP;
        }
        else if (nTransportProtocol == SdpMedia::TRANSPORT_TCP)
        {
            nFormatType = SdpMediaFormat::TYPE_TCP;
        }
        else
        {
            nFormatType = SdpMediaFormat::TYPE_OTHER;
        }

        for (IMS_SINT32 i = 0; i < objFormats.GetCount(); ++i)
        {
            SdpMediaFormat* pFormat = new SdpMediaFormat(nFormatType);

            if (pFormat == IMS_NULL)
            {
                return IMS_FALSE;
            }

            pFormat->SetValue(objFormats.GetElementAt(i));

            if (!objOutFormats.Append(pFormat))
            {
                delete pFormat;
                return IMS_FALSE;
            }
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL SdpPrecondition* SdpMediaParameter::CreatePrecondition(
        IN const ImsList<SdpAttribute>& objAttributes, OUT ImsList<SdpAttribute>& objQosAttrs)
{
    if (objAttributes.IsEmpty())
    {
        return IMS_NULL;
    }

    SdpPrecondition* pPrecondition = IMS_NULL;
    IMS_SINT32 nType = SdpPrecondition::TYPE_INVALID;
    IMS_SINT32 nSubType = SdpPrecondition::STATUS_E2E;
    SdpPrecondition::DetailInfo objInfo;

    for (IMS_UINT32 i = 0; i < objAttributes.GetSize(); ++i)
    {
        const SdpAttribute& objAttr = objAttributes.GetAt(i);

        IMS_TRACE_D("'qos' attribute :: %s", objAttr.GetAttributeValue().GetStr(), 0, 0);

        if (!SdpPrecondition::ExtractProperties(
                    objAttr.GetAttributeValue(), nType, nSubType, objInfo))
        {
            continue;
        }

        if (nType != SdpPrecondition::TYPE_QOS)
        {
            continue;
        }

        if ((nSubType != SdpPrecondition::SUBTYPE_E2E) &&
                (nSubType != SdpPrecondition::SUBTYPE_SEGMENTED))
        {
            continue;
        }

        if (pPrecondition == IMS_NULL)
        {
            if (nSubType == SdpPrecondition::SUBTYPE_E2E)
            {
                pPrecondition = new SdpE2EPrecondition();
            }
            else
            {
                pPrecondition = new SdpSegmentedPrecondition();
            }

            if (pPrecondition == IMS_NULL)
            {
                continue;
            }
        }

        if (!pPrecondition->AddStatus(
                    objInfo.GetStatus(), objInfo.GetDirection(), objInfo.GetStrength()))
        {
            IMS_TRACE_E(0, "Adding status (%s) failed", objAttr.GetAttributeValue().GetStr(), 0, 0);
            continue;
        }

        objQosAttrs.Append(objAttr);
    }

    if (pPrecondition == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (!pPrecondition->IsPreconditionPresent())
    {
        delete pPrecondition;
        return IMS_NULL;
    }

    return pPrecondition;
}

PRIVATE VIRTUAL void SdpMediaParameter::Clear()
{
    SdpParameter::Clear();

    m_objConnections.Clear();
    m_objPrevConnections.Clear();

    ClearMediaFormat(m_objMediaFormats);

#if defined(__IMS_SDP_PRECONDITION__)
    {
        ClearPrecondition(this);
    }
#endif

    for (IMS_SINT32 i = 0; i < ATTR_MAX; ++i)
    {
        m_abAttributeContains[i] = IMS_FALSE;
    }
}

PRIVATE VIRTUAL IMS_BOOL SdpMediaParameter::IsDirectionAttributeRequired() const
{
    if (SdpProfile::GetInstance()->IsAttributeDirectionRequiredForRemovedMedia())
    {
        return SdpParameter::IsDirectionAttributeRequired();
    }

    if (m_objMedia.GetPort() == 0)
    {
        IMS_TRACE_D(
                "Direction attribute will not be formed; m-line is removed or rejected", 0, 0, 0);
        return IMS_FALSE;
    }

    return SdpParameter::IsDirectionAttributeRequired();
}

PRIVATE GLOBAL void SdpMediaParameter::ClearMediaFormat(
        IN_OUT ImsList<SdpMediaFormat*>& objMediaFormats)
{
    for (IMS_UINT32 i = 0; i < objMediaFormats.GetSize(); ++i)
    {
        SdpMediaFormat* pMediaFormat = objMediaFormats.GetAt(i);

        delete pMediaFormat;
    }

    objMediaFormats.Clear();
}

PRIVATE GLOBAL IMS_BOOL SdpMediaParameter::CopyMediaFormat(
        IN const ImsList<SdpMediaFormat*>& objInFormats,
        OUT ImsList<SdpMediaFormat*>& objOutFormats)
{
    if (objInFormats.IsEmpty())
    {
        return IMS_TRUE;
    }

    for (IMS_UINT32 i = 0; i < objInFormats.GetSize(); ++i)
    {
        const SdpMediaFormat* pFormat = objInFormats.GetAt(i);

        if (pFormat == IMS_NULL)
        {
            continue;
        }

        SdpMediaFormat* pNewFormat = IMS_NULL;

        if (pFormat->GetType() == SdpMediaFormat::TYPE_RTP)
        {
            const SdpAvCodec* pCodec = DYNAMIC_CAST(const SdpAvCodec*, pFormat);

            if (pCodec == IMS_NULL)
            {
                continue;
            }

            pNewFormat = new SdpAvCodec(*pCodec);
        }
        else
        {
            pNewFormat = new SdpMediaFormat(*pFormat);
        }

        if (pNewFormat == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (!objOutFormats.Append(pNewFormat))
        {
            delete pNewFormat;
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE GLOBAL IMS_BOOL SdpMediaParameter::IsSameAvCodecPresent(
        IN const ImsList<SdpMediaFormat*>& objFormats, IN const SdpMediaFormat* pMediaFormat)
{
    if (pMediaFormat == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (pMediaFormat->GetType() != SdpMediaFormat::TYPE_RTP)
    {
        return IMS_FALSE;
    }

    const SdpAvCodec* pCodec = DYNAMIC_CAST(const SdpAvCodec*, pMediaFormat);

    if (pCodec == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objFormats.GetSize(); ++i)
    {
        const SdpMediaFormat* pTmpMediaFormat = objFormats.GetAt(i);

        if (pTmpMediaFormat == IMS_NULL)
        {
            continue;
        }

        if (pTmpMediaFormat->GetType() != SdpMediaFormat::TYPE_RTP)
        {
            continue;
        }

        const SdpAvCodec* pTmpCodec = DYNAMIC_CAST(const SdpAvCodec*, pTmpMediaFormat);

        if (pTmpCodec == IMS_NULL)
        {
            continue;
        }

        // If the payload type equals,
        // it means that the format list already contains the specified codec.
        if (pCodec->GetPayloadType() == pTmpCodec->GetPayloadType())
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL IMS_BOOL SdpMediaParameter::IsSameNonAvCodecPresent(
        IN const ImsList<SdpMediaFormat*>& objFormats, IN const SdpMediaFormat* pMediaFormat)
{
    if (pMediaFormat == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (pMediaFormat->GetType() == SdpMediaFormat::TYPE_RTP)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objFormats.GetSize(); ++i)
    {
        const SdpMediaFormat* pTmpMediaFormat = objFormats.GetAt(i);

        if (pTmpMediaFormat == IMS_NULL)
        {
            continue;
        }

        if (pTmpMediaFormat->GetType() == SdpMediaFormat::TYPE_RTP)
        {
            continue;
        }

        if (pTmpMediaFormat->Equals(pMediaFormat))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL void SdpMediaParameter::ClearPrecondition(IN_OUT SdpMediaParameter* pMediaParam)
{
    if (pMediaParam == IMS_NULL)
    {
        return;
    }

    if (pMediaParam->m_pCurrentStatus != IMS_NULL)
    {
        delete (pMediaParam->m_pCurrentStatus);
        pMediaParam->m_pCurrentStatus = IMS_NULL;
    }

    if (pMediaParam->m_pDesiredStatus != IMS_NULL)
    {
        delete (pMediaParam->m_pDesiredStatus);
        pMediaParam->m_pDesiredStatus = IMS_NULL;
    }

    if (pMediaParam->m_pConfirmedStatus != IMS_NULL)
    {
        delete (pMediaParam->m_pConfirmedStatus);
        pMediaParam->m_pConfirmedStatus = IMS_NULL;
    }
}

PRIVATE GLOBAL IMS_BOOL SdpMediaParameter::CopyPrecondition(
        IN const SdpMediaParameter& objMediaParam, IN_OUT SdpMediaParameter& objOutMediaParam)
{
    if ((objMediaParam.m_pCurrentStatus == IMS_NULL) &&
            (objMediaParam.m_pDesiredStatus == IMS_NULL) &&
            (objMediaParam.m_pConfirmedStatus == IMS_NULL))
    {
        return IMS_TRUE;
    }

    const SdpE2EPrecondition* pE2E;
    const SdpSegmentedPrecondition* pSegmented;

    if (objMediaParam.m_pCurrentStatus != IMS_NULL)
    {
        if (objMediaParam.m_pCurrentStatus->GetSubType() == SdpPrecondition::SUBTYPE_E2E)
        {
            pE2E = DYNAMIC_CAST(SdpE2EPrecondition*, objMediaParam.m_pCurrentStatus);

            if (pE2E != IMS_NULL)
            {
                objOutMediaParam.m_pCurrentStatus = new SdpE2EPrecondition(*pE2E);
            }
        }
        else
        {
            pSegmented = DYNAMIC_CAST(SdpSegmentedPrecondition*, objMediaParam.m_pCurrentStatus);

            if (pSegmented != IMS_NULL)
            {
                objOutMediaParam.m_pCurrentStatus = new SdpSegmentedPrecondition(*pSegmented);
            }
        }
    }

    if (objMediaParam.m_pDesiredStatus != IMS_NULL)
    {
        if (objMediaParam.m_pDesiredStatus->GetSubType() == SdpPrecondition::SUBTYPE_E2E)
        {
            pE2E = DYNAMIC_CAST(SdpE2EPrecondition*, objMediaParam.m_pDesiredStatus);

            if (pE2E != IMS_NULL)
            {
                objOutMediaParam.m_pDesiredStatus = new SdpE2EPrecondition(*pE2E);
            }
        }
        else
        {
            pSegmented = DYNAMIC_CAST(SdpSegmentedPrecondition*, objMediaParam.m_pDesiredStatus);

            if (pSegmented != IMS_NULL)
            {
                objOutMediaParam.m_pDesiredStatus = new SdpSegmentedPrecondition(*pSegmented);
            }
        }
    }

    if (objMediaParam.m_pConfirmedStatus != IMS_NULL)
    {
        if (objMediaParam.m_pConfirmedStatus->GetSubType() == SdpPrecondition::SUBTYPE_E2E)
        {
            pE2E = DYNAMIC_CAST(SdpE2EPrecondition*, objMediaParam.m_pConfirmedStatus);

            if (pE2E != IMS_NULL)
            {
                objOutMediaParam.m_pConfirmedStatus = new SdpE2EPrecondition(*pE2E);
            }
        }
        else
        {
            pSegmented = DYNAMIC_CAST(SdpSegmentedPrecondition*, objMediaParam.m_pConfirmedStatus);

            if (pSegmented != IMS_NULL)
            {
                objOutMediaParam.m_pConfirmedStatus = new SdpSegmentedPrecondition(*pSegmented);
            }
        }
    }

    return IMS_TRUE;
}

#if defined(__SDP_CORRECT_FMTP_FOR_DUPLICATE_PAYLOAD_TYPES__)
PUBLIC GLOBAL void SdpMediaParameter::CorrectFmtps(
        IN ImsList<IMS_SINT32>& objPayloadTypes4Fmtp, IN_OUT ImsList<SdpAttribute>& objFmtps)
{
    ImsMap<IMS_SINT32, ImsList<IMS_SINT32>> objDuplicatedFmtps;
    IMS_BOOL bHasDuplicatedFmtp = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < objPayloadTypes4Fmtp.GetSize(); ++i)
    {
        IMS_SINT32 nPayloadType = objPayloadTypes4Fmtp.GetAt(i);
        IMS_SLONG nIndex = objDuplicatedFmtps.GetIndexOfKey(nPayloadType);

        if (nIndex < 0)
        {
            ImsList<IMS_SINT32> objIndices;
            objIndices.Append(i);

            objDuplicatedFmtps.SetValue(nPayloadType, objIndices);
        }
        else
        {
            ImsList<IMS_SINT32>& objIndices = objDuplicatedFmtps.GetValueAt(nIndex);
            objIndices.Append(i);

            bHasDuplicatedFmtp = IMS_TRUE;
        }
    }

    if (!bHasDuplicatedFmtp)
    {
        return;
    }

    IMS_TRACE_D("SdpMediaParameter :: correct the duplicated fmtp attributes ...", 0, 0, 0);

    for (IMS_UINT32 i = 0; i < objDuplicatedFmtps.GetSize(); ++i)
    {
        const ImsList<IMS_SINT32>& objIndices = objDuplicatedFmtps.GetValueAt(i);

        if (objIndices.GetSize() <= 1)
        {
            continue;
        }

        SdpAttribute& objAnchorAttr = objFmtps.GetAt(objIndices.GetAt(0));
        AString strOtherFmtp = objAnchorAttr.GetAttributeValue();
        AString strParameters;
        IMS_SINT32 nPayloadType;

        for (IMS_UINT32 j = 1; j < objIndices.GetSize(); ++j)
        {
            const SdpAttribute& objAttr = objFmtps.GetAt(objIndices.GetAt(j));

            if (Sdp::ParseAttributeFmtp(objAttr.GetAttributeValue(), nPayloadType, strParameters))
            {
                strOtherFmtp.Append("; ");
                strOtherFmtp.Append(strParameters);
            }
        }

        objAnchorAttr.SetValue(SdpAttribute::FMTP, strOtherFmtp);
    }
}
#endif
