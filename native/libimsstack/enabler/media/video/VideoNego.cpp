/**
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

#include "ISessionDescriptor.h"
#include "ServiceTrace.h"
#include "offeranswer/SdpAvCodec.h"
#include "offeranswer/SdpMediaFormatParameter.h"
#include "offeranswer/SdpRtcpFeedback.h"

#include "MediaNegoUtil.h"
#include "MediaProfileFactory.h"
#include "MediaProfileUtil.h"
#include "config/MediaSessionConfig.h"
#include "config/MediaSessionConfigFactory.h"
#include "video/VideoNego.h"
#include "video/VideoNegoAvc.h"
#include "video/VideoNegoHevc.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC VideoNego::VideoNego(IN const IMS_SINT32 nSlotId) :
        BaseNego(nSlotId, MEDIA_TYPE_VIDEO),
        m_pProfileExtractor(std::make_unique<VideoProfileExtractor>())
{
    IMS_TRACE_I("+VideoNego() - slot[%d]", nSlotId, 0, 0);
}

PUBLIC
VideoNego::VideoNego(IN const VideoNego& obj) :
        BaseNego(obj.GetSlotId())
{
    Copy(&obj);
}

PUBLIC
VideoNego& VideoNego::operator=(IN const VideoNego& obj)
{
    if (this != &obj)
    {
        Copy(&obj);
    }
    return (*this);
}

PUBLIC VideoNego::~VideoNego()
{
    IMS_TRACE_I("~VideoNego()", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL VideoNego::IsMediaCodecFromSdpSupported(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    // Handling exception case
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return MEDIA_TYPE_INVALID;
    }

    IMS_TRACE_I("IsMediaCodecFromSdpSupported()", 0, 0, 0);

    OaModel objOaModel;
    objOaModel.pLocalProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_VIDEO, m_pBaseProfile);

    // Make a destination profile from SDP
    objOaModel.pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_VIDEO);

    if (m_pProfileExtractor->Extract(
                pSessionDescriptor, pDescriptor, GetPeerProfile(&objOaModel)) != IMS_TRUE)
    {
        return MEDIA_TYPE_INVALID;
    }

    // Make a negotiated profile from the local and peer profile
    objOaModel.pNegotiatedProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_VIDEO);

    if (MakeNegotiatedProfile(GetLocalProfile(&objOaModel), GetPeerProfile(&objOaModel), IMS_TRUE,
                GetNegotiatedProfile(&objOaModel)) != IMS_TRUE)
    {
        return MEDIA_TYPE_INVALID;
    }

    return (objOaModel.pNegotiatedProfile->GetPayloadList().GetSize() > 0 &&
                   objOaModel.pNegotiatedProfile->GetDataPort() != 0)
            ? IMS_TRUE
            : IMS_FALSE;
}

PUBLIC
VIDEO_RESOLUTION VideoNego::GetNegotiatedResolution()
{
    MediaBaseProfile::BasePayload* pPayload = GetNegotiatedPayload();

    if (pPayload == IMS_NULL)
    {
        return VIDEO_RESOLUTION_INVALID;
    }

    if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"))
    {
        VideoProfile::AvcFmtp* pFmtp = (VideoProfile::AvcFmtp*)pPayload->GetFmtp();
        if (pFmtp != IMS_NULL)
        {
            return pFmtp->GetResolution();
        }
    }
    else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H265"))
    {
        VideoProfile::HevcFmtp* pFmtp = (VideoProfile::HevcFmtp*)pPayload->GetFmtp();
        if (pFmtp != IMS_NULL)
        {
            return pFmtp->GetResolution();
        }
    }

    return VIDEO_RESOLUTION_NOT_USED;
}

PUBLIC VideoConfiguration* VideoNego::ConfigCasting(IN MediaConfiguration* pConfig)
{
    return (pConfig != IMS_NULL) ? static_cast<VideoConfiguration*>(pConfig) : IMS_NULL;
}

PUBLIC VideoProfile* VideoNego::ProfileCasting(IN MediaBaseProfile* pProfile)
{
    return (pProfile != IMS_NULL) ? static_cast<VideoProfile*>(pProfile) : IMS_NULL;
}

PUBLIC VideoProfile::Payload* VideoNego::PayloadCasting(IN MediaBaseProfile::BasePayload* pPayload)
{
    return (pPayload != IMS_NULL) ? static_cast<VideoProfile::Payload*>(pPayload) : IMS_NULL;
}

PROTECTED VideoProfile* VideoNego::GetLocalProfile(IN OaModel* pOaModel)
{
    return ProfileCasting(BaseNego::GetLocalProfile(pOaModel));
}

PROTECTED VideoProfile* VideoNego::GetPeerProfile(IN OaModel* pOaModel)
{
    return ProfileCasting(BaseNego::GetPeerProfile(pOaModel));
}

PROTECTED VideoProfile* VideoNego::GetNegotiatedProfile(IN OaModel* pOaModel)
{
    return ProfileCasting(BaseNego::GetNegotiatedProfile(pOaModel));
}

PROTECTED IMS_BOOL VideoNego::FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable)
{
    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || m_listOaModel.GetSize() == 0)
    {
        return IMS_FALSE;
    }

    if (eDirection == MEDIA_DIRECTION_INVALID && bDisable != IMS_TRUE)
    {
        IMS_TRACE_E(0, "FormAnswer() - direction invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    // Getting OaModel from list
    OaModel* pNewOaModel = GetNegotiatedOaModel();

    if (pNewOaModel == IMS_NULL || pNewOaModel->IsAllProfileExist() == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("FormAnswer() - eDirection[%d] - bDisable[%d]", eDirection, bDisable, 0);

    // Modify a RTP/RTCP port to ZERO if video is not supported
    if (bDisable == IMS_TRUE)
    {
        pNewOaModel->pNegotiatedProfile->SetDataPort(0);
        pNewOaModel->pNegotiatedProfile->SetControlPort(0);
    }

    // Modify a direction by Enabler
    if (IS_VALID_MEDIA_DIRECTION(eDirection))
    {
        IMS_TRACE_D("FormAnswer() Enforced Set to direction[%d]", eDirection, 0, 0);
        pNewOaModel->pNegotiatedProfile->SetDirection(eDirection);
    }
    else
    {
        pNewOaModel->pNegotiatedProfile->SetDirection(
                UpdateDirectionToMine(pNewOaModel->pPeerProfile->GetDirection(),
                        pNewOaModel->pLocalProfile->GetDirection(), IMS_FALSE));

        IMS_TRACE_D("FormAnswer() Enforced Set to direction[%d] made from peer direction[%d]",
                pNewOaModel->pNegotiatedProfile->GetDirection(),
                pNewOaModel->pPeerProfile->GetDirection(), 0);
    }

    // Make the SDP from profile
    return MakeSdpFromProfile(pSessionDescriptor, pDescriptor, GetNegotiatedProfile(pNewOaModel));
}

PROTECTED IMS_BOOL VideoNego::FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable,
        IN IMS_BOOL bEnforceReofferMode)
{
    IMS_TRACE_I("FormReoffer() - pDescriptor[%" PFLS_x "], eDirection[%d], OaModel Size[%d]",
            pDescriptor, eDirection, m_listOaModel.GetSize());
    IMS_TRACE_I("FormReoffer() - bDisable[%d] EnforceReofferMode[%d]", bDisable,
            bEnforceReofferMode, 0);

    // Handling exception case
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (eDirection == MEDIA_DIRECTION_INVALID && bDisable != IMS_TRUE)
    {
        IMS_TRACE_E(0, "FormReoffer() - direction invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    pSessionDescriptor->SetDirection(MEDIA_DIRECTION_INVALID);

    // Make new Offer/Answer model, and copy source profile from previous negotiated profile
    OaModel* pNewOaModel = new OaModel();

    if (pNewOaModel == IMS_NULL)
    {
        return IMS_FALSE;
    }

    MediaSessionConfig* pMediaSessionConfig =
            MediaSessionConfigFactory::GetInstance()->FindMediaSessionConfig(
                    GetSlotId(), m_pEnvironment->eServiceType);

    if (m_listOaModel.GetSize() == 0)
    {
        pNewOaModel->pLocalProfile =
                MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_VIDEO, m_pBaseProfile);
    }
    else
    {
        OaModel* pPrevOaModel = GetNegotiatedOaModel();

        if (pPrevOaModel == IMS_NULL)
        {
            delete pNewOaModel;
            return IMS_FALSE;
        }

        if (pPrevOaModel->pNegotiatedProfile != IMS_NULL &&
                pPrevOaModel->pNegotiatedProfile->GetDataPort() == 0 && bDisable == IMS_TRUE)
        {
            if (pMediaSessionConfig->IsSdpReofferFullCapability() == IMS_TRUE)
            {
                IMS_TRACE_D("VideoNego::FormReOffer() - Try to Full Capability", 0, 0, 0);
                pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                        MEDIA_TYPE_VIDEO, m_pBaseProfile);
            }
            else
            {
                IMS_TRACE_D("VideoNego::FormReOffer() - Try to Negotiated Capability", 0, 0, 0);
                pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                        MEDIA_TYPE_VIDEO, GetNegotiatedProfile(pPrevOaModel));
            }
        }
        else
        {
            // pNewOaModel->pLocalProfile =
            // MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_VIDEO, m_pBaseProfile);
            // //org
            if (bEnforceReofferMode == IMS_TRUE)
            {
                pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                        MEDIA_TYPE_VIDEO, m_pBaseProfile);
            }
            else
            {
                if (pMediaSessionConfig->IsSdpReofferFullCapability() == IMS_TRUE)
                {
                    IMS_TRACE_D("VideoNego::FormReOffer() - Try to Full Capability", 0, 0, 0);
                    pNewOaModel->pLocalProfile = MediaProfileFactory::GetInstance()->CreateProfile(
                            MEDIA_TYPE_VIDEO, m_pBaseProfile);
                }
                else
                {
                    if (pPrevOaModel->pNegotiatedProfile != IMS_NULL)
                    {
                        IMS_TRACE_D(
                                "VideoNego::FormReOffer() - Try to Negotiated Capability", 0, 0, 0);
                        pNewOaModel->pLocalProfile =
                                MediaProfileFactory::GetInstance()->CreateProfile(
                                        MEDIA_TYPE_VIDEO, GetNegotiatedProfile(pPrevOaModel));
                    }
                }
            }
        }
    }

    if (pNewOaModel->pLocalProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "VideoNego::Create Local Profile failed", 0, 0, 0);
        delete pNewOaModel;
        return IMS_FALSE;
    }

    // Modify a RTP/RTCP port if video is not supported
    if (bDisable == IMS_TRUE)
    {
        pNewOaModel->pLocalProfile->SetDataPort(0);
        pNewOaModel->pLocalProfile->SetControlPort(0);
    }
    else
    {
        pNewOaModel->pLocalProfile->SetDataPort(m_pBaseProfile->GetDataPort());
        pNewOaModel->pLocalProfile->SetControlPort(m_pBaseProfile->GetControlPort());
    }

    // Modify a direction by Enabler
    if (IS_VALID_MEDIA_DIRECTION(eDirection))
    {
        IMS_TRACE_I("FormReoffer() Enforced Set to direction[%d]", eDirection, 0, 0);
        pNewOaModel->pLocalProfile->SetDirection(eDirection);
    }
    else
    {
        pNewOaModel->pLocalProfile->SetDataPort(0);
        pNewOaModel->pLocalProfile->SetControlPort(0);
    }

    // Modify a RS/RR by conditions (for RTCP enable/disable)
    MediaProfileUtil::SetRtcpRsRr(GetLocalProfile(pNewOaModel), m_pConfig);
    m_listOaModel.Append(pNewOaModel);

    // Make the SDP from profile
    return MakeSdpFromProfile(pSessionDescriptor, pDescriptor, GetLocalProfile(pNewOaModel));
}

PROTECTED MEDIA_DIRECTION VideoNego::NegotiateOffer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    // Handling exception case
    if (m_pBaseProfile == IMS_NULL || pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_TRACE_I("NegotiateOffer()", 0, 0, 0);

    // Make new Offer/Answer model, and copy source profile
    OaModel* pNewOaModel = new OaModel();
    pNewOaModel->pLocalProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_VIDEO, m_pBaseProfile);

    // Make a destination profile from SDP
    pNewOaModel->pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_VIDEO);

    if (m_pProfileExtractor->Extract(
                pSessionDescriptor, pDescriptor, GetPeerProfile(pNewOaModel)) != IMS_TRUE)
    {
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile from Local & Peer profile
    pNewOaModel->pNegotiatedProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_VIDEO);

    if (MakeNegotiatedProfile(GetLocalProfile(pNewOaModel), GetPeerProfile(pNewOaModel), IMS_TRUE,
                GetNegotiatedProfile(pNewOaModel)) != IMS_TRUE)
    {
        delete pNewOaModel;
        return MEDIA_DIRECTION_INVALID;
    }

    // add session key in NewOaModel
    IMS_TRACE_D("NegotiateOffer() - add session key in NewOaModel [%" PFLS_x "]",
            reinterpret_cast<IMS_SINTP>(pSessionDescriptor), 0, 0);
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);
    m_listOaModel.Append(pNewOaModel);

    // Return the direction of negotiated profile
    return pNewOaModel->pNegotiatedProfile->GetDirection();
}

PROTECTED MEDIA_DIRECTION VideoNego::NegotiateAnswer(
        IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor)
{
    // Handling exception case
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }
    if (m_listOaModel.GetSize() < 1)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_TRACE_I("NegotiateAnswer()", 0, 0, 0);

    // Get the latest OAmodel from list
    OaModel* pNewOaModel = m_listOaModel.GetAt(m_listOaModel.GetSize() - 1);
    if (pNewOaModel == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a destination profile from SDP
    pNewOaModel->pPeerProfile = MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_VIDEO);
    if (m_pProfileExtractor->Extract(
                pSessionDescriptor, pDescriptor, GetPeerProfile(pNewOaModel)) != IMS_TRUE)
    {
        delete pNewOaModel;
        m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    // Make a negotiated profile from Local & Peer profile
    pNewOaModel->pNegotiatedProfile =
            MediaProfileFactory::GetInstance()->CreateProfile(MEDIA_TYPE_VIDEO);

    if (MakeNegotiatedProfile(GetLocalProfile(pNewOaModel), GetPeerProfile(pNewOaModel), IMS_FALSE,
                GetNegotiatedProfile(pNewOaModel)) != IMS_TRUE)
    {
        delete pNewOaModel;
        m_listOaModel.RemoveAt(m_listOaModel.GetSize() - 1);
        return MEDIA_DIRECTION_INVALID;
    }

    // add session key in NewOaModel
    IMS_TRACE_D("NegotiateAnswer() - add session key in NewOaModel [%" PFLS_x "]",
            reinterpret_cast<IMS_SINTP>(pSessionDescriptor), 0, 0);
    pNewOaModel->nSessionDescriptorKey = reinterpret_cast<IMS_SINTP>(pSessionDescriptor);

    // Return the direction of negotiated profile
    return pNewOaModel->pNegotiatedProfile->GetDirection();
}

PROTECTED
IMS_BOOL VideoNego::MakeSdpFromProfile(OUT ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pBaseProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pBaseProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    VideoProfile* pProfile = ProfileCasting(pBaseProfile);

    IMS_TRACE_I("MakeSdpFromProfile() - PayloadSize[%d], AS[%d]",
            pProfile->GetPayloadList().GetSize(), pProfile->GetBandwidthAs(), 0);

    pDescriptor->RemoveAttribute(SdpAttribute::ATTRIBUTE_ALL);
    ImsList<AString> strEmptyList;
    pDescriptor->SetBandwidthInfo(strEmptyList);

    // make "c" line of media level if IP does not matched
    if (!pSessionDescriptor->GetLocalAddress().Equals(pProfile->GetIpAddress()))
    {
        IMS_TRACE_D("MakeSdpFromProfile() - IP does not matched, SessionIP[%s], ProfileIP[%s]",
                pSessionDescriptor->GetLocalAddress().ToCharString(),
                pProfile->GetIpAddress().ToCharString(), 0);

        pDescriptor->SetConnectionAddress(pProfile->GetIpAddress().ToString());
    }

    // make "m" line
    // ------ "m=video xxxx RTP/AVP aaa bbb ccc ddd"
    AStringArray objVideoFormat;
    AString strPayloadNum;

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        VideoProfile::Payload* pPayload = pProfile->GetPayloadAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        strPayloadNum.Sprintf("%d", pPayload->GetRtpMap().GetPayloadNumber());
        objVideoFormat.AddElement(strPayloadNum);
    }

    // make SDPCapNeg attributes for initial SDP if AVPF is supported
    if (pProfile->GetTransportType().EqualsIgnoreCase("RTP/AVPF"))
    {
        pDescriptor->SetMediaDescription(SdpMedia::TYPE_VIDEO, pProfile->GetDataPort(),
                SdpMedia::TRANSPORT_RTP_AVPF, objVideoFormat);
    }
    else
    {
        pDescriptor->SetMediaDescription(SdpMedia::TYPE_VIDEO, pProfile->GetDataPort(),
                SdpMedia::TRANSPORT_RTP_AVP, objVideoFormat);
    }

    // Previously check all payload for RTCP-FB wildcard(*) attributes
    IMS_BOOL bTrrSupportedAll = IMS_TRUE;
    IMS_BOOL bNackSupportedAll = IMS_TRUE;
    IMS_BOOL bTmmbrSupportedAll = IMS_TRUE;
    IMS_BOOL bPliSupportedAll = IMS_TRUE;
    IMS_BOOL bFirSupportedAll = IMS_TRUE;

    if (pProfile->IsAvpfSupported() == IMS_TRUE)
    {
        for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
        {
            VideoProfile::Payload* pPayload = pProfile->GetPayloadAt(i);
            if (pPayload != IMS_NULL)
            {
                if (pPayload->GetRtcpFbAttr().IsTrrSupported() == IMS_FALSE)
                {
                    bTrrSupportedAll = IMS_FALSE;
                }
                if (pPayload->GetRtcpFbAttr().IsNackSupported() == IMS_FALSE)
                {
                    bNackSupportedAll = IMS_FALSE;
                }
                if (pPayload->GetRtcpFbAttr().IsTmmbrSupported() == IMS_FALSE)
                {
                    bTmmbrSupportedAll = IMS_FALSE;
                }
                if (pPayload->GetRtcpFbAttr().IsPliSupported() == IMS_FALSE)
                {
                    bPliSupportedAll = IMS_FALSE;
                }
                if (pPayload->GetRtcpFbAttr().IsFirSupported() == IMS_FALSE)
                {
                    bFirSupportedAll = IMS_FALSE;
                }
            }
        }
    }
    else
    {
        bTrrSupportedAll = IMS_FALSE;
        bNackSupportedAll = IMS_FALSE;
        bTmmbrSupportedAll = IMS_FALSE;
        bPliSupportedAll = IMS_FALSE;
        bFirSupportedAll = IMS_FALSE;
    }

    // make bandwidth
    // ------ "b=AS:xx"
    // ------ "b=AS:xx"
    // ------ "b=AS:xx"
    if (pProfile->GetBandwidthAs() > 0)
    {
        pDescriptor->AddBandwidth(SdpBandwidth::TYPE_AS, pProfile->GetBandwidthAs());

        if (pProfile->GetBandwidthRs() >= 0)
        {
            pDescriptor->AddBandwidth(SdpBandwidth::TYPE_RS, pProfile->GetBandwidthRs());
        }

        if (pProfile->GetBandwidthRr() >= 0)
        {
            pDescriptor->AddBandwidth(SdpBandwidth::TYPE_RR, pProfile->GetBandwidthRr());
        }
    }

    // make each payload
    // ------ "a=rtpmap:104 H264/16000/1"
    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        AString strRtpmap, strFmtp;
        AString strResolutionAttr;
        VIDEO_RESOLUTION eResolution;

        VideoProfile::Payload* pPayload = pProfile->GetPayloadAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        // make "rtpmap"
        strRtpmap.Sprintf("%d %s/%d", pPayload->GetRtpMap().GetPayloadNumber(),
                pPayload->GetRtpMap().GetPayloadType().GetStr(),
                pPayload->GetRtpMap().GetSamplingRate());

        if (pPayload->GetRtpMap().GetChannel() > 0)
        {
            AString strChannel;
            strChannel.Sprintf("/%d", pPayload->GetRtpMap().GetChannel());
            strRtpmap.Append(strChannel);
        }

        IMS_TRACE_I("MakeSdpFromProfile() - Payload[%d], strRtpmap[%s]", i, strRtpmap.GetStr(), 0);

        // make "fmtp"
        // ------ "a=fmtp:104 profile-level-id=42C016; packetization-mode=1;
        // ----------  sprop-parameter-sets=Z0LAFukDwKMg,aM4G4g=="
        SdpAvCodec* pFormat = new SdpAvCodec();

        if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"))
        {
            VideoProfile::AvcFmtp* pAvcFmtp = (VideoProfile::AvcFmtp*)pPayload->GetFmtp();

            if (pAvcFmtp == IMS_NULL)
            {
                delete pFormat;
                continue;
            }

            strFmtp = VideoNegoAvc::SetSdpFmtpFromAvcFmtp(pAvcFmtp);

            eResolution = pAvcFmtp->GetResolution();
        }
        else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H265"))
        {
            VideoProfile::HevcFmtp* pHevcFmtp = (VideoProfile::HevcFmtp*)pPayload->GetFmtp();

            if (pHevcFmtp == IMS_NULL)
            {
                delete pFormat;
                continue;
            }

            strFmtp = VideoNegoHevc::SetSdpFmtpFromHevcFmtp(pHevcFmtp);

            eResolution = pHevcFmtp->GetResolution();
        }

        else
        {
            delete pFormat;
            continue;
        }

        if (strFmtp.GetLength() == 0)
        {
            strFmtp = AString::ConstNull();
        }

        AString strCompletedFmtp = AString::ConstNull();
        if (!strFmtp.IsNULL())
        {
            strCompletedFmtp.Sprintf("%d ", pPayload->GetRtpMap().GetPayloadNumber());
            strCompletedFmtp.Append(strFmtp);
        }
        if (pFormat == IMS_NULL)
        {
            continue;
        }
        if (pFormat->SetParameters(strRtpmap, strCompletedFmtp) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "MakeSdpFromProfile() SetParameters() Fail. strRtpmap[%s], strFmtp[%s]",
                    strRtpmap.GetStr(), strCompletedFmtp.GetStr(), 0);
        }

        // make "image attribute"
        if (pPayload->IsImageAttrIncluded() == IMS_TRUE)
        {
            if (MakeImageAttributeLine(
                        pPayload->GetRtpMap().GetPayloadNumber(), eResolution, strResolutionAttr))
            {
                pDescriptor->AddAttribute(SdpAttribute::IMAGEATTR, strResolutionAttr);
            }
        }

        // make "framesize"
        if (pPayload->IsFrameSizeIncluded() == IMS_TRUE)
        {
            if (MakeFrameSizeLine(
                        pPayload->GetRtpMap().GetPayloadNumber(), eResolution, strResolutionAttr))
            {
                pDescriptor->AddAttribute(SdpAttribute::FRAMESIZE, strResolutionAttr);
            }
        }

        // make "rtcp-fb"
        if ((pProfile->IsAvpfSupported() == IMS_TRUE) &&
                ((pProfile->IsCapaNegoForAvpfSupported() == IMS_FALSE) ||
                        (pProfile->IsCapaNegoForAvpfSupported() == IMS_TRUE &&
                                pProfile->GetCapaNego().IsAttCapaInPcfg() == IMS_FALSE)))
        {
            IMS_SINT32 nPayloadNumForRtcpFb = -1;

            // TRR-INT
            if (bTrrSupportedAll == IMS_TRUE && i == 0)
            {
                nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
            }
            else if (bTrrSupportedAll == IMS_FALSE &&
                    pPayload->GetRtcpFbAttr().IsTrrSupported() == IMS_TRUE)
            {
                nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->GetRtpMap().GetPayloadNumber();
            }

            if (nPayloadNumForRtcpFb != -1)
            {
                AString strTemp = "";
                SdpRtcpFeedback* pTrr_IntAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);

                pTrr_IntAttr->SetType("trr-int");

                strTemp.Sprintf("%d", pPayload->GetRtcpFbAttr().GetTrrInt());
                pTrr_IntAttr->SetParameter(strTemp);

                pFormat->AddExtraParameter(pTrr_IntAttr);
            }

            nPayloadNumForRtcpFb = -1;
            // NACK
            if (bNackSupportedAll == IMS_TRUE && i == 0)
            {
                nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
            }
            else if (bNackSupportedAll == IMS_FALSE &&
                    pPayload->GetRtcpFbAttr().IsNackSupported() == IMS_TRUE)
            {
                nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->GetRtpMap().GetPayloadNumber();
            }

            if (nPayloadNumForRtcpFb != -1)
            {
                SdpRtcpFeedback* pNackAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);
                pNackAttr->SetType("nack");
                pFormat->AddExtraParameter(pNackAttr);
            }

            nPayloadNumForRtcpFb = -1;
            // PLI
            if (bPliSupportedAll == IMS_TRUE && i == 0)
            {
                nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
            }
            else if (bPliSupportedAll == IMS_FALSE &&
                    pPayload->GetRtcpFbAttr().IsPliSupported() == IMS_TRUE)
            {
                nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->GetRtpMap().GetPayloadNumber();
            }

            if (nPayloadNumForRtcpFb != -1)
            {
                SdpRtcpFeedback* pPliAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);
                pPliAttr->SetType("nack");
                pPliAttr->SetParameter("pli");
                pFormat->AddExtraParameter(pPliAttr);
            }

            nPayloadNumForRtcpFb = -1;
            // FIR
            if (bFirSupportedAll == IMS_TRUE && i == 0)
            {
                nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
            }
            else if (bFirSupportedAll == IMS_FALSE &&
                    pPayload->GetRtcpFbAttr().IsFirSupported() == IMS_TRUE)
            {
                nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->GetRtpMap().GetPayloadNumber();
            }

            if (nPayloadNumForRtcpFb != -1)
            {
                SdpRtcpFeedback* pFIRAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);
                pFIRAttr->SetType("ccm");
                pFIRAttr->SetParameter("fir");
                pFormat->AddExtraParameter(pFIRAttr);
            }

            nPayloadNumForRtcpFb = -1;
            // TMMBR
            if (bTmmbrSupportedAll == IMS_TRUE && i == 0)
            {
                nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
            }
            else if (bTmmbrSupportedAll == IMS_FALSE &&
                    pPayload->GetRtcpFbAttr().IsTmmbrSupported() == IMS_TRUE)
            {
                nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->GetRtpMap().GetPayloadNumber();
            }

            if (nPayloadNumForRtcpFb != -1)
            {
                SdpRtcpFeedback* pTmmbrAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);
                pTmmbrAttr->SetType("ccm");
                pTmmbrAttr->SetParameter("tmmbr");
                pFormat->AddExtraParameter(pTmmbrAttr);
            }
        }

        pDescriptor->SetMediaFormat(pFormat);

        delete pFormat;
    }

    // make direction
    pDescriptor->SetDirection(pProfile->GetDirection());

    // make framerate
    pDescriptor->AddAttributeInt(SdpAttribute::FRAMERATE, pProfile->GetFrameRate());

    // make CVO
    if (pProfile->GetCvoId() > 0)
    {
        AString strCvoAttribute;
        strCvoAttribute.Sprintf("%d urn:3gpp:video-orientation", pProfile->GetCvoId());
        pDescriptor->AddAttribute(SdpAttribute::ATTRIBUTE_OTHER, strCvoAttribute, "extmap");
    }

    // make Capa Nego Attribute
    if (pProfile->IsCapaNegoForAvpfSupported() == IMS_TRUE)
    {
        // add "ACFG" if it's a initial answer
        if (pProfile->GetCapaNego().GetAcfg().GetLength() > 0)
        {
            AString strAcfg;
            IMS_TRACE_D("MakeSdpFromProfile() - Negotiated Acfg [%s]",
                    pProfile->GetCapaNego().GetAcfg().GetStr(), 0, 0);
            strAcfg.Sprintf("%s", pProfile->GetCapaNego().GetAcfg().GetStr());
            pDescriptor->AddAttribute(SdpAttribute::ACFG, strAcfg);
        }

        IMS_TRACE_D("MakeSdpFromProfile() Support Avpf[%d], Transport Type[%s]",
                pProfile->IsAvpfSupported(), pProfile->GetTransportType().GetStr(), 0);

        if (pProfile->IsAvpfSupported() == IMS_TRUE &&
                pProfile->GetTransportType().Contains("AVPF") == IMS_FALSE)
        {
            // make tcap, acap, pcfg for capa nego offer...
            IMS_UINT32 i = 0;
            // AString strTcap = "1 RTP/AVPF";             // only support avpf profile
            AString strTcap = "";
            AString strAcap = "";
            AString strPcfg = "";

            IMS_TRACE_I("MakeSdpFromProfile() - Entered, PcfgSize[%d], TcapSize[%d], AcapSize[%d]",
                    pProfile->GetCapaNego().GetListPcfg().GetSize(),
                    pProfile->GetCapaNego().GetMapTcap().GetSize(),
                    pProfile->GetCapaNego().GetMapAcap().GetSize());

            for (i = 0; i < pProfile->GetCapaNego().GetMapTcap().GetSize(); i++)
            {
                strTcap = "";
                strTcap.Sprintf("%d %s", i + 1,
                        pProfile->GetCapaNego().GetMapTcap().GetValueAt(i).GetStr());
                pDescriptor->AddAttribute(SdpAttribute::TCAP, strTcap);
            }

            if (pProfile->GetCapaNego().IsAttCapaInPcfg() == IMS_TRUE)
            {
                for (i = 0; i < pProfile->GetCapaNego().GetMapAcap().GetSize(); i++)
                {
                    strAcap = "";
                    strAcap.Sprintf("%d %s", i + 1,
                            pProfile->GetCapaNego().GetMapAcap().GetValueAt(i).GetStr());
                    pDescriptor->AddAttribute(SdpAttribute::ACAP, strAcap);
                    IMS_TRACE_I("MakeSdpFromProfile() - Add strAcap : %s", strAcap.GetStr(), 0, 0);
                }
            }

            for (i = 0; i < pProfile->GetCapaNego().GetListPcfg().GetSize(); i++)
            {
                strPcfg = "";
                strPcfg.Sprintf(
                        "%d %s", i + 1, pProfile->GetCapaNego().GetListPcfg().GetAt(i).GetStr());
                pDescriptor->AddAttribute(SdpAttribute::PCFG, strPcfg);
            }
        }
    }

    return IMS_TRUE;
}

PRIVATE IMS_BOOL VideoNego::MakeNegotiatedPayload(IN VideoProfile::Payload* pLocalPayload,
        IN VideoProfile::Payload* pPeerPayload, OUT VideoProfile::Payload* pNegoPayload)
{
    if (pLocalPayload == IMS_NULL || pPeerPayload == IMS_NULL || pNegoPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    *pNegoPayload = *pLocalPayload;
    pNegoPayload->GetRtpMap().SetPayloadNumber(pPeerPayload->GetRtpMap().GetPayloadNumber());
    pNegoPayload->SetIncludeFrameSize(pLocalPayload->IsFrameSizeIncluded());
    pNegoPayload->SetIncludeImageAttr(pLocalPayload->IsImageAttrIncluded());

    return IMS_TRUE;
}

PRIVATE IMS_BOOL VideoNego::MakeNegotiatedProfile(IN VideoProfile* pLocalProfile,
        IN VideoProfile* pPeerProfile, IN IMS_BOOL bIsOfferReceived,
        OUT VideoProfile* pNegotiatedProfile)
{
    IMS_BOOL ret = IMS_FALSE;

    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "MakeNegotiatedProfile() invalid argument", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_pConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Setting IP of mine
    pNegotiatedProfile->SetIpAddress(pLocalProfile->GetIpAddress());

    IMS_TRACE_D("MakeNegotiatedProfile() - IPAddr nego[%s] src[%s] DestPayloadSize[%d]",
            pNegotiatedProfile->GetIpAddress().ToCharString(),
            pLocalProfile->GetIpAddress().ToCharString(), pPeerProfile->GetPayloadList().GetSize());

    // Setting RTP/RTCP port of mine
    pNegotiatedProfile->SetDataPort(pLocalProfile->GetDataPort());
    pNegotiatedProfile->SetControlPort(pLocalProfile->GetControlPort());

    if (pNegotiatedProfile->GetDataPort() == 0 || pPeerProfile->GetDataPort() == 0)
    {
        *pNegotiatedProfile = *pLocalProfile;
        pNegotiatedProfile->SetDataPort(0);
        pNegotiatedProfile->SetNegotiatedPayloadIndex(-1);

        IMS_TRACE_D("MakeNegotiatedProfile() - ZERO Port. DO NOT Use the video[%d][%d]",
                pNegotiatedProfile->GetDataPort(), pPeerProfile->GetDataPort(), 0);
        return IMS_TRUE;
    }

    // Setting profile type
    if (pLocalProfile->IsAvpfSupported() == IMS_TRUE && pPeerProfile->IsAvpfSupported() == IMS_TRUE)
    {
        pNegotiatedProfile->SetSupportAvpf(IMS_TRUE);
    }

    pNegotiatedProfile->SetSupportCapaNegoForAvpf(pPeerProfile->IsCapaNegoForAvpfSupported());

    IMS_TRACE_I("MakeNegotiatedProfile() - PeerProfile: CapaNegoForAVPF[%d], Avpf[%d]",
            pNegotiatedProfile->IsCapaNegoForAvpfSupported(), pNegotiatedProfile->IsAvpfSupported(),
            0);

    // Capability Negotiation for AVPF, SRTP
    if (pNegotiatedProfile->IsCapaNegoForAvpfSupported() == IMS_TRUE)
    {
        if (MakeNegotiatedCapaNegoProfile(&(pLocalProfile->GetCapaNego()),
                    &(pPeerProfile->GetCapaNego()),
                    &(pNegotiatedProfile->GetCapaNego())) != IMS_TRUE)
        {
            // Capa Nego Fail, return to original transport protocol.
            IMS_TRACE_D("MakeNegotiatedProfile() - Capability Negotiation Fail Case", 0, 0, 0);
        }
        else
        {
            // Check Negotiated Transport Type
            IMS_BOOL bNegotiatedAVPF = IMS_FALSE;

            for (IMS_UINT32 i = 0; i < pNegotiatedProfile->GetCapaNego().GetMapTcap().GetSize();
                    i++)
            {
                AString strAttribute = pNegotiatedProfile->GetCapaNego().GetMapTcap().GetValueAt(i);

                if (strAttribute != IMS_NULL && strAttribute.Contains("AVPF") == IMS_TRUE)
                {
                    bNegotiatedAVPF = IMS_TRUE;
                }
            }

            if (pNegotiatedProfile->IsCapaNegoForAvpfSupported() == IMS_TRUE)
            {
                pNegotiatedProfile->SetSupportAvpf(bNegotiatedAVPF);
            }
        }

        pNegotiatedProfile->GetCapaNego().GetMapTcap().Clear();
        pNegotiatedProfile->GetCapaNego().GetMapAcap().Clear();
    }

    pNegotiatedProfile->SetTransportType(
            (pNegotiatedProfile->IsAvpfSupported() == IMS_TRUE) ? "RTP/AVPF" : "RTP/AVP");

    IMS_TRACE_D("MakeNegotiatedProfile() - AVPF enable[%d], Transport Type[%s]",
            pNegotiatedProfile->IsAvpfSupported(), pNegotiatedProfile->GetTransportType().GetStr(),
            0);

    // Compare each payload based destination's profile
    VideoProfile::Payload* pNegotiatedPayload = IMS_NULL;
    IMS_SINT32 nNegotiatedMaxFrameRate = 0;
    IMS_SINT32 nNegotiatedMaxAs = 0;

    VideoProfile::Payload* pLocalPayload = IMS_NULL;
    VideoProfile::Payload* pPeerPayload = IMS_NULL;
    VideoProfile::Payload* pTmpPayload = IMS_NULL;
    VideoProfile::Payload* pMatchedPeerPayload = IMS_NULL;

    for (IMS_UINT32 nPeerIndex = 0; nPeerIndex < pPeerProfile->GetPayloadList().GetSize();
            nPeerIndex++)
    {
        if (pNegotiatedProfile->GetPayloadList().GetSize() > 0)
        {
            break;
        }

        pPeerPayload = pPeerProfile->GetPayloadAt(nPeerIndex);

        if (pPeerPayload == IMS_NULL)
        {
            continue;
        }

        if (pPeerPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"))
        {
            // start source profile loop
            for (IMS_UINT32 nLocalIndex = 0;
                    nLocalIndex < pLocalProfile->GetPayloadList().GetSize(); nLocalIndex++)
            {
                pLocalPayload = pLocalProfile->GetPayloadAt(nLocalIndex);

                if (pLocalPayload == IMS_NULL)
                {
                    continue;
                }

                // find matched payload - H264 find options
                if (pLocalPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"))
                {
                    // FMTP compare
                    VideoProfile::AvcFmtp* pLocalFmtp =
                            (VideoProfile::AvcFmtp*)pLocalPayload->GetFmtp();
                    VideoProfile::AvcFmtp* pPeerFmtp =
                            (VideoProfile::AvcFmtp*)pPeerPayload->GetFmtp();

                    if (pLocalFmtp == IMS_NULL || pPeerFmtp == IMS_NULL)
                    {
                        continue;
                    }

                    IMS_TRACE_D("MakeNegotiatedProfile() profileLevelID[%s]<->profileLevelID[%s]",
                            pLocalFmtp->GetProfileLevelId().GetStr(),
                            pPeerFmtp->GetProfileLevelId().GetStr(), 0);

                    IMS_TRACE_D("MakeNegotiatedProfile() Level[%d]<->Level[%d]",
                            pLocalFmtp->GetLevel(), pPeerFmtp->GetLevel(), 0);
                    IMS_TRACE_D("MakeNegotiatedProfile() Profile[%d]<->Profile[%d]",
                            pLocalFmtp->GetProfile(), pPeerFmtp->GetProfile(), 0);

                    // same level is adapt first, reject higher level
                    if (pLocalFmtp->GetLevel() < pPeerFmtp->GetLevel())
                    {
                        IMS_TRACE_D("MakeNegotiatedProfile() NOT MATCHED AVC Level[%d]<->[%d]",
                                pLocalFmtp->GetLevel(), pPeerFmtp->GetLevel(), 0);

                        if (pTmpPayload == IMS_NULL)
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() Accept Highest Temp Src \
                                    profileLevelID[%s]",
                                    pLocalFmtp->GetProfileLevelId().GetStr(), 0, 0);
                            pTmpPayload = pLocalPayload;
                            pMatchedPeerPayload = pPeerPayload;
                        }

                        continue;
                    }
                    else
                    {
                        if (pLocalFmtp->GetLevel() != pPeerFmtp->GetLevel())
                        {
                            IMS_BOOL bFoundPayload = IMS_FALSE;

                            for (IMS_UINT32 nIndex = nLocalIndex;
                                    nIndex < pLocalProfile->GetPayloadList().GetSize(); nIndex++)
                            {
                                // if find matching level fmtp, skip unmatched level payload
                                VideoProfile::Payload* pPotentialPayload =
                                        pLocalProfile->GetPayloadAt(nIndex);

                                if (pPotentialPayload->GetRtpMap()
                                                .GetPayloadType()
                                                .EqualsIgnoreCase("H264"))
                                {
                                    VideoProfile::AvcFmtp* pPotentialFmtp =
                                            (VideoProfile::AvcFmtp*)pPotentialPayload->GetFmtp();

                                    // check level and payload
                                    if (pPotentialFmtp->GetLevel() == pPeerFmtp->GetLevel() &&
                                            pPotentialFmtp->GetResolution() ==
                                                    pPeerFmtp->GetResolution())
                                    {
                                        bFoundPayload = IMS_TRUE;
                                        break;
                                    }
                                }
                            }

                            if (bFoundPayload == IMS_TRUE)
                            {
                                continue;
                            }
                        }
                    }

                    if (pPeerFmtp->GetResolution() == VIDEO_RESOLUTION_NOT_USED)
                    {
                        VIDEO_RESOLUTION eTempResolution = GetNegotiatedResolution();

                        if (eTempResolution != VIDEO_RESOLUTION_NOT_USED &&
                                eTempResolution != VIDEO_RESOLUTION_INVALID)
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() - Far Resolution is not \
                                    specified[%d] -> Temp use Prev. Negotiated Resolution[%d]",
                                    pPeerFmtp->GetResolution(), eTempResolution, 0);
                            pPeerFmtp->SetResolution(eTempResolution);
                        }
                        else
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() - Far Resolution is not \
                                    specified[%d] -> Temp use Src Resolution[%d]",
                                    pPeerFmtp->GetResolution(), pPeerFmtp->GetResolution(), 0);

                            pPeerFmtp->SetResolution(pLocalFmtp->GetResolution());
                        }
                    }

                    if (pLocalFmtp->GetResolution() != pPeerFmtp->GetResolution())
                    {
                        IMS_TRACE_D("MakeNegotiatedProfile() NOT MATCHED Avc Resolution[%d]<->[%d]",
                                pLocalFmtp->GetResolution(), pPeerFmtp->GetResolution(), 0);

                        if (pLocalFmtp->GetLevel() >= pPeerFmtp->GetLevel())
                        {
                            // Keep 1st payload(resolution mismatched) to be used
                            // when no strictly matched resolution is found
                            if (pTmpPayload == IMS_NULL)
                            {
                                IMS_TRACE_D("MakeNegotiatedProfile() - Keep profileLevelID[%s]",
                                        pLocalFmtp->GetProfileLevelId().GetStr(), 0, 0);
                                pTmpPayload = pLocalPayload;
                                pMatchedPeerPayload = pPeerPayload;
                            }
                        }
                        else
                        {
                            // Keep 1st payload(resolution mismatched) to be used
                            // when no strictly matched resolution is found
                            if (pTmpPayload == IMS_NULL)
                            {
                                IMS_TRACE_D("MakeNegotiatedProfile() - Keep dynamic resolution \
                                        profileLevelID[%s]",
                                        pLocalFmtp->GetProfileLevelId().GetStr(), 0, 0);
                                pTmpPayload = pLocalPayload;
                                pMatchedPeerPayload = pPeerPayload;
                            }
                        }
                        continue;
                    }

                    IMS_TRACE_D("MakeNegotiatedProfile() - Matched payload found, \
                            Profile[%d], Level[%d], Resolution[%d]",
                            pLocalFmtp->GetProfile(), pLocalFmtp->GetLevel(),
                            pLocalFmtp->GetResolution());

                    // make nego payload
                    VideoProfile::Payload* pNegoPayload = new VideoProfile::Payload();

                    if (MakeNegotiatedPayload(pLocalPayload, pPeerPayload, pNegoPayload) ==
                            IMS_FALSE)
                    {
                        IMS_TRACE_E(
                                0, "MakeNegotiatedProfile() - Cannot Make Nego payload", 0, 0, 0);
                        continue;
                    }

                    // Make a RTCP-FB negotiation result
                    if (pNegotiatedProfile->IsAvpfSupported() == IMS_TRUE)
                    {
                        if (pLocalPayload->GetRtcpFbAttr().IsTrrSupported() == IMS_TRUE &&
                                pPeerPayload->GetRtcpFbAttr().IsTrrSupported() == IMS_TRUE)
                        {
                            pNegoPayload->GetRtcpFbAttr().SetTrrInt(
                                    pPeerPayload->GetRtcpFbAttr().GetTrrInt());
                            pNegoPayload->GetRtcpFbAttr().SetTrrSupported(IMS_TRUE);
                        }

                        if (pLocalPayload->GetRtcpFbAttr().IsNackSupported() == IMS_TRUE &&
                                pPeerPayload->GetRtcpFbAttr().IsNackSupported() == IMS_TRUE)
                        {
                            pNegoPayload->GetRtcpFbAttr().SetNackSupported(IMS_TRUE);
                        }

                        if (pLocalPayload->GetRtcpFbAttr().IsTmmbrSupported() == IMS_TRUE &&
                                pPeerPayload->GetRtcpFbAttr().IsTmmbrSupported() == IMS_TRUE)
                        {
                            pNegoPayload->GetRtcpFbAttr().SetTmmbrSupported(IMS_TRUE);
                        }

                        if (pLocalPayload->GetRtcpFbAttr().IsPliSupported() == IMS_TRUE &&
                                pPeerPayload->GetRtcpFbAttr().IsPliSupported() == IMS_TRUE)
                        {
                            pNegoPayload->GetRtcpFbAttr().SetPliSupported(IMS_TRUE);
                        }

                        if (pLocalPayload->GetRtcpFbAttr().IsFirSupported() == IMS_TRUE &&
                                pPeerPayload->GetRtcpFbAttr().IsFirSupported() == IMS_TRUE)
                        {
                            pNegoPayload->GetRtcpFbAttr().SetFirSupported(IMS_TRUE);
                        }

                        IMS_TRACE_D("MakeNegotiatedProfile() - AVPF supported. \
                                bNACK[%d], bTMMBR[%d], bPLI[%d]",
                                pNegoPayload->GetRtcpFbAttr().IsNackSupported(),
                                pNegoPayload->GetRtcpFbAttr().IsTmmbrSupported(),
                                pNegoPayload->GetRtcpFbAttr().IsPliSupported());
                        IMS_TRACE_D("MakeNegotiatedProfile() - AVPF supported. \
                                bFIR[%d], bTRR_Int[%d], nTrr-int[%d]",
                                pNegoPayload->GetRtcpFbAttr().IsFirSupported(),
                                pNegoPayload->GetRtcpFbAttr().IsTrrSupported(),
                                pNegoPayload->GetRtcpFbAttr().GetTrrInt());
                    }

                    VideoProfile::AvcFmtp* fmtp = (VideoProfile::AvcFmtp*)pNegoPayload->GetFmtp();

                    if (fmtp == IMS_NULL)
                    {
                        break;
                    }

                    pNegotiatedProfile->GetPayloadList().Append(pNegoPayload);

                    if (pPeerProfile->GetNegotiatedPayloadIndex() == -1)
                    {
                        pPeerProfile->SetNegotiatedPayloadIndex(nPeerIndex);
                        pLocalProfile->SetNegotiatedPayloadIndex(nLocalIndex);

                        // MT case : change src PT# to dest PT#
                        if (bIsOfferReceived == IMS_TRUE &&
                                pLocalProfile->GetNegotiatedPayloadIndex() != -1)
                        {
                            VideoProfile::Payload* pTempNegoLocalPayload =
                                    pLocalProfile->GetPayloadAt(
                                            pLocalProfile->GetNegotiatedPayloadIndex());
                            pTempNegoLocalPayload->GetRtpMap().SetPayloadNumber(
                                    pPeerPayload->GetRtpMap().GetPayloadNumber());
                        }
                    }

                    if (fmtp->GetFramerate() > nNegotiatedMaxFrameRate)
                    {
                        nNegotiatedMaxFrameRate = fmtp->GetFramerate();
                    }
                    if (fmtp->GetAs() > nNegotiatedMaxAs)
                    {
                        nNegotiatedMaxAs = fmtp->GetAs();
                    }

                    break;
                }
            }
        }
        else if (pPeerPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H265"))
        {
            // start source profile loop
            for (IMS_UINT32 nLocalIndex = 0;
                    nLocalIndex < pLocalProfile->GetPayloadList().GetSize(); nLocalIndex++)
            {
                pLocalPayload = pLocalProfile->GetPayloadAt(nLocalIndex);
                if (pLocalPayload == IMS_NULL)
                {
                    continue;
                }

                // find matched payload - H265 find options
                if (pLocalPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H265"))
                {
                    // FMTP compare
                    VideoProfile::HevcFmtp* pLocalFmtp =
                            (VideoProfile::HevcFmtp*)pLocalPayload->GetFmtp();
                    VideoProfile::HevcFmtp* pPeerFmtp =
                            (VideoProfile::HevcFmtp*)pPeerPayload->GetFmtp();
                    if (pLocalFmtp == IMS_NULL || pPeerFmtp == IMS_NULL)
                    {
                        continue;
                    }

                    IMS_TRACE_D("MakeNegotiatedProfile() - profileId[%d]<->profileId[%d]",
                            pLocalFmtp->GetProfile(), pPeerFmtp->GetProfile(), 0);

                    // same level is adapt first, reject higher level
                    if (pLocalFmtp->GetLevel() < pPeerFmtp->GetLevel())
                    {
                        IMS_TRACE_D("MakeNegotiatedProfile() - NOT MATCHED HEVC Level[%d]<->[%d]",
                                pLocalFmtp->GetLevel(), pPeerFmtp->GetLevel(), 0);

                        if (pTmpPayload == IMS_NULL)
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() - Accept Highest Temp Src \
                                    profileID[%d]",
                                    pLocalFmtp->GetProfile(), 0, 0);
                            pTmpPayload = pLocalPayload;
                            pMatchedPeerPayload = pPeerPayload;
                        }
                        continue;
                    }

                    if (pPeerFmtp->GetResolution() == VIDEO_RESOLUTION_NOT_USED)
                    {
                        VIDEO_RESOLUTION eTempResolution = GetNegotiatedResolution();

                        if (eTempResolution != VIDEO_RESOLUTION_NOT_USED &&
                                eTempResolution != VIDEO_RESOLUTION_INVALID)
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() - Far Resolution is not \
                                    specified[%d] -> Temp use Prev. Negotiated Resolution[%d]",
                                    pPeerFmtp->GetResolution(), eTempResolution, 0);
                            pPeerFmtp->SetResolution(eTempResolution);
                        }
                        else
                        {
                            IMS_TRACE_D("MakeNegotiatedProfile() - Far Resolution is not \
                                    specified[%d] -> Temp use Src Resolution[%d]",
                                    pPeerFmtp->GetResolution(), pPeerFmtp->GetResolution(), 0);
                            pPeerFmtp->SetResolution(pLocalFmtp->GetResolution());
                        }
                    }

                    if (pLocalFmtp->GetResolution() != pPeerFmtp->GetResolution())
                    {
                        IMS_TRACE_D("MakeNegotiatedProfile() - NOT MATCHED HEVC Resolution\
                                [%d]<->[%d]",
                                pLocalFmtp->GetResolution(), pPeerFmtp->GetResolution(), 0);

                        if (pLocalFmtp->GetLevel() >= pPeerFmtp->GetLevel())
                        {
                            // Keep 1st payload(resolution mismatched) to be used
                            // when no strictly matched resolution is found
                            if (pTmpPayload == IMS_NULL)
                            {
                                IMS_TRACE_D("MakeNegotiatedProfile() - Keep profile[%d]",
                                        pLocalFmtp->GetProfile(), 0, 0);
                                pTmpPayload = pLocalPayload;
                                pMatchedPeerPayload = pPeerPayload;
                            }
                        }
                        else
                        {
                            // Keep 1st payload(resolution mismatched) to be used
                            // when no strictly matched resolution is found
                            if (pTmpPayload == IMS_NULL)
                            {
                                IMS_TRACE_D("MakeNegotiatedProfile() - Keep dynamic resolution \
                                        profileID[%d]",
                                        pLocalFmtp->GetProfile(), 0, 0);
                                pTmpPayload = pLocalPayload;
                                pMatchedPeerPayload = pPeerPayload;
                            }
                        }
                        continue;
                    }

                    IMS_TRACE_D("MakeNegotiatedProfile() - Matched payload found, \
                            Profile[%d], Level[%d], Resolution[%d]",
                            pLocalFmtp->GetProfile(), pLocalFmtp->GetLevel(),
                            pLocalFmtp->GetResolution());

                    // make nego payload
                    VideoProfile::Payload* pNegoPayload = new VideoProfile::Payload();

                    if (MakeNegotiatedPayload(pLocalPayload, pPeerPayload, pNegoPayload) ==
                            IMS_FALSE)
                    {
                        IMS_TRACE_E(0, "MakeNegotiatedProfile() Cannot Make Nego payload", 0, 0, 0);
                        continue;
                    }

                    // Make a RTCP-FB negotiation result
                    if (pNegotiatedProfile->IsAvpfSupported() == IMS_TRUE)
                    {
                        if (pLocalPayload->GetRtcpFbAttr().IsNackSupported() == IMS_TRUE &&
                                pPeerPayload->GetRtcpFbAttr().IsNackSupported() == IMS_TRUE)
                        {
                            pNegoPayload->GetRtcpFbAttr().SetNackSupported(IMS_TRUE);
                        }
                        if (pLocalPayload->GetRtcpFbAttr().IsTmmbrSupported() == IMS_TRUE &&
                                pPeerPayload->GetRtcpFbAttr().IsTmmbrSupported() == IMS_TRUE)
                        {
                            pNegoPayload->GetRtcpFbAttr().SetTmmbrSupported(IMS_TRUE);
                        }
                        if (pLocalPayload->GetRtcpFbAttr().IsPliSupported() == IMS_TRUE &&
                                pPeerPayload->GetRtcpFbAttr().IsPliSupported() == IMS_TRUE)
                        {
                            pNegoPayload->GetRtcpFbAttr().SetPliSupported(IMS_TRUE);
                        }
                        if (pLocalPayload->GetRtcpFbAttr().IsFirSupported() == IMS_TRUE &&
                                pPeerPayload->GetRtcpFbAttr().IsFirSupported() == IMS_TRUE)
                        {
                            pNegoPayload->GetRtcpFbAttr().SetFirSupported(IMS_TRUE);
                        }

                        IMS_TRACE_D("MakeNegotiatedProfile() - AVPF supported. \
                                NACK[%d], TMMBR[%d], PLI[%d]",
                                pNegoPayload->GetRtcpFbAttr().IsNackSupported(),
                                pNegoPayload->GetRtcpFbAttr().IsTmmbrSupported(),
                                pNegoPayload->GetRtcpFbAttr().IsPliSupported());
                        IMS_TRACE_D("MakeNegotiatedProfile() - AVPF supported. FIR[%d]",
                                pNegoPayload->GetRtcpFbAttr().IsFirSupported(), 0, 0);
                    }

                    VideoProfile::HevcFmtp* fmtp = (VideoProfile::HevcFmtp*)pNegoPayload->GetFmtp();

                    if (fmtp == IMS_NULL)
                    {
                        break;
                    }

                    pNegotiatedProfile->GetPayloadList().Append(pNegoPayload);

                    if (pPeerProfile->GetNegotiatedPayloadIndex() == -1)
                    {
                        pPeerProfile->SetNegotiatedPayloadIndex(nPeerIndex);
                        pLocalProfile->SetNegotiatedPayloadIndex(nLocalIndex);

                        // MT case : change src PT# to dest PT#
                        if (bIsOfferReceived == IMS_TRUE &&
                                pLocalProfile->GetNegotiatedPayloadIndex() != -1)
                        {
                            VideoProfile::Payload* pTempNegoLocalPayload =
                                    pLocalProfile->GetPayloadAt(
                                            pLocalProfile->GetNegotiatedPayloadIndex());

                            pTempNegoLocalPayload->GetRtpMap().SetPayloadNumber(
                                    pPeerPayload->GetRtpMap().GetPayloadNumber());
                        }
                    }

                    if (fmtp->GetFramerate() > nNegotiatedMaxFrameRate)
                    {
                        nNegotiatedMaxFrameRate = fmtp->GetFramerate();
                    }
                    if (fmtp->GetAs() > nNegotiatedMaxAs)
                    {
                        nNegotiatedMaxAs = fmtp->GetAs();
                    }

                    break;
                }
            }
        }
        else
        {
            IMS_TRACE_D("MakeNegotiatedProfile() UNSUPPORTED codec[%s]",
                    pPeerPayload->GetRtpMap().GetPayloadType().GetStr(), 0, 0);
        }
    }

    if (pNegotiatedProfile->GetPayloadList().GetSize() > 0)
    {
        pNegotiatedPayload = pNegotiatedProfile->GetPayloadAt(0);
    }
    else  // negotiated payload is not exist, use temporary payload
    {
        if (pTmpPayload != IMS_NULL)
        {
            VideoProfile::Payload* pNegoPayload = new VideoProfile::Payload();

            if (MakeNegotiatedPayload(pTmpPayload, pMatchedPeerPayload, pNegoPayload) == IMS_FALSE)
            {
                IMS_TRACE_E(0, "MakeNegotiatedProfile() - Cannot Make Nego payload", 0, 0, 0);
                return IMS_FALSE;
            }

            if (pMatchedPeerPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"))
            {
                VideoProfile::AvcFmtp* pAvcFmtp = (VideoProfile::AvcFmtp*)pNegoPayload->GetFmtp();

                if (pAvcFmtp == IMS_NULL)
                {
                    return IMS_FALSE;
                }

                VIDEO_RESOLUTION nProperResol = GetAvcMaxResolutionFromLevel(pAvcFmtp->GetLevel());

                // if the set resolution is invalid or too big with level, decide resolution via
                // payload pre-set and negotatied level
                IMS_BOOL bFoundResol = IMS_FALSE;

                // first decide with source profile payload
                for (IMS_UINT32 nLocalIndex = 0;
                        nLocalIndex < pLocalProfile->GetPayloadList().GetSize(); nLocalIndex++)
                {
                    VideoProfile::Payload* pPayload = pLocalProfile->GetPayloadAt(nLocalIndex);
                    VideoProfile::AvcFmtp* pTempLocalFmtp =
                            reinterpret_cast<VideoProfile::AvcFmtp*>(pPayload->GetFmtp());

                    if (pTempLocalFmtp->GetLevel() <= pAvcFmtp->GetLevel())
                    {
                        pAvcFmtp->SetResolution(pTempLocalFmtp->GetResolution());
                        bFoundResol = IMS_TRUE;
                        break;
                    }
                }

                // decide by level
                if (bFoundResol == IMS_FALSE)
                {
                    pAvcFmtp->SetResolution(nProperResol);
                }

                IMS_TRACE_D("MakeNegotiatedProfile() set profile[%s] set resolution[%d]",
                        pAvcFmtp->GetProfileLevelId().GetStr(), pAvcFmtp->GetResolution(), 0);

                if (pNegotiatedProfile->IsAvpfSupported() == IMS_TRUE)
                {
                    if (pLocalPayload->GetRtcpFbAttr().IsTrrSupported() == IMS_TRUE &&
                            pPeerPayload->GetRtcpFbAttr().IsTrrSupported() == IMS_TRUE)
                    {
                        pNegoPayload->GetRtcpFbAttr().SetTrrInt(
                                pPeerPayload->GetRtcpFbAttr().GetTrrInt());
                        pNegoPayload->GetRtcpFbAttr().SetTrrSupported(IMS_TRUE);
                    }
                    if (pLocalPayload->GetRtcpFbAttr().IsNackSupported() == IMS_TRUE &&
                            pPeerPayload->GetRtcpFbAttr().IsNackSupported() == IMS_TRUE)
                    {
                        pNegoPayload->GetRtcpFbAttr().SetNackSupported(IMS_TRUE);
                    }
                    if (pLocalPayload->GetRtcpFbAttr().IsTmmbrSupported() == IMS_TRUE &&
                            pPeerPayload->GetRtcpFbAttr().IsTmmbrSupported() == IMS_TRUE)
                    {
                        pNegoPayload->GetRtcpFbAttr().SetTmmbrSupported(IMS_TRUE);
                    }
                    if (pLocalPayload->GetRtcpFbAttr().IsPliSupported() == IMS_TRUE &&
                            pPeerPayload->GetRtcpFbAttr().IsPliSupported() == IMS_TRUE)
                    {
                        pNegoPayload->GetRtcpFbAttr().SetPliSupported(IMS_TRUE);
                    }
                    if (pLocalPayload->GetRtcpFbAttr().IsFirSupported() == IMS_TRUE &&
                            pPeerPayload->GetRtcpFbAttr().IsFirSupported() == IMS_TRUE)
                    {
                        pNegoPayload->GetRtcpFbAttr().SetFirSupported(IMS_TRUE);
                    }

                    IMS_TRACE_D("MakeNegotiatedProfile() - AVPF NACK[%d], TMMBR[%d], PLI[%d]",
                            pNegoPayload->GetRtcpFbAttr().IsNackSupported(),
                            pNegoPayload->GetRtcpFbAttr().IsTmmbrSupported(),
                            pNegoPayload->GetRtcpFbAttr().IsPliSupported());
                    IMS_TRACE_D("MakeNegotiatedProfile() - AVPF FIR[%d], TRR[%d], Trr-int[%d]",
                            pNegoPayload->GetRtcpFbAttr().IsFirSupported(),
                            pNegoPayload->GetRtcpFbAttr().IsTrrSupported(),
                            pNegoPayload->GetRtcpFbAttr().GetTrrInt());
                }

                if (pPeerProfile->GetNegotiatedPayloadIndex() == -1)
                {
                    pPeerProfile->SetNegotiatedPayloadIndex(
                            FindPayloadIndexFromProfile(pPeerProfile, pMatchedPeerPayload));
                    pLocalProfile->SetNegotiatedPayloadIndex(
                            FindPayloadIndexFromProfile(pLocalProfile, pTmpPayload));

                    // MT case : change src PT# to dest PT#
                    if (bIsOfferReceived == IMS_TRUE &&
                            pLocalProfile->GetNegotiatedPayloadIndex() != -1)
                    {
                        VideoProfile::Payload* pTempNegoLocalPayload = pLocalProfile->GetPayloadAt(
                                pLocalProfile->GetNegotiatedPayloadIndex());

                        pTempNegoLocalPayload->GetRtpMap().SetPayloadNumber(
                                pPeerPayload->GetRtpMap().GetPayloadNumber());
                    }
                }

                pNegotiatedProfile->GetPayloadList().Append(pNegoPayload);
                pNegotiatedPayload = pNegoPayload;
                pNegotiatedProfile->SetNegotiatedPayloadIndex(0);

                if (pAvcFmtp->GetFramerate() > nNegotiatedMaxFrameRate)
                {
                    nNegotiatedMaxFrameRate = pAvcFmtp->GetFramerate();
                }
                if (pAvcFmtp->GetAs() > nNegotiatedMaxAs)
                {
                    nNegotiatedMaxAs = pAvcFmtp->GetAs();
                }
            }
            else if (pMatchedPeerPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H265"))
            {
                // Make a RTCP-FB negotiation result
                if (pNegotiatedProfile->IsAvpfSupported() == IMS_TRUE)
                {
                    if (pLocalPayload->GetRtcpFbAttr().IsNackSupported() == IMS_TRUE &&
                            pPeerPayload->GetRtcpFbAttr().IsNackSupported() == IMS_TRUE)
                    {
                        pNegoPayload->GetRtcpFbAttr().SetNackSupported(IMS_TRUE);
                    }
                    if (pLocalPayload->GetRtcpFbAttr().IsTmmbrSupported() == IMS_TRUE &&
                            pPeerPayload->GetRtcpFbAttr().IsTmmbrSupported() == IMS_TRUE)
                    {
                        pNegoPayload->GetRtcpFbAttr().SetTmmbrSupported(IMS_TRUE);
                    }
                    if (pLocalPayload->GetRtcpFbAttr().IsPliSupported() == IMS_TRUE &&
                            pPeerPayload->GetRtcpFbAttr().IsPliSupported() == IMS_TRUE)
                    {
                        pNegoPayload->GetRtcpFbAttr().SetPliSupported(IMS_TRUE);
                    }
                    if (pLocalPayload->GetRtcpFbAttr().IsFirSupported() == IMS_TRUE &&
                            pPeerPayload->GetRtcpFbAttr().IsFirSupported() == IMS_TRUE)
                    {
                        pNegoPayload->GetRtcpFbAttr().SetFirSupported(IMS_TRUE);
                    }

                    IMS_TRACE_D("MakeNegotiatedProfile() - AVPF supported. \
                            NACK[%d], TMMBR[%d], PLI[%d]",
                            pNegoPayload->GetRtcpFbAttr().IsNackSupported(),
                            pNegoPayload->GetRtcpFbAttr().IsTmmbrSupported(),
                            pNegoPayload->GetRtcpFbAttr().IsPliSupported());
                    IMS_TRACE_D("MakeNegotiatedProfile() - AVPF supported. FIR[%d]",
                            pNegoPayload->GetRtcpFbAttr().IsFirSupported(), 0, 0);
                }

                VideoProfile::HevcFmtp* fmtp =
                        reinterpret_cast<VideoProfile::HevcFmtp*>(pNegoPayload->GetFmtp());
                if (fmtp == IMS_NULL)
                {
                    return IMS_FALSE;
                }

                VideoProfile::HevcFmtp* pTempLocalFmtp =
                        reinterpret_cast<VideoProfile::HevcFmtp*>(pMatchedPeerPayload->GetFmtp());
                fmtp->SetResolution(pTempLocalFmtp->GetResolution());

                if (pPeerProfile->GetNegotiatedPayloadIndex() == -1)
                {
                    pPeerProfile->SetNegotiatedPayloadIndex(
                            FindPayloadIndexFromProfile(pPeerProfile, pMatchedPeerPayload));
                    pLocalProfile->SetNegotiatedPayloadIndex(
                            FindPayloadIndexFromProfile(pLocalProfile, pTmpPayload));

                    // MT case : change src PT# to dest PT#
                    if (bIsOfferReceived == IMS_TRUE &&
                            pLocalProfile->GetNegotiatedPayloadIndex() != -1)
                    {
                        VideoProfile::Payload* pTempNegoLocalPayload = pLocalProfile->GetPayloadAt(
                                pLocalProfile->GetNegotiatedPayloadIndex());

                        pTempNegoLocalPayload->GetRtpMap().SetPayloadNumber(
                                pPeerPayload->GetRtpMap().GetPayloadNumber());
                    }
                }

                pNegotiatedProfile->GetPayloadList().Append(pNegoPayload);
                pNegotiatedPayload = pNegoPayload;
                pNegotiatedProfile->SetNegotiatedPayloadIndex(0);

                if (fmtp->GetFramerate() > nNegotiatedMaxFrameRate)
                {
                    nNegotiatedMaxFrameRate = fmtp->GetFramerate();
                }
                if (fmtp->GetAs() > nNegotiatedMaxAs)
                {
                    nNegotiatedMaxAs = fmtp->GetAs();
                }
            }
        }
    }

    if (pNegotiatedPayload != IMS_NULL)
    {
        if (pNegotiatedProfile->GetDataPort() == 0 || pPeerProfile->GetDataPort() == 0 ||
                pNegotiatedProfile->GetPayloadList().GetSize() == 0)
        {
            pNegotiatedProfile->SetDirection(MEDIA_DIRECTION_INVALID);
        }
        else
        {
            pNegotiatedProfile->SetDirection(UpdateDirectionToMine(
                    pPeerProfile->GetDirection(), pLocalProfile->GetDirection(), bIsOfferReceived));
        }

        // if the case using different interval in live and hold, set here.
        pNegotiatedProfile->SetBandwidthRs(pPeerProfile->GetBandwidthRs());
        pNegotiatedProfile->SetBandwidthRr(pPeerProfile->GetBandwidthRr());

        if (pNegotiatedProfile->GetBandwidthRs() == 0 && pNegotiatedProfile->GetBandwidthRr() == 0)
        {
            pNegotiatedProfile->SetRtcpInterval(0);
            IMS_TRACE_D(
                    "MakeNegotiatedProfile() - negotiated rs and rr are 0, disable rtcp", 0, 0, 0);
        }
        else
        {
            pNegotiatedProfile->SetRtcpInterval(m_pConfig->GetRtcpInterval());

            if (pNegotiatedProfile->GetDirection() == MEDIA_DIRECTION_SEND_RECEIVE &&
                    m_pConfig->GetRtcpLiveInterval() > 0)
            {
                pNegotiatedProfile->SetRtcpInterval(m_pConfig->GetRtcpLiveInterval());
            }
        }

        // Setting bandwidth AS/RS/RR
        VideoProfileUtil::MakeNegotiatedBandwidth(ConfigCasting(m_pConfig), pLocalProfile,
                pPeerProfile, bIsOfferReceived, nNegotiatedMaxAs, pNegotiatedProfile);

        // Setting framerate
        pNegotiatedProfile->SetFrameRate(nNegotiatedMaxFrameRate);

        // Candidate Priority (no need in video)

        // CVO mode
        if (pLocalProfile->GetCvoId() > 0 && pPeerProfile->GetCvoId() > 0)
        {
            pNegotiatedProfile->SetCvoId(pPeerProfile->GetCvoId());
        }
        else
        {
            if (pNegotiatedProfile->GetDataPort() == 0)
            {
                pNegotiatedProfile->SetCvoId(pLocalProfile->GetCvoId());
            }
            else
            {
                pNegotiatedProfile->SetCvoId(0);
            }
        }

        IMS_TRACE_D("MakeNegotiatedProfile() CVO Id[%d]", pNegotiatedProfile->GetCvoId(), 0, 0);

        ret = IMS_TRUE;
    }
    else
    {
        if (pLocalProfile->GetPayloadList().GetSize() > 0)
        {
            IMS_TRACE_D("MakeNegotiatedProfile() There's no negotiated payload. \
                    copy LocalProfile and make port 0 ",
                    0, 0, 0);

            *pNegotiatedProfile = *pLocalProfile;
            pNegotiatedProfile->SetDataPort(0);
            pNegotiatedProfile->SetDirection(MEDIA_DIRECTION_INVALID);
            ret = IMS_TRUE;
        }
        else
        {
            IMS_TRACE_E(0, "There's no Payload in Src Profile", 0, 0, 0);
        }
    }

    IMS_TRACE_D("MakeNegotiatedProfile() Ended - Negotiated srcIndex[%d], destIndex[%d]",
            pLocalProfile->GetNegotiatedPayloadIndex(), pPeerProfile->GetNegotiatedPayloadIndex(),
            0);

    return ret;
}

PRIVATE
VideoProfile::Payload* VideoNego::FindPayloadInProfile(
        IN VideoProfile* pProfile, IN VideoProfile::Payload* pTargetPayload)
{
    if (pProfile == IMS_NULL || pTargetPayload == IMS_NULL)
    {
        return IMS_NULL;
    }

    VideoProfile::Payload* pTempPayload = IMS_NULL;  // To keep secondary payload

    if (m_pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        VideoProfile::Payload* pOriginPayload = pProfile->GetPayloadAt(i);

        if (pOriginPayload == IMS_NULL)
        {
            continue;
        }

        if ((pOriginPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                    pTargetPayload->GetRtpMap().GetPayloadType())) &&
                (pOriginPayload->GetRtpMap().GetSamplingRate() ==
                        pTargetPayload->GetRtpMap().GetSamplingRate()))
        {
            if (pOriginPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"))
            {
                VideoProfile::AvcFmtp* pOriginFmtp =
                        (VideoProfile::AvcFmtp*)pOriginPayload->GetFmtp();
                VideoProfile::AvcFmtp* pReceivedFmtp =
                        (VideoProfile::AvcFmtp*)pTargetPayload->GetFmtp();
                if (pOriginFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
                {
                    continue;
                }

                // same level is adapt first
                IMS_TRACE_D("FindPayloadInProfile() - profileLevelID[%s]<->profileLevelID[%s]",
                        pOriginFmtp->GetProfileLevelId().GetStr(),
                        pReceivedFmtp->GetProfileLevelId().GetStr(), 0);

                if (pOriginFmtp->GetLevel() < pReceivedFmtp->GetLevel())
                {
                    IMS_TRACE_D("FindPayloadInProfile() - NOT MATCHED AVC Level[%d]<->[%d]",
                            pOriginFmtp->GetLevel(), pReceivedFmtp->GetLevel(), 0);

                    if (pTempPayload == IMS_NULL)
                    {
                        IMS_TRACE_D("FindPayloadInProfile() - Priority profileLevelID[%d]",
                                pOriginFmtp->GetProfileLevelId().GetStr(), 0, 0);
                        pTempPayload = pOriginPayload;
                    }
                    continue;
                }

                if (pReceivedFmtp->GetResolution() == VIDEO_RESOLUTION_NOT_USED)
                {
                    VIDEO_RESOLUTION eTempResolution = GetNegotiatedResolution();

                    if (eTempResolution != VIDEO_RESOLUTION_NOT_USED &&
                            eTempResolution != VIDEO_RESOLUTION_INVALID)
                    {
                        IMS_TRACE_D("FindPayloadInProfile() - Far Resolution is not specified[%d]\
                                -> Temp use Prev. Negotiated Resolution[%d]",
                                pReceivedFmtp->GetResolution(), eTempResolution, 0);

                        pReceivedFmtp->SetResolution(eTempResolution);
                    }
                    else
                    {
                        IMS_TRACE_D("FindPayloadInProfile() - Far Resolution is not specified[%d]\
                                -> Temp use Src Resolution[%d]",
                                pReceivedFmtp->GetResolution(), pOriginFmtp->GetResolution(), 0);

                        pReceivedFmtp->SetResolution(pOriginFmtp->GetResolution());
                    }
                }

                if (pOriginFmtp->GetResolution() != pReceivedFmtp->GetResolution())
                {
                    // Keep 1st payload(resolution mismatched) to be used
                    // when no strictly matched resolution is found
                    pTempPayload = pOriginPayload;

                    IMS_TRACE_D("FindPayloadInProfile() - NOT MATCHED AVC Resolution[%d]<->[%d]",
                            pOriginFmtp->GetResolution(), pReceivedFmtp->GetResolution(), 0);
                    continue;
                }
                else if (pOriginFmtp->GetLevel() != pReceivedFmtp->GetLevel())
                {
                    pTempPayload = pOriginPayload;
                    continue;
                }

                IMS_TRACE_D("FindPayloadInProfile() Found, Profile[%d], Level[%d], Resolution[%d]",
                        pOriginFmtp->GetProfile(), pOriginFmtp->GetLevel(),
                        pOriginFmtp->GetResolution());

                return pOriginPayload;
            }
            else if (pOriginPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H265"))
            {
                VideoProfile::HevcFmtp* pOriginFmtp =
                        (VideoProfile::HevcFmtp*)pOriginPayload->GetFmtp();
                VideoProfile::HevcFmtp* pReceivedFmtp =
                        (VideoProfile::HevcFmtp*)pTargetPayload->GetFmtp();
                if (pOriginFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
                {
                    continue;
                }

                // same level is adapt first
                IMS_TRACE_D("FindPayloadInProfile() - profileID[%d] <-> profileID[%d]",
                        pOriginFmtp->GetProfile(), pReceivedFmtp->GetProfile(), 0);

                if (pOriginFmtp->GetLevel() < pReceivedFmtp->GetLevel())
                {
                    IMS_TRACE_D("FindPayloadInProfile() - NOT MATCHED HEVC Level [%d]<->[%d]",
                            pOriginFmtp->GetLevel(), pReceivedFmtp->GetLevel(), 0);
                }

                if (pReceivedFmtp->GetResolution() == VIDEO_RESOLUTION_NOT_USED)
                {
                    VIDEO_RESOLUTION eTempResolution = GetNegotiatedResolution();

                    if (eTempResolution != VIDEO_RESOLUTION_NOT_USED &&
                            eTempResolution != VIDEO_RESOLUTION_INVALID)
                    {
                        IMS_TRACE_D("FindPayloadInProfile() - Far Resolution is not specified[%d]\
                                -> Temp use Prev. Negotiated Resolution[%d]",
                                pReceivedFmtp->GetResolution(), eTempResolution, 0);

                        pReceivedFmtp->SetResolution(eTempResolution);
                    }
                    else
                    {
                        IMS_TRACE_D("FindPayloadInProfile() - Far Resolution is not specified[%d]\
                                -> Temp use Src Resolution[%d]",
                                pReceivedFmtp->GetResolution(), pOriginFmtp->GetResolution(), 0);

                        pReceivedFmtp->SetResolution(pOriginFmtp->GetResolution());
                    }
                }

                if (pOriginFmtp->GetResolution() != pReceivedFmtp->GetResolution())
                {
                    // Keep 1st payload(resolution mismatched) to be used
                    // when no strictly matched resolution is found
                    pTempPayload = pOriginPayload;

                    IMS_TRACE_D("FindPayloadInProfile() - NOT MATCHED HEVC Resolution [%d]<->[%d]",
                            pOriginFmtp->GetResolution(), pReceivedFmtp->GetResolution(), 0);
                    continue;
                }
                else if (pOriginFmtp->GetLevel() != pReceivedFmtp->GetLevel())
                {
                    pTempPayload = pOriginPayload;
                    continue;
                }

                IMS_TRACE_D("FindPayloadInProfile() Found, Profile[%d], Level[%d], Resolution[%d]",
                        pOriginFmtp->GetProfile(), pOriginFmtp->GetLevel(),
                        pOriginFmtp->GetResolution());

                return pOriginPayload;
            }
        }
    }

    // When there's no perfectly matched payload, use secondary (only resolution is mismatched)
    if (pTempPayload != IMS_NULL &&
            pTempPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"))
    {
        VideoProfile::AvcFmtp* pOriginFmtp = (VideoProfile::AvcFmtp*)pTempPayload->GetFmtp();
        VideoProfile::AvcFmtp* pReceivedFmtp = (VideoProfile::AvcFmtp*)pTargetPayload->GetFmtp();
        if (pOriginFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
        {
            return IMS_NULL;
        }

        if (pOriginFmtp->GetResolution() != pReceivedFmtp->GetResolution())
        {
            IMS_TRACE_D("FindPayloadInProfile() - Accept mismatched Resolution[%d]<->[%d]",
                    pOriginFmtp->GetResolution(), pReceivedFmtp->GetResolution(), 0);

            if (pOriginFmtp->GetLevel() >= pReceivedFmtp->GetLevel())
            {
                pOriginFmtp->SetResolution(pReceivedFmtp->GetResolution());
            }
        }
        else if (pOriginFmtp->GetLevel() != pReceivedFmtp->GetLevel())
        {
            IMS_TRACE_D("FindPayloadInProfile() - Accept lower Level[%d]<->[%d]",
                    pOriginFmtp->GetLevel(), pReceivedFmtp->GetLevel(), 0);
        }

        return pTempPayload;
    }
    if (pTempPayload != IMS_NULL &&
            pTempPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H265"))
    {
        VideoProfile::HevcFmtp* pOriginFmtp = (VideoProfile::HevcFmtp*)pTempPayload->GetFmtp();
        VideoProfile::HevcFmtp* pReceivedFmtp = (VideoProfile::HevcFmtp*)pTargetPayload->GetFmtp();
        if (pOriginFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
            return IMS_NULL;

        if (pOriginFmtp->GetResolution() != pReceivedFmtp->GetResolution())
        {
            IMS_TRACE_D("FindPayloadInProfile() - Accept mismatched Resolution [%d]<->[%d]",
                    pOriginFmtp->GetResolution(), pReceivedFmtp->GetResolution(), 0);

            if (pOriginFmtp->GetLevel() >= pReceivedFmtp->GetLevel())
            {
                pOriginFmtp->SetResolution(pReceivedFmtp->GetResolution());
            }
        }
        else if (pOriginFmtp->GetLevel() != pReceivedFmtp->GetLevel())
        {
            IMS_TRACE_D("FindPayloadInProfile() - Accept lower Level[%d]<->[%d]",
                    pOriginFmtp->GetLevel(), pReceivedFmtp->GetLevel(), 0);
        }

        return pTempPayload;
    }

    IMS_TRACE_E(0, "FindPayloadInProfile() - No matched payload Found", 0, 0, 0);
    return IMS_NULL;
}

PRIVATE
IMS_SINT32 VideoNego::FindPayloadIndexFromProfile(
        IN VideoProfile* pProfile, IN const VideoProfile::Payload* pPayload)
{
    if (pProfile == IMS_NULL || pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "FindPayloadInProfile() - Null Input", 0, 0, 0);
        return -1;
    }

    // find the index of negotiated payload
    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        VideoProfile::Payload* comparedPayload = pProfile->GetPayloadAt(i);
        if (comparedPayload == IMS_NULL)
            continue;

        if (comparedPayload == pPayload)
        {
            IMS_TRACE_D("FindPayloadIndexFromProfile() - FindIndex[%d]", i, 0, 0);
            return i;
        }
    }

    return -1;
}

PRIVATE
MEDIA_DIRECTION VideoNego::UpdateDirectionToMine(
        IN MEDIA_DIRECTION ePeerDir, IN MEDIA_DIRECTION eSrcDir, IN IMS_BOOL bIsMtCase)
{
    IMS_TRACE_D("UpdateDirectionToMine() - ePeerDir[%d], eSrcDir[%d], bIsMtCase[%d]", ePeerDir,
            eSrcDir, bIsMtCase);
    MEDIA_DIRECTION eNegotiatedDir = MEDIA_DIRECTION_INVALID;

    switch (ePeerDir)
    {
        case MEDIA_DIRECTION_INACTIVE:
        case MEDIA_DIRECTION_SEND_RECEIVE:
            eNegotiatedDir = ePeerDir;
            break;
        case MEDIA_DIRECTION_RECEIVE:
            eNegotiatedDir = MEDIA_DIRECTION_SEND;
            break;
        case MEDIA_DIRECTION_SEND:
            eNegotiatedDir = MEDIA_DIRECTION_RECEIVE;
            break;
        default:
            return MEDIA_DIRECTION_INVALID;
    }

    if (bIsMtCase == IMS_FALSE)
    {
        // direction check strictly
        if ((eSrcDir == MEDIA_DIRECTION_SEND &&
                    (ePeerDir == MEDIA_DIRECTION_SEND ||
                            ePeerDir == MEDIA_DIRECTION_SEND_RECEIVE)) ||
                (eSrcDir == MEDIA_DIRECTION_RECEIVE &&
                        (ePeerDir == MEDIA_DIRECTION_RECEIVE ||
                                ePeerDir == MEDIA_DIRECTION_SEND_RECEIVE)) ||
                (eSrcDir == MEDIA_DIRECTION_INACTIVE && (ePeerDir != MEDIA_DIRECTION_INACTIVE)))
        {
            return MEDIA_DIRECTION_INVALID;
        }
    }
    return eNegotiatedDir;
}

/** TODO_MEDIA video sprop
PRIVATE IMS_BOOL VideoNego::GetWidthHeightFromSdp_SpropParam(IN VIDEO_CODEC codecType,
        IN IMS_CHAR* szSprop, OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight)
{
    tMMPFGetInfoParam getInfo;
    tMMPFGetInfoResult InfoResult;
    eMMPFResult eResult = MMPF_RESULT_ERR_UNKNOWN;

    if (codecType == VIDEO_CODEC_AVC)
    {
        getInfo.type = MMPF_INFO_PARSE_SPROPPARAM;
    }
    else if (codecType == VIDEO_CODEC_HEVC)
    {
        getInfo.type = MMPF_INFO_PARSE_SPROPPARAM_H265;
    }
    else
    {
        return IMS_FALSE;
    }

    IMS_StrCpy(getInfo.szSpropParam, MMPF_MAX_CONFIG_LEN, szSprop);

    // eResult = MMPFSession::getInfo(MMPF_INTERFACEID_AUTO, &getInfo, &InfoResult);

    if (eResult == MMPF_RESULT_OK)
    {
        *nImageWidth = InfoResult.tCodecAttribute.nWidth;
        *nImageHeight = InfoResult.tCodecAttribute.nHeight;

        IMS_TRACE_D("GetWidthHeightFromSdp_SpropParam() - Parsing Success. nWidth[%d], nHeight[%d]",
                *nImageWidth, *nImageHeight, 0);

        return IMS_TRUE;
    }
    else
    {
        IMS_TRACE_E(0, "GetWidthHeightFromSdp_SpropParam() INVALID szSprop[%s]", szSprop, 0, 0);
        return IMS_FALSE;
    }
}
*/

