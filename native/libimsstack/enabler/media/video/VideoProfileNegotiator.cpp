/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "video/VideoProfileNegotiator.h"

#include "MediaDef.h"
#include "MediaProfileUtil.h"
#include "ServiceTrace.h"
#include "video/VideoProfile.h"
#include "video/VideoProfileUtil.h"

__IMS_TRACE_TAG_MEDIA__;

namespace
{
template <typename FmtpType>
void NegotiateVideoFmtp(
        const std::shared_ptr<FmtpType>& pNegoFmtp, const std::shared_ptr<FmtpType>& pPeerFmtp)
{
    if (pNegoFmtp->GetLevel() > pPeerFmtp->GetLevel())
    {
        if constexpr (std::is_same_v<FmtpType, VideoProfile::AvcFmtp>)
        {
            pNegoFmtp->SetProfileLevelId(pPeerFmtp->GetProfileLevelId());
        }

        pNegoFmtp->SetLevel(pPeerFmtp->GetLevel());
    }

    if (pPeerFmtp->GetResolution() != VIDEO_RESOLUTION_NOT_USED)
    {
        if (VideoProfileUtil::CompareResolution(
                    pNegoFmtp->GetResolution(), pPeerFmtp->GetResolution()) > 0)
        {
            pNegoFmtp->SetResolution(pPeerFmtp->GetResolution());
        }
    }

    pNegoFmtp->SetPacketizationMode(pPeerFmtp->GetPacketizationMode());

    IMS_TRACE_I("NegotiateVideoFmtp(): profile[%d], level[%d], resolution[%d]",
            pNegoFmtp->GetProfile(), pNegoFmtp->GetLevel(), pNegoFmtp->GetResolution());
}

}  // namespace

PUBLIC VideoProfileNegotiator::VideoProfileNegotiator() :
        MediaProfileNegotiator(MEDIA_TYPE_VIDEO)
{
}

PUBLIC VIRTUAL VideoProfileNegotiator::~VideoProfileNegotiator() {}

PUBLIC IMS_BOOL VideoProfileNegotiator::Negotiate(IN VideoProfile* pLocalProfile,
        IN VideoProfile* pPeerProfile, IN IMS_BOOL bIsOfferReceived,
        OUT VideoProfile* pNegotiatedProfile, IN MediaConfiguration* pConfig)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL ||
            pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "Negotiate(): invalid argument", 0, 0, 0);
        return IMS_FALSE;
    }

    m_bIsOfferReceived = bIsOfferReceived;

    NegotiateIpPort(pLocalProfile, pPeerProfile, pNegotiatedProfile);
    NegotiateAvpf(pLocalProfile, pPeerProfile, pNegotiatedProfile);
    NegotiateTransportType(pNegotiatedProfile);

    IMS_SINT32 nNegotiatedMaxFrameRate = pPeerProfile->GetFrameRate();
    IMS_BOOL bNegotiatedPayload = NegotiatePayload(
            pLocalProfile, pPeerProfile, pNegotiatedProfile, &nNegotiatedMaxFrameRate);

    if (bNegotiatedPayload)
    {
        pNegotiatedProfile->SetDirection(UpdateDirectionToMine(
                pPeerProfile->GetDirection(), pLocalProfile->GetDirection(), m_bIsOfferReceived));

        // Setting bandwidth AS/RS/RR
        MakeNegotiatedBandwidth(static_cast<VideoConfiguration*>(pConfig), pLocalProfile,
                pPeerProfile, m_bIsOfferReceived, pNegotiatedProfile);

        // Setting framerate
        pNegotiatedProfile->SetFrameRate(nNegotiatedMaxFrameRate);
        NegotiateCvo(pLocalProfile, pPeerProfile, pNegotiatedProfile);
        pNegotiatedProfile->SetOmitAttributes(IMS_FALSE);
    }
    else
    {
        // reset using the peer profile
        ResetNegotiatedProfile(IMS_TRUE, pLocalProfile, pPeerProfile,
                reinterpret_cast<MediaBaseProfile**>(&pNegotiatedProfile));
        pNegotiatedProfile->SetOmitAttributes(IMS_TRUE);
    }

    if (pNegotiatedProfile->GetDataPort() == 0)
    {
        pNegotiatedProfile->SetDirection(MEDIA_DIRECTION_INACTIVE);
    }

    // RTCP interval
    if (pNegotiatedProfile->GetBandwidthRs() == 0 && pNegotiatedProfile->GetBandwidthRr() == 0)
    {
        pNegotiatedProfile->SetRtcpInterval(0);
    }
    else
    {
        pNegotiatedProfile->SetRtcpInterval(pConfig->GetRtcpIntervalOnHold());

        if (pNegotiatedProfile->GetDirection() == MEDIA_DIRECTION_SEND_RECEIVE &&
                pConfig->GetRtcpIntervalOnActive() > 0)
        {
            pNegotiatedProfile->SetRtcpInterval(pConfig->GetRtcpIntervalOnActive());
        }
    }

    IMS_TRACE_D("Negotiate(): negotiated payload size[%d], port[%d], direction[%d], ",
            pNegotiatedProfile->GetPayloadListSize(), pNegotiatedProfile->GetDataPort(),
            pNegotiatedProfile->GetDirection());

    return IMS_TRUE;
}