PRIVATE IMS_BOOL VideoNego::MakeImageAttributeLine(
        IN IMS_UINT32 nPayloadType, IN VIDEO_RESOLUTION eResolutionId, OUT AString& strImageAttr)
{
    IMS_UINT32 nWidth, nHeight;

    if (GetWidthHeightFromResolutionId(eResolutionId, &nWidth, &nHeight) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    strImageAttr.Sprintf(
            "%d send [x=%d,y=%d] recv [x=%d,y=%d]", nPayloadType, nWidth, nHeight, nWidth, nHeight);

    return IMS_TRUE;
}

PRIVATE IMS_BOOL VideoNego::MakeFrameSizeLine(
        IN IMS_UINT32 nPayloadType, IN VIDEO_RESOLUTION eResolutionId, OUT AString& strFrameSize)
{
    IMS_UINT32 nWidth, nHeight;

    if (GetWidthHeightFromResolutionId(eResolutionId, &nWidth, &nHeight) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    strFrameSize.Sprintf("%d %d-%d", nPayloadType, nWidth, nHeight);

    return IMS_TRUE;
}

PRIVATE IMS_BOOL VideoNego::MakeNegotiatedCapaNegoProfile(IN VideoProfile::CapaNego* pLocalCapaNego,
        IN VideoProfile::CapaNego* pPeerCapaNego, OUT VideoProfile::CapaNego* pNegotiatedCapaNego)
{
    if (pLocalCapaNego == IMS_NULL || pPeerCapaNego == IMS_NULL || pNegotiatedCapaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "MakeNegotiatedCapaNego() invalid argument, %" PFLS_x " %" PFLS_x,
                pLocalCapaNego, pPeerCapaNego, 0);
        return IMS_FALSE;
    }

    IMS_BOOL ret = IMS_FALSE;
    IMS_UINT32 i = 0, j = 0, k = 0, l = 0;
    IMS_BOOL bAttributeCheckable = IMS_FALSE;
    IMS_BOOL bPCFGSupportable = IMS_FALSE;

    ImsMap<IMS_SINT32, AString> mapLocalTCap = pLocalCapaNego->GetMapTcap();
    ImsMap<IMS_SINT32, AString> mapLocalACap = pLocalCapaNego->GetMapAcap();

    ImsList<AString> lstDstPCFG = pPeerCapaNego->GetListPcfg();
    ImsMap<IMS_SINT32, AString> mapPeerTCap = pPeerCapaNego->GetMapTcap();
    ImsMap<IMS_SINT32, AString> mapPeerACap = pPeerCapaNego->GetMapAcap();

    if (pPeerCapaNego->GetAcfg().GetLength() > 0)
    {
        if (mapPeerTCap.IsEmpty() == IMS_TRUE)
        {
            pNegotiatedCapaNego->GetMapTcap() = mapLocalTCap;
        }

        IMS_TRACE_I("MakeNegotiatedCapaNego() ACFG - %s", pPeerCapaNego->GetAcfg().GetStr(), 0, 0);
        return IMS_TRUE;
    }

    // parse pcfg
    for (i = 0; i < lstDstPCFG.GetSize(); i++)
    {
        AString strPCFGline = lstDstPCFG.GetAt(i);  // get "# t=# a=#,#,#,# ..."
        if (strPCFGline.GetLength() == 0)
            continue;

        ImsList<AString> lstSplitSpace = lstDstPCFG.GetAt(i).Split(' ');

        for (j = 0; j < lstSplitSpace.GetSize(); j++)
        {
            bAttributeCheckable = IMS_FALSE;
            if (j == 1)  // t=#
            {
                AString strPCFG_Transport = lstSplitSpace.GetAt(j);
                if (strPCFG_Transport.GetLength() == 0)
                    continue;

                ImsList<AString> lstSplitEquals = strPCFG_Transport.Split('=');
                if (lstSplitEquals.GetSize() == 0)
                    continue;

                if (lstSplitEquals.GetAt(0).Equals('t') == IMS_TRUE && lstSplitEquals.GetSize() > 1)
                {
                    // compare transport capa
                    AString strTmp = mapPeerTCap.GetValue(lstSplitEquals.GetAt(1).ToInt32());

                    for (k = 0; k < mapLocalTCap.GetSize(); k++)
                    {
                        if (strTmp.Equals(mapLocalTCap.GetValueAt(k)) == IMS_TRUE)
                        {
                            bAttributeCheckable = IMS_TRUE;
                            bPCFGSupportable = IMS_TRUE;

                            // set Negotiated Transport Capa Nego Value...
                            pNegotiatedCapaNego->GetMapTcap().Add(
                                    lstSplitEquals.GetAt(1).ToInt32(), strTmp);
                            break;
                        }
                    }

                    // if there are no matched transport capa, then next pcfg check...
                    // -----it's first for_loop break case..
                    if (bAttributeCheckable == IMS_FALSE)
                    {
                        IMS_TRACE_I("MakeNegotiatedCapaNego() does not match transport capa - PCFG "
                                    "#[%d]",
                                i, 0, 0);
                        break;
                    }
                    // if there are matched transport capa, check attribute capa
                    // -----it's not first for_loop break case..
                }
            }
            else if (j == 2)  // a=#,#,#,#...
            {
                // if attribute pcfg is exist in SDP, then bPCFGSupportable reset to IMS_FALSE for
                // attribute capa nego..
                bPCFGSupportable = IMS_FALSE;

                AString strPCFG_Attribute = lstSplitSpace.GetAt(j);
                if (strPCFG_Attribute.GetLength() == 0)
                    continue;

                ImsList<AString> lstSplitEquals = strPCFG_Attribute.Split('=');
                if (lstSplitEquals.GetSize() == 0)
                    continue;

                if (lstSplitEquals.GetAt(0).Equals('a') == IMS_TRUE && lstSplitEquals.GetSize() > 1)
                {
                    IMS_UINT32 cnt = 0;
                    // compare Attribute capa
                    AString strTmp = lstSplitEquals.GetAt(1);  // tmp = "1,2,3,4"

                    // attribute comma parsing..
                    ImsList<AString> lstSplitComma = strTmp.Split(',');
                    IMS_TRACE_I("MakeNegotiatedCapaNego() attribute size[%d]",
                            lstSplitComma.GetSize(), 0, 0);

                    if (lstSplitComma.GetSize() == 0)
                        continue;

                    // check attribute capa negotiation
                    for (k = 0; k < lstSplitComma.GetSize(); k++)
                    {
                        AString strDestAttributeCapa =
                                mapPeerACap.GetValue(lstSplitComma.GetAt(k).ToInt32());
                        IMS_TRACE_I("MakeNegotiatedCapaNego() strDestAttributeCapa [%s]",
                                strDestAttributeCapa.GetStr(), 0, 0);

                        if (strDestAttributeCapa.Contains("trr-int") == IMS_TRUE ||
                                strDestAttributeCapa.Contains("nack") == IMS_TRUE)
                        {
                            cnt++;
                            pNegotiatedCapaNego->GetMapAcap().Add(
                                    lstSplitComma.GetAt(k).ToInt32(), strDestAttributeCapa);
                        }
                        else if (strDestAttributeCapa.Contains("ccm") == IMS_TRUE)
                        {
                            if (strDestAttributeCapa.Contains("fir") == IMS_TRUE ||
                                    strDestAttributeCapa.Contains("tmmbr") == IMS_TRUE)
                            {
                                cnt++;
                                pNegotiatedCapaNego->GetMapAcap().Add(
                                        lstSplitComma.GetAt(k).ToInt32(), strDestAttributeCapa);
                            }
                        }
                        else if (strDestAttributeCapa.Contains("crypto") == IMS_TRUE)
                        {
                            // crypto attribute negotiate only srtp profile type
                            ImsList<AString> lstSrcCryptoAttribute =
                                    mapLocalACap.GetValueAt(l).Split(' ');
                            ImsList<AString> lstDestCryptoAttribute =
                                    strDestAttributeCapa.Split(' ');
                            if (lstDestCryptoAttribute.GetAt(1).Equals(
                                        lstSrcCryptoAttribute.GetAt(1)) == IMS_TRUE)
                            {
                                cnt++;
                                pNegotiatedCapaNego->GetMapAcap().Add(
                                        lstSplitComma.GetAt(k).ToInt32(), strDestAttributeCapa);

                                IMS_TRACE_I("MakeNegotiatedCapaNego()  strDestAttributeCapa.Equals "
                                            "CNT[%d]",
                                        cnt, 0, 0);
                                break;
                            }
                        }
                    }

                    // if ue support pcfg about transport capa, bPCFGSupportable variable set to
                    // True..
                    if (cnt == lstSplitComma.GetSize())
                    {
                        IMS_TRACE_I(
                                "MakeNegotiatedCapaNego()  capa nego success.. cnt[%d]", cnt, 0, 0);
                        bPCFGSupportable = IMS_TRUE;
                        break;
                    }
                }
            }
        }

        // check capa nego success
        if (bPCFGSupportable == IMS_TRUE)
        {
            pNegotiatedCapaNego->GetAcfg().Sprintf("%s", strPCFGline.GetStr());
            IMS_TRACE_I("MakeNegotiatedCapaNego() UE support capa nego- ACFG [%s]",
                    strPCFGline.GetStr(), 0, 0);
            // strAcfg value available, if capa nego success.
            ret = IMS_TRUE;
            break;
        }
        else  // capa nego dose not succsee case//
        {
            // clear saved negotiatedCapaNego imfo.
            IMS_TRACE_I("MakeNegotiatedCapaNego() capa nego does not success pcfg[%d]", i, 0, 0);
            pNegotiatedCapaNego->GetMapTcap().Clear();
            pNegotiatedCapaNego->GetMapAcap().Clear();
        }
    }

    return ret;
}

PRIVATE IMS_BOOL VideoNego::GetWidthHeightFromResolutionId(
        IN VIDEO_RESOLUTION eResolutionId, OUT IMS_UINT32* pnWidth, OUT IMS_UINT32* pnHeight)
{
    IMS_TRACE_D("GetWidthHeightFromResolutionId() resolution ID [%d]", eResolutionId, 0, 0);
    switch (eResolutionId)
    {
        case VIDEO_RESOLUTION_QCIF_LS:
            *pnWidth = 176;
            *pnHeight = 144;
            break;
        case VIDEO_RESOLUTION_QCIF_PR:
            *pnWidth = 144;
            *pnHeight = 176;
            break;
        case VIDEO_RESOLUTION_QVGA_LS:
            *pnWidth = 320;
            *pnHeight = 240;
            break;
        case VIDEO_RESOLUTION_QVGA_PR:
            *pnWidth = 240;
            *pnHeight = 320;
            break;
        case VIDEO_RESOLUTION_VGA_LS:
            *pnWidth = 640;
            *pnHeight = 480;
            break;
        case VIDEO_RESOLUTION_VGA_PR:
            *pnWidth = 480;
            *pnHeight = 640;
            break;
        case VIDEO_RESOLUTION_CIF_LS:
            *pnWidth = 352;
            *pnHeight = 288;
            break;
        case VIDEO_RESOLUTION_CIF_PR:
            *pnWidth = 288;
            *pnHeight = 352;
            break;
        case VIDEO_RESOLUTION_SIF_PR:
            *pnWidth = 240;
            *pnHeight = 352;
            break;
        case VIDEO_RESOLUTION_SIF_LS:
            *pnWidth = 352;
            *pnHeight = 240;
            break;
        case VIDEO_RESOLUTION_SQCIF_LS:
            *pnWidth = 128;
            *pnHeight = 96;
            break;
        case VIDEO_RESOLUTION_SQCIF_PR:
            *pnWidth = 96;
            *pnHeight = 128;
            break;
        case VIDEO_RESOLUTION_HD_LS:
            *pnWidth = 1280;
            *pnHeight = 720;
            break;
        case VIDEO_RESOLUTION_HD_PR:
            *pnWidth = 720;
            *pnHeight = 1280;
            break;
        case VIDEO_RESOLUTION_FHD_LS:
            *pnWidth = 1920;
            *pnHeight = 1080;
            break;
        case VIDEO_RESOLUTION_FHD_PR:
            *pnWidth = 1080;
            *pnHeight = 1920;
            break;
        default:
            *pnWidth = 176;
            *pnHeight = 144;
            IMS_TRACE_E(0, "GetWidthHeightFromResolutionId() INVALID resolution ID", 0, 0, 0);

            return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
VIDEO_RESOLUTION VideoNego::GetAvcMaxResolutionFromLevel(IN IMS_UINT32 nLevel)
{
    IMS_TRACE_D("GetAvcMaxResolutionFromLevel() - Level[%d]", nLevel, 0, 0);

    if (nLevel > 31)
    {
        nLevel = 31;
    }

    // default resoltuion is portrait
    switch (nLevel)
    {
        case 31:
            return VIDEO_RESOLUTION_HD_PR;
        case 30:
        case 22:
            return VIDEO_RESOLUTION_VGA_PR;
        case 21:
        case 20:
        case 14:
        case 13:
        case 12:
            return VIDEO_RESOLUTION_CIF_PR;
        case 11:
        case 10:
            return VIDEO_RESOLUTION_QCIF_PR;
        default:
            return VIDEO_RESOLUTION_VGA_PR;
    }
}