PRIVATE
void VideoProfileNegotiator::NegotiateAvpf(IN VideoProfile* pLocalProfile,
        IN VideoProfile* pPeerProfile, OUT VideoProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return;
    }

    if (pLocalProfile->IsAvpfSupported() && pPeerProfile->IsAvpfSupported())
    {
        pNegotiatedProfile->SetSupportAvpf(IMS_TRUE);
    }

    pNegotiatedProfile->SetSupportCapaNegoForAvpf(pPeerProfile->IsCapaNegoForAvpfSupported());

    IMS_TRACE_I("NegotiateAvpf(): Avpf[%d], CapaNegoForAVPF[%d]",
            pNegotiatedProfile->IsAvpfSupported(), pNegotiatedProfile->IsCapaNegoForAvpfSupported(),
            0);

    // Capability Negotiation for AVPF, SRTP
    if (pNegotiatedProfile->IsCapaNegoForAvpfSupported())
    {
        if (!MakeNegotiatedCapaNegoProfile(&(pLocalProfile->GetCapaNego()),
                    &(pPeerProfile->GetCapaNego()), &(pNegotiatedProfile->GetCapaNego())))
        {
            // Capa Nego Fail, return to original transport protocol.
            IMS_TRACE_D("NegotiateAvpf(): Capability Negotiation Fail Case", 0, 0, 0);
        }
        else
        {
            // Check Negotiated Transport Type
            IMS_BOOL bNegotiatedAVPF = IMS_FALSE;

            for (IMS_UINT32 i = 0; i < pNegotiatedProfile->GetCapaNego().GetMapTcap().GetSize();
                    i++)
            {
                AString strAttribute = pNegotiatedProfile->GetCapaNego().GetMapTcap().GetValueAt(i);

                if (strAttribute != IMS_NULL && strAttribute.Contains("AVPF"))
                {
                    bNegotiatedAVPF = IMS_TRUE;
                }
            }

            if (pNegotiatedProfile->IsCapaNegoForAvpfSupported())
            {
                pNegotiatedProfile->SetSupportAvpf(bNegotiatedAVPF);
            }
        }

        pNegotiatedProfile->GetCapaNego().GetMapTcap().Clear();
        pNegotiatedProfile->GetCapaNego().GetMapAcap().Clear();
    }
}

PRIVATE
void VideoProfileNegotiator::NegotiateTransportType(OUT VideoProfile* pNegotiatedProfile)
{
    if (pNegotiatedProfile == IMS_NULL)
    {
        return;
    }

    pNegotiatedProfile->SetTransportType(
            (pNegotiatedProfile->IsAvpfSupported()) ? "RTP/AVPF" : "RTP/AVP");

    IMS_TRACE_D("NegotiateTransportType(): Transport Type[%s]",
            pNegotiatedProfile->GetTransportType().GetStr(), 0, 0);
}

PRIVATE
IMS_BOOL VideoProfileNegotiator::NegotiatePayload(IN VideoProfile* pLocalProfile,
        IN VideoProfile* pPeerProfile, OUT VideoProfile* pNegotiatedProfile,
        OUT IMS_SINT32* nNegotiatedMaxFrameRate)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    VideoProfile::Payload* pLocalPayload = IMS_NULL;
    VideoProfile::Payload* pPeerPayload = IMS_NULL;
    VideoProfile::Payload* pTempPayload = IMS_NULL;
    VideoProfile::Payload* pMatchedPeerPayload = IMS_NULL;
    VideoProfile::Payload* pNegoPayload = IMS_NULL;
    std::shared_ptr<VideoProfile::VideoFmtp> pNegotiatedFmtp = IMS_NULL;

    IMS_TRACE_D("NegotiatePayload(): local payload count[%d], peer payload count[%d]",
            pLocalProfile->GetPayloadListSize(), pPeerProfile->GetPayloadListSize(), 0);

    for (IMS_UINT32 nPeerIndex = 0; nPeerIndex < pPeerProfile->GetPayloadListSize(); nPeerIndex++)
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

        for (IMS_UINT32 nLocalIndex = 0; nLocalIndex < pLocalProfile->GetPayloadList().GetSize();
                nLocalIndex++)
        {
            pLocalPayload = pLocalProfile->GetPayloadAt(nLocalIndex);

            if (pLocalPayload == IMS_NULL)
            {
                continue;
            }

            IMS_BOOL bVideoPayloadNegotiated = IMS_FALSE;

            if (pPeerPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264") &&
                    pLocalPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"))
            {
                pNegoPayload = new VideoProfile::Payload();
                bVideoPayloadNegotiated = NegotiateAvc(pLocalPayload, pPeerPayload, pNegoPayload,
                        nLocalIndex, pLocalProfile, &pTempPayload, &pMatchedPeerPayload);
            }
            else if (pPeerPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H265") &&
                    pLocalPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H265"))
            {
                pNegoPayload = new VideoProfile::Payload();
                bVideoPayloadNegotiated = NegotiateHevc(pLocalPayload, pPeerPayload, pNegoPayload,
                        pLocalProfile, pNegotiatedProfile, &pTempPayload, &pMatchedPeerPayload);
            }

            if (bVideoPayloadNegotiated && pNegoPayload != IMS_NULL)
            {
                pTempPayload = pLocalPayload;
                pMatchedPeerPayload = pPeerPayload;
                pNegotiatedFmtp = pNegoPayload->GetFmtp();

                if (pNegotiatedFmtp != IMS_NULL)
                {
                    pNegotiatedProfile->AddPayload(pNegoPayload);
                    break;
                }
            }
            else
            {
                delete pNegoPayload;
                pNegoPayload = IMS_NULL;
            }
        }
    }

    VideoProfile::Payload* pNegotiatedPayload = IMS_NULL;

    if (pNegotiatedProfile->GetPayloadListSize() > 0)
    {
        pNegotiatedPayload = pNegotiatedProfile->GetPayloadAt(0);
    }
    else if (pTempPayload != IMS_NULL && pMatchedPeerPayload != IMS_NULL)
    {
        pNegotiatedPayload = new VideoProfile::Payload();

        if (MakeNegotiatedPayload(pTempPayload, pMatchedPeerPayload, pNegotiatedPayload))
        {
            pNegotiatedProfile->AddPayload(pNegotiatedPayload);
            pNegotiatedProfile->SetNegotiatedPayloadIndex(0);
        }
        else
        {
            delete pNegotiatedPayload;
            IMS_TRACE_E(0, "NegotiatePayload(): fail to negotiate payload, size[%d]",
                    pNegotiatedProfile->GetPayloadListSize(), 0, 0);
            return IMS_FALSE;
        }
    }
    else
    {
        IMS_TRACE_E(0, "NegotiatePayload(): fail to negotiate payload, size[%d]",
                pNegotiatedProfile->GetPayloadListSize(), 0, 0);
        return IMS_FALSE;
    }

    NegotiateRtcpFb(pNegotiatedProfile, pLocalPayload, pPeerPayload, pNegotiatedPayload);

    if (SetNegotiatedPayloadIndex(pLocalProfile, pPeerProfile,
                FindPayloadIndexFromProfile(pLocalProfile, pTempPayload),
                FindPayloadIndexFromProfile(pPeerProfile, pMatchedPeerPayload)))
    {
        if (m_bIsOfferReceived)
        {
            NegotiatePayloadNumber(pLocalProfile, pPeerPayload);
        }
    }

    pNegotiatedFmtp = pNegotiatedPayload->GetFmtp();

    if (pNegotiatedFmtp != IMS_NULL)
    {
        SetMaxFrameRate(pNegotiatedFmtp->GetFramerate(), nNegotiatedMaxFrameRate);
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL VideoProfileNegotiator::NegotiateAvc(IN VideoProfile::Payload* pLocalPayload,
        IN VideoProfile::Payload* pPeerPayload, OUT VideoProfile::Payload* pNegoPayload,
        IN IMS_UINT32 nLocalIndex, IN VideoProfile* pLocalProfile,
        OUT VideoProfile::Payload** pTempPayload, OUT VideoProfile::Payload** pMatchedPeerPayload)
{
    if (pLocalPayload == IMS_NULL || pPeerPayload == IMS_NULL || pNegoPayload == IMS_NULL ||
            pLocalProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    auto pLocalFmtp = std::static_pointer_cast<VideoProfile::AvcFmtp>(pLocalPayload->GetFmtp());
    auto pPeerFmtp = std::static_pointer_cast<VideoProfile::AvcFmtp>(pPeerPayload->GetFmtp());

    if (pLocalFmtp == IMS_NULL || pPeerFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("NegotiateAvc(): Local profileLevelID[%s] <-> Peer profileLevelID[%s]",
            pLocalFmtp->GetProfileLevelId().GetStr(), pPeerFmtp->GetProfileLevelId().GetStr(), 0);
    IMS_TRACE_D("NegotiateAvc(): Local Level[%d] <-> Peer Level[%d]", pLocalFmtp->GetLevel(),
            pPeerFmtp->GetLevel(), 0);
    IMS_TRACE_D("NegotiateAvc(): Local Profile[%d] <-> Peer Profile[%d]", pLocalFmtp->GetProfile(),
            pPeerFmtp->GetProfile(), 0);

    // reject unmatched profile or reject higher level
    if (pLocalFmtp->GetProfile() != pPeerFmtp->GetProfile() ||
            pLocalFmtp->GetLevel() < pPeerFmtp->GetLevel())
    {
        if (*pTempPayload == IMS_NULL)
        {
            IMS_TRACE_D("NegotiateAvc(): Accept temporary payload[%d], profileLevelID[%s]",
                    pLocalPayload->GetRtpMap().GetPayloadNumber(),
                    pLocalFmtp->GetProfileLevelId().GetStr(), 0);
            *pTempPayload = pLocalPayload;
            *pMatchedPeerPayload = pPeerPayload;
        }

        return IMS_FALSE;
    }
    else
    {
        if (pLocalFmtp->GetLevel() != pPeerFmtp->GetLevel())
        {
            IMS_BOOL bFoundPayload = IMS_FALSE;

            for (IMS_UINT32 nIndex = nLocalIndex;
                    nIndex < pLocalProfile->GetPayloadList().GetSize(); nIndex++)
            {
                // if find matching level FMTP, skip unmatched level payload
                VideoProfile::Payload* pPotentialPayload = pLocalProfile->GetPayloadAt(nIndex);

                if (pPotentialPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"))
                {
                    auto pPotentialFmtp = std::static_pointer_cast<VideoProfile::AvcFmtp>(
                            pPotentialPayload->GetFmtp());

                    // check level and payload
                    if (pPotentialFmtp->GetLevel() == pPeerFmtp->GetLevel() &&
                            pPotentialFmtp->GetResolution() == pPeerFmtp->GetResolution() &&
                            pPeerFmtp->GetResolution() != VIDEO_RESOLUTION_NOT_USED)
                    {
                        bFoundPayload = IMS_TRUE;
                        break;
                    }
                }
            }

            if (bFoundPayload)
            {
                return IMS_FALSE;
            }
        }
    }

    if (pLocalFmtp->GetResolution() != pPeerFmtp->GetResolution())
    {
        IMS_TRACE_D("NegotiateAvc(): NOT MATCHED Avc Resolution[%d]<->[%d]",
                pLocalFmtp->GetResolution(), pPeerFmtp->GetResolution(), 0);

        // Keep 1st payload(resolution mismatched) to be used
        // when no strictly matched resolution is found
        if (*pTempPayload == IMS_NULL)
        {
            IMS_TRACE_D("NegotiateAvc(): Keep profileLevelID[%s]",
                    pLocalFmtp->GetProfileLevelId().GetStr(), 0, 0);
            *pTempPayload = pLocalPayload;
            *pMatchedPeerPayload = pPeerPayload;
        }

        return IMS_FALSE;
    }

    IMS_TRACE_D("NegotiateAvc(): Matched payload found, Profile[%d], Level[%d], Resolution[%d]",
            pLocalFmtp->GetProfile(), pLocalFmtp->GetLevel(), pLocalFmtp->GetResolution());

    return MakeNegotiatedPayload(pLocalPayload, pPeerPayload, pNegoPayload);
}

PRIVATE
IMS_BOOL VideoProfileNegotiator::NegotiateHevc(IN VideoProfile::Payload* pLocalPayload,
        IN VideoProfile::Payload* pPeerPayload, OUT VideoProfile::Payload* pNegoPayload,
        IN VideoProfile* pLocalProfile, OUT VideoProfile* pNegotiatedProfile,
        OUT VideoProfile::Payload** pTempPayload, OUT VideoProfile::Payload** pMatchedPeerPayload)
{
    if (pLocalPayload == IMS_NULL || pPeerPayload == IMS_NULL || pNegoPayload == IMS_NULL ||
            pLocalProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    auto pLocalFmtp = std::static_pointer_cast<VideoProfile::HevcFmtp>(pLocalPayload->GetFmtp());
    auto pPeerFmtp = std::static_pointer_cast<VideoProfile::HevcFmtp>(pPeerPayload->GetFmtp());

    if (pLocalFmtp == IMS_NULL || pPeerFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("NegotiateHevc(): Local profileId[%d] <-> Peer profileId[%d]",
            pLocalFmtp->GetProfile(), pPeerFmtp->GetProfile(), 0);

    // same level is adapt first, reject higher level
    if (pLocalFmtp->GetLevel() < pPeerFmtp->GetLevel())
    {
        IMS_TRACE_D("NegotiateHevc(): NOT MATCHED HEVC Local Level[%d] <-> Peer Level[%d]",
                pLocalFmtp->GetLevel(), pPeerFmtp->GetLevel(), 0);

        if (*pTempPayload == IMS_NULL)
        {
            IMS_TRACE_D("NegotiateHevc(): Accept Highest Temp Src profileID[%d]",
                    pLocalFmtp->GetProfile(), 0, 0);
            *pTempPayload = pLocalPayload;
            *pMatchedPeerPayload = pPeerPayload;
        }

        return IMS_FALSE;
    }

    if (pPeerFmtp->GetResolution() == VIDEO_RESOLUTION_NOT_USED)
    {
        VIDEO_RESOLUTION eTempResolution =
                GetNegotiatedResolution((pNegotiatedProfile->GetNegotiatedPayloadIndex() > 0)
                                ? pNegotiatedProfile->GetPayloadAt(
                                          pNegotiatedProfile->GetNegotiatedPayloadIndex())
                                : pNegotiatedProfile->GetPayloadAt(0));

        if (eTempResolution != VIDEO_RESOLUTION_NOT_USED &&
                eTempResolution != VIDEO_RESOLUTION_INVALID)
        {
            IMS_TRACE_D("NegotiateHevc(): Far Resolution is not specified[%d] -> Temp use Prev. "
                        "Negotiated Resolution[%d]",
                    pPeerFmtp->GetResolution(), eTempResolution, 0);
            pPeerFmtp->SetResolution(eTempResolution);
        }
        else
        {
            IMS_TRACE_D("NegotiateHevc(): Far Resolution is not specified[%d] -> Temp use Src "
                        "Resolution[%d]",
                    pPeerFmtp->GetResolution(), pPeerFmtp->GetResolution(), 0);
            pPeerFmtp->SetResolution(pLocalFmtp->GetResolution());
        }
    }

    if (pLocalFmtp->GetResolution() != pPeerFmtp->GetResolution())
    {
        IMS_TRACE_D("NegotiateHevc(): NOT MATCHED HEVC Resolution [%d]<->[%d]",
                pLocalFmtp->GetResolution(), pPeerFmtp->GetResolution(), 0);

        if (*pTempPayload == IMS_NULL)
        {
            IMS_TRACE_D("NegotiateHevc(): keep 1st payload profile[%d]", pLocalFmtp->GetProfile(),
                    0, 0);

            *pTempPayload = pLocalPayload;
            *pMatchedPeerPayload = pPeerPayload;
        }

        return IMS_FALSE;
    }

    IMS_TRACE_D("NegotiateHevc(): Matched payload found, Profile[%d], Level[%d], Resolution[%d]",
            pLocalFmtp->GetProfile(), pLocalFmtp->GetLevel(), pLocalFmtp->GetResolution());

    return MakeNegotiatedPayload(pLocalPayload, pPeerPayload, pNegoPayload);
}

PRIVATE
void VideoProfileNegotiator::NegotiateRtcpFb(OUT VideoProfile* pNegotiatedProfile,
        IN VideoProfile::Payload* pLocalPayload, IN VideoProfile::Payload* pPeerPayload,
        OUT VideoProfile::Payload* pNegoPayload)
{
    if (pNegotiatedProfile == IMS_NULL || pLocalPayload == IMS_NULL || pPeerPayload == IMS_NULL ||
            pNegoPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateRtcpFb(): invalid arguments", 0, 0, 0);
        return;
    }

    if (!pNegotiatedProfile->IsAvpfSupported())
    {
        IMS_TRACE_D("NegotiateRtcpFb(): avpf unsupported", 0, 0, 0);
        return;
    }

    if (pLocalPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"))
    {
        if (pLocalPayload->GetRtcpFbAttr().IsTrrSupported() &&
                pPeerPayload->GetRtcpFbAttr().IsTrrSupported())
        {
            pNegoPayload->GetRtcpFbAttr().SetTrrInt(pPeerPayload->GetRtcpFbAttr().GetTrrInt());
            pNegoPayload->GetRtcpFbAttr().SetTrrSupported(IMS_TRUE);
        }
    }

    if (pLocalPayload->GetRtcpFbAttr().IsNackSupported() &&
            pPeerPayload->GetRtcpFbAttr().IsNackSupported())
    {
        pNegoPayload->GetRtcpFbAttr().SetNackSupported(IMS_TRUE);
    }

    if (pLocalPayload->GetRtcpFbAttr().IsTmmbrSupported() &&
            pPeerPayload->GetRtcpFbAttr().IsTmmbrSupported())
    {
        pNegoPayload->GetRtcpFbAttr().SetTmmbrSupported(IMS_TRUE);
    }

    if (pLocalPayload->GetRtcpFbAttr().IsPliSupported() &&
            pPeerPayload->GetRtcpFbAttr().IsPliSupported())
    {
        pNegoPayload->GetRtcpFbAttr().SetPliSupported(IMS_TRUE);
    }

    if (pLocalPayload->GetRtcpFbAttr().IsFirSupported() &&
            pPeerPayload->GetRtcpFbAttr().IsFirSupported())
    {
        pNegoPayload->GetRtcpFbAttr().SetFirSupported(IMS_TRUE);
    }

    IMS_TRACE_D("NegotiateRtcpFb(): AVPF supported. NACK[%d], TMMBR[%d], PLI[%d]",
            pNegoPayload->GetRtcpFbAttr().IsNackSupported(),
            pNegoPayload->GetRtcpFbAttr().IsTmmbrSupported(),
            pNegoPayload->GetRtcpFbAttr().IsPliSupported());
    IMS_TRACE_D("NegotiateRtcpFb(): AVPF supported. FIR[%d], TRR[%d], Trr-int[%d]",
            pNegoPayload->GetRtcpFbAttr().IsFirSupported(),
            pNegoPayload->GetRtcpFbAttr().IsTrrSupported(),
            pNegoPayload->GetRtcpFbAttr().GetTrrInt());
}

PRIVATE
IMS_BOOL VideoProfileNegotiator::SetNegotiatedPayloadIndex(OUT VideoProfile* pLocalProfile,
        OUT VideoProfile* pPeerProfile, IN IMS_SINT32 nLocalIndex, IN IMS_SINT32 nPeerIndex)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetNegotiatedPayloadIndex(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pPeerProfile->GetNegotiatedPayloadIndex() == -1)
    {
        pLocalProfile->SetNegotiatedPayloadIndex(nLocalIndex);
        pPeerProfile->SetNegotiatedPayloadIndex(nPeerIndex);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void VideoProfileNegotiator::NegotiatePayloadNumber(
        OUT VideoProfile* pLocalProfile, IN VideoProfile::Payload* pPeerPayload)
{
    if (pLocalProfile == IMS_NULL || pPeerPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiatePayloadNumber(): invalid arguments", 0, 0, 0);
        return;
    }

    if (pLocalProfile->GetNegotiatedPayloadIndex() != -1)
    {
        VideoProfile::Payload* pNegotiatedLocalPayload =
                pLocalProfile->GetPayloadAt(pLocalProfile->GetNegotiatedPayloadIndex());

        pNegotiatedLocalPayload->GetRtpMap().SetPayloadNumber(
                pPeerPayload->GetRtpMap().GetPayloadNumber());
    }
}

PRIVATE
void VideoProfileNegotiator::NegotiateCvo(IN VideoProfile* pLocalProfile,
        IN VideoProfile* pPeerProfile, OUT VideoProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateCvo(): invalid arguments", 0, 0, 0);
        return;
    }

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

    IMS_TRACE_D("NegotiateCvo(): CVO Id[%d]", pNegotiatedProfile->GetCvoId(), 0, 0);
}

PRIVATE
void VideoProfileNegotiator::SetMaxFrameRate(
        IN IMS_SINT32 nFrameRate, OUT IMS_SINT32* nNegotiatedMaxFrameRate)
{
    if (nFrameRate > *nNegotiatedMaxFrameRate)
    {
        *nNegotiatedMaxFrameRate = nFrameRate;
    }
}

PRIVATE
IMS_SINT32 VideoProfileNegotiator::FindPayloadIndexFromProfile(
        IN VideoProfile* pProfile, IN const VideoProfile::Payload* pPayload)
{
    if (pProfile == IMS_NULL || pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "FindPayloadIndexFromProfile(): invalid arguments", 0, 0, 0);
        return -1;
    }

    // find the index of negotiated payload
    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        const VideoProfile::Payload* pComparedPayload = pProfile->GetPayloadAt(i);
        if (pComparedPayload == IMS_NULL)
            continue;

        if (pComparedPayload == pPayload)
        {
            IMS_TRACE_D("FindPayloadIndexFromProfile(): FindIndex[%d]", i, 0, 0);
            return i;
        }
    }

    return -1;
}

PUBLIC
MEDIA_DIRECTION VideoProfileNegotiator::UpdateDirectionToMine(IN MEDIA_DIRECTION ePeerDirection,
        IN MEDIA_DIRECTION eLocalDirection, IN IMS_BOOL bIsMtCase)
{
    IMS_TRACE_D("UpdateDirectionToMine(): PeerDirection[%d], LocalDirection[%d], IsMtCase[%d]",
            ePeerDirection, eLocalDirection, bIsMtCase);
    MEDIA_DIRECTION eNegotiatedDir = MEDIA_DIRECTION_INVALID;

    switch (ePeerDirection)
    {
        case MEDIA_DIRECTION_INACTIVE:
        case MEDIA_DIRECTION_SEND_RECEIVE:
            eNegotiatedDir = ePeerDirection;
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

    if (!bIsMtCase)
    {
        // direction check strictly
        if ((eLocalDirection == MEDIA_DIRECTION_SEND &&
                    (ePeerDirection == MEDIA_DIRECTION_SEND ||
                            ePeerDirection == MEDIA_DIRECTION_SEND_RECEIVE)) ||
                (eLocalDirection == MEDIA_DIRECTION_RECEIVE &&
                        (ePeerDirection == MEDIA_DIRECTION_RECEIVE ||
                                ePeerDirection == MEDIA_DIRECTION_SEND_RECEIVE)) ||
                (eLocalDirection == MEDIA_DIRECTION_INACTIVE &&
                        (ePeerDirection != MEDIA_DIRECTION_INACTIVE)))
        {
            return MEDIA_DIRECTION_INVALID;
        }
    }
    return eNegotiatedDir;
}

PRIVATE IMS_BOOL VideoProfileNegotiator::MakeNegotiatedCapaNegoProfile(
        IN VideoProfile::CapaNego* pLocalCapaNego, IN VideoProfile::CapaNego* pPeerCapaNego,
        OUT VideoProfile::CapaNego* pNegotiatedCapaNego)
{
    if (pLocalCapaNego == IMS_NULL || pPeerCapaNego == IMS_NULL || pNegotiatedCapaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "MakeNegotiatedCapaNego(): invalid argument, %" PFLS_x " %" PFLS_x,
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
        if (mapPeerTCap.IsEmpty())
        {
            pNegotiatedCapaNego->GetMapTcap() = mapLocalTCap;
        }

        IMS_TRACE_I("MakeNegotiatedCapaNego(): ACFG[%s]", pPeerCapaNego->GetAcfg().GetStr(), 0, 0);
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

                if (lstSplitEquals.GetAt(0).Equals('t') && lstSplitEquals.GetSize() > 1)
                {
                    // compare transport capa
                    AString strTmp = mapPeerTCap.GetValue(lstSplitEquals.GetAt(1).ToInt32());

                    for (k = 0; k < mapLocalTCap.GetSize(); k++)
                    {
                        if (strTmp.Equals(mapLocalTCap.GetValueAt(k)))
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
                        IMS_TRACE_I(
                                "MakeNegotiatedCapaNego(): does not match transport capa - PCFG "
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

                if (lstSplitEquals.GetAt(0).Equals('a') && lstSplitEquals.GetSize() > 1)
                {
                    IMS_UINT32 cnt = 0;
                    // compare Attribute capa
                    AString strTmp = lstSplitEquals.GetAt(1);  // tmp = "1,2,3,4"

                    // attribute comma parsing..
                    ImsList<AString> lstSplitComma = strTmp.Split(',');
                    IMS_TRACE_I("MakeNegotiatedCapaNego(): attribute size[%d]",
                            lstSplitComma.GetSize(), 0, 0);

                    if (lstSplitComma.GetSize() == 0)
                        continue;

                    // check attribute capa negotiation
                    for (k = 0; k < lstSplitComma.GetSize(); k++)
                    {
                        AString strDestAttributeCapa =
                                mapPeerACap.GetValue(lstSplitComma.GetAt(k).ToInt32());
                        IMS_TRACE_I("MakeNegotiatedCapaNego(): strDestAttributeCapa [%s]",
                                strDestAttributeCapa.GetStr(), 0, 0);

                        if (strDestAttributeCapa.Contains("trr-int") ||
                                strDestAttributeCapa.Contains("nack"))
                        {
                            cnt++;
                            pNegotiatedCapaNego->GetMapAcap().Add(
                                    lstSplitComma.GetAt(k).ToInt32(), strDestAttributeCapa);
                        }
                        else if (strDestAttributeCapa.Contains("ccm"))
                        {
                            if (strDestAttributeCapa.Contains("fir") ||
                                    strDestAttributeCapa.Contains("tmmbr"))
                            {
                                cnt++;
                                pNegotiatedCapaNego->GetMapAcap().Add(
                                        lstSplitComma.GetAt(k).ToInt32(), strDestAttributeCapa);
                            }
                        }
                        else if (strDestAttributeCapa.Contains("crypto"))
                        {
                            // crypto attribute negotiate only srtp profile type
                            ImsList<AString> lstSrcCryptoAttribute =
                                    mapLocalACap.GetValueAt(l).Split(' ');
                            ImsList<AString> lstDestCryptoAttribute =
                                    strDestAttributeCapa.Split(' ');
                            if (lstDestCryptoAttribute.GetAt(1).Equals(
                                        lstSrcCryptoAttribute.GetAt(1)))
                            {
                                cnt++;
                                pNegotiatedCapaNego->GetMapAcap().Add(
                                        lstSplitComma.GetAt(k).ToInt32(), strDestAttributeCapa);

                                IMS_TRACE_I("MakeNegotiatedCapaNego(): strDestAttributeCapa.Equals "
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
                                "MakeNegotiatedCapaNego(): capa nego success, cnt[%d]", cnt, 0, 0);
                        bPCFGSupportable = IMS_TRUE;
                        break;
                    }
                }
            }
        }

        // check capa nego success
        if (bPCFGSupportable)
        {
            pNegotiatedCapaNego->GetAcfg().Sprintf("%s", strPCFGline.GetStr());
            IMS_TRACE_I("MakeNegotiatedCapaNego(): UE support capa nego- ACFG [%s]",
                    strPCFGline.GetStr(), 0, 0);
            // strAcfg value available, if capa nego success.
            ret = IMS_TRUE;
            break;
        }
        else  // capa nego dose not succsee case//
        {
            // clear saved negotiatedCapaNego imfo.
            IMS_TRACE_I("MakeNegotiatedCapaNego(): capa nego does not success pcfg[%d]", i, 0, 0);
            pNegotiatedCapaNego->GetMapTcap().Clear();
            pNegotiatedCapaNego->GetMapAcap().Clear();
        }
    }

    return ret;
}

PUBLIC
VIDEO_RESOLUTION VideoProfileNegotiator::GetNegotiatedResolution(
        IN MediaBaseProfile::BasePayload* pPayload)
{
    if (pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetNegotiatedResolution(): invalid payload", 0, 0, 0);
        return VIDEO_RESOLUTION_INVALID;
    }

    if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264") ||
            pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H265"))
    {
        std::shared_ptr<VideoProfile::VideoFmtp> pFmtp =
                static_cast<VideoProfile::Payload*>(pPayload)->GetFmtp();

        if (pFmtp != IMS_NULL)
        {
            return pFmtp->GetResolution();
        }
    }

    return VIDEO_RESOLUTION_NOT_USED;
}

PRIVATE IMS_BOOL VideoProfileNegotiator::MakeNegotiatedPayload(
        IN VideoProfile::Payload* pLocalPayload, IN VideoProfile::Payload* pPeerPayload,
        OUT VideoProfile::Payload* pNegoPayload)
{
    if (pLocalPayload == IMS_NULL || pPeerPayload == IMS_NULL || pNegoPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "MakeNegotiatedPayload(): invalid payload", 0, 0, 0);
        return IMS_FALSE;
    }

    *pNegoPayload = *pLocalPayload;
    pNegoPayload->GetRtpMap().SetPayloadNumber(pPeerPayload->GetRtpMap().GetPayloadNumber());

    if (pLocalPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"))
    {
        NegotiateVideoFmtp(std::static_pointer_cast<VideoProfile::AvcFmtp>(pNegoPayload->GetFmtp()),
                std::static_pointer_cast<VideoProfile::AvcFmtp>(pPeerPayload->GetFmtp()));
    }
    else if (pLocalPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H265"))
    {
        NegotiateVideoFmtp(
                std::static_pointer_cast<VideoProfile::HevcFmtp>(pNegoPayload->GetFmtp()),
                std::static_pointer_cast<VideoProfile::HevcFmtp>(pPeerPayload->GetFmtp()));
    }

    return IMS_TRUE;
}

PRIVATE IMS_BOOL VideoProfileNegotiator::MakeNegotiatedBandwidth(
        IN const VideoConfiguration* pConfig, IN VideoProfile* pLocalProfile,
        IN VideoProfile* pPeerProfile, IN IMS_BOOL bIsOfferReceived,
        OUT VideoProfile* pNegotiatedProfile)
{
    // Negotiate AS: Choose the less value if both are positive
    IMS_SINT32 localAs = pLocalProfile->GetBandwidthAs();
    IMS_SINT32 peerAs = pPeerProfile->GetBandwidthAs();

    if (localAs > 0 && peerAs > 0)
    {
        pNegotiatedProfile->SetBandwidthAs(std::min(localAs, peerAs));
    }
    else
    {
        pNegotiatedProfile->SetBandwidthAs(std::max(localAs, peerAs));
    }

    // Negotiate RS/RR Value
    if (!bIsOfferReceived)
    {
        // Exception Handling (b=RS/RR line is not included in Answer SDP)
        if (pNegotiatedProfile->GetBandwidthRs() < 0 || pNegotiatedProfile->GetBandwidthRr() < 0)
        {
            pNegotiatedProfile->SetBandwidthRs(pLocalProfile->GetBandwidthRs());
            pNegotiatedProfile->SetBandwidthRr(pLocalProfile->GetBandwidthRr());

            IMS_TRACE_D("MakeNegotiatedBandwidth(): Negotiated Profile AS[%d] RS[%d] RR[%d]",
                    pNegotiatedProfile->GetBandwidthAs(), pNegotiatedProfile->GetBandwidthRs(),
                    pNegotiatedProfile->GetBandwidthRr());

            return IMS_TRUE;
        }

        // Normal Case
        // if RS/RR is used for RTCP remote value
        if (pConfig->GetBandwidthNegoOption() == MediaConfiguration::BW_OPTION_REMOTE_VALUE)
        {
            pNegotiatedProfile->SetBandwidthRs(pPeerProfile->GetBandwidthRs());
            pNegotiatedProfile->SetBandwidthRr(pPeerProfile->GetBandwidthRr());
        }
        else
        {
            pNegotiatedProfile->SetBandwidthRs(pLocalProfile->GetBandwidthRs());
            pNegotiatedProfile->SetBandwidthRr(pLocalProfile->GetBandwidthRr());
        }
    }
    else
    {
        // Set RS/RR Value
        if (pPeerProfile->GetDirection() != MEDIA_DIRECTION_SEND_RECEIVE &&
                pPeerProfile->GetDirection() != MEDIA_DIRECTION_RECEIVE &&
                pPeerProfile->GetDirection() != MEDIA_DIRECTION_SEND)
        {
            // Hold Case
            MediaProfileUtil::SetRtcpRsRr(pNegotiatedProfile, pConfig, IMS_FALSE);
        }
        else
        {
            // Active Call Case
            // Exception Handling (b=RS/RR line is not included in Answer SDP)
            if (pNegotiatedProfile->GetBandwidthRs() < 0 ||
                    pNegotiatedProfile->GetBandwidthRr() < 0)
            {
                pNegotiatedProfile->SetBandwidthRs(pLocalProfile->GetBandwidthRs());
                pNegotiatedProfile->SetBandwidthRr(pLocalProfile->GetBandwidthRr());

                IMS_TRACE_D("MakeNegotiatedBandwidth(): Negotiated Profile AS[%d] RS[%d] RR[%d]",
                        pNegotiatedProfile->GetBandwidthAs(), pNegotiatedProfile->GetBandwidthRs(),
                        pNegotiatedProfile->GetBandwidthRr());

                return IMS_TRUE;
            }

            // 3.2.2 Normal Case
            if (pConfig->GetBandwidthNegoOption() == MediaConfiguration::BW_OPTION_REMOTE_VALUE)
            {
                // only use rtcp when rtcp state is enable
                pNegotiatedProfile->SetBandwidthRs(pPeerProfile->GetBandwidthRs());
                pNegotiatedProfile->SetBandwidthRr(pPeerProfile->GetBandwidthRr());
            }
            else
            {
                // default case (Local values ​​are used instead of remote values.)
                pNegotiatedProfile->SetBandwidthRs(pLocalProfile->GetBandwidthRs());
                pNegotiatedProfile->SetBandwidthRr(pLocalProfile->GetBandwidthRr());
            }
        }
    }

    IMS_TRACE_D("MakeNegotiatedBandwidth(): Negotiated Profile AS[%d] RS[%d] RR[%d]",
            pNegotiatedProfile->GetBandwidthAs(), pNegotiatedProfile->GetBandwidthRs(),
            pNegotiatedProfile->GetBandwidthRr());

    return IMS_TRUE;
}
