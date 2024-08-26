// Copyright 2024 Google LLC

#include "ServiceTrace.h"
#include "offeranswer/SdpAvCodec.h"
#include "offeranswer/SdpMediaFormatParameter.h"
#include "offeranswer/SdpRtcpFeedback.h"

#include "config/VideoConfiguration.h"
#include "video/VideoNegoAvc.h"
#include "video/VideoNegoHevc.h"
#include "video/VideoSdpNegotiator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC VideoSdpNegotiator::VideoSdpNegotiator() :
        SdpNegotiator(MEDIA_TYPE_VIDEO)
{
    IMS_TRACE_I("+VideoSdpNegotiator()", 0, 0, 0);
}

PUBLIC VIRTUAL VideoSdpNegotiator::~VideoSdpNegotiator()
{
    IMS_TRACE_I("~VideoSdpNegotiator()", 0, 0, 0);
}

PUBLIC IMS_BOOL VideoSdpNegotiator::Negotiate(IN VideoProfile* pLocalProfile,
        IN VideoProfile* pPeerProfile, IN IMS_BOOL bIsOfferReceived,
        OUT VideoProfile* pNegotiatedProfile, IN MediaConfiguration* pConfig)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL ||
            pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "Negotiate() invalid argument", 0, 0, 0);
        return IMS_FALSE;
    }

    m_bIsOfferReceived = bIsOfferReceived;

    IMS_TRACE_I("Negotiate() - IsOfferReceived[%d]", m_bIsOfferReceived, 0, 0);

    IMS_BOOL ret = IMS_FALSE;

    if (NegotiateIpPort(pLocalProfile, pPeerProfile, pNegotiatedProfile) != IMS_TRUE)
    {
        ResetNegotiatedProfile(pLocalProfile, pNegotiatedProfile);
        return IMS_TRUE;
    }

    // Setting profile type
    if (pLocalProfile->IsAvpfSupported() == IMS_TRUE && pPeerProfile->IsAvpfSupported() == IMS_TRUE)
    {
        pNegotiatedProfile->SetSupportAvpf(IMS_TRUE);
    }

    pNegotiatedProfile->SetSupportCapaNegoForAvpf(pPeerProfile->IsCapaNegoForAvpfSupported());

    IMS_TRACE_I("Negotiate() - PeerProfile: CapaNegoForAVPF[%d], Avpf[%d]",
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
            IMS_TRACE_D("Negotiate() - Capability Negotiation Fail Case", 0, 0, 0);
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

    IMS_TRACE_D("Negotiate() - AVPF enable[%d], Transport Type[%s]",
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

                    IMS_TRACE_D("Negotiate() profileLevelID[%s]<->profileLevelID[%s]",
                            pLocalFmtp->GetProfileLevelId().GetStr(),
                            pPeerFmtp->GetProfileLevelId().GetStr(), 0);

                    IMS_TRACE_D("Negotiate() Level[%d]<->Level[%d]", pLocalFmtp->GetLevel(),
                            pPeerFmtp->GetLevel(), 0);
                    IMS_TRACE_D("Negotiate() Profile[%d]<->Profile[%d]", pLocalFmtp->GetProfile(),
                            pPeerFmtp->GetProfile(), 0);

                    // same level is adapt first, reject higher level
                    if (pLocalFmtp->GetLevel() < pPeerFmtp->GetLevel())
                    {
                        IMS_TRACE_D("Negotiate() NOT MATCHED AVC Level[%d]<->[%d]",
                                pLocalFmtp->GetLevel(), pPeerFmtp->GetLevel(), 0);

                        if (pTmpPayload == IMS_NULL)
                        {
                            IMS_TRACE_D("Negotiate() Accept Highest Temp Src \
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
                        VIDEO_RESOLUTION eTempResolution = GetNegotiatedResolution(
                                (pNegotiatedProfile->GetNegotiatedPayloadIndex() > 0)
                                        ? pNegotiatedProfile->GetPayloadAt(
                                                  pNegotiatedProfile->GetNegotiatedPayloadIndex())
                                        : pNegotiatedProfile->GetPayloadAt(0));

                        if (eTempResolution != VIDEO_RESOLUTION_NOT_USED &&
                                eTempResolution != VIDEO_RESOLUTION_INVALID)
                        {
                            IMS_TRACE_D("Negotiate() - Far Resolution is not \
                                    specified[%d] -> Temp use Prev. Negotiated Resolution[%d]",
                                    pPeerFmtp->GetResolution(), eTempResolution, 0);
                            pPeerFmtp->SetResolution(eTempResolution);
                        }
                        else
                        {
                            IMS_TRACE_D("Negotiate() - Far Resolution is not \
                                    specified[%d] -> Temp use Src Resolution[%d]",
                                    pPeerFmtp->GetResolution(), pPeerFmtp->GetResolution(), 0);

                            pPeerFmtp->SetResolution(pLocalFmtp->GetResolution());
                        }
                    }

                    if (pLocalFmtp->GetResolution() != pPeerFmtp->GetResolution())
                    {
                        IMS_TRACE_D("Negotiate() NOT MATCHED Avc Resolution[%d]<->[%d]",
                                pLocalFmtp->GetResolution(), pPeerFmtp->GetResolution(), 0);

                        if (pLocalFmtp->GetLevel() >= pPeerFmtp->GetLevel())
                        {
                            // Keep 1st payload(resolution mismatched) to be used
                            // when no strictly matched resolution is found
                            if (pTmpPayload == IMS_NULL)
                            {
                                IMS_TRACE_D("Negotiate() - Keep profileLevelID[%s]",
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
                                IMS_TRACE_D("Negotiate() - Keep dynamic resolution \
                                        profileLevelID[%s]",
                                        pLocalFmtp->GetProfileLevelId().GetStr(), 0, 0);
                                pTmpPayload = pLocalPayload;
                                pMatchedPeerPayload = pPeerPayload;
                            }
                        }
                        continue;
                    }

                    IMS_TRACE_D("Negotiate() - Matched payload found, \
                            Profile[%d], Level[%d], Resolution[%d]",
                            pLocalFmtp->GetProfile(), pLocalFmtp->GetLevel(),
                            pLocalFmtp->GetResolution());

                    // make nego payload
                    VideoProfile::Payload* pNegoPayload = new VideoProfile::Payload();

                    if (MakeNegotiatedPayload(pLocalPayload, pPeerPayload, pNegoPayload) ==
                            IMS_FALSE)
                    {
                        IMS_TRACE_E(0, "Negotiate() - Cannot Make Nego payload", 0, 0, 0);
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

                        IMS_TRACE_D("Negotiate() - AVPF supported. \
                                bNACK[%d], bTMMBR[%d], bPLI[%d]",
                                pNegoPayload->GetRtcpFbAttr().IsNackSupported(),
                                pNegoPayload->GetRtcpFbAttr().IsTmmbrSupported(),
                                pNegoPayload->GetRtcpFbAttr().IsPliSupported());
                        IMS_TRACE_D("Negotiate() - AVPF supported. \
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
                        if (m_bIsOfferReceived == IMS_TRUE &&
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

                    IMS_TRACE_D("Negotiate() - profileId[%d]<->profileId[%d]",
                            pLocalFmtp->GetProfile(), pPeerFmtp->GetProfile(), 0);

                    // same level is adapt first, reject higher level
                    if (pLocalFmtp->GetLevel() < pPeerFmtp->GetLevel())
                    {
                        IMS_TRACE_D("Negotiate() - NOT MATCHED HEVC Level[%d]<->[%d]",
                                pLocalFmtp->GetLevel(), pPeerFmtp->GetLevel(), 0);

                        if (pTmpPayload == IMS_NULL)
                        {
                            IMS_TRACE_D("Negotiate() - Accept Highest Temp Src \
                                    profileID[%d]",
                                    pLocalFmtp->GetProfile(), 0, 0);
                            pTmpPayload = pLocalPayload;
                            pMatchedPeerPayload = pPeerPayload;
                        }
                        continue;
                    }

                    if (pPeerFmtp->GetResolution() == VIDEO_RESOLUTION_NOT_USED)
                    {
                        VIDEO_RESOLUTION eTempResolution = GetNegotiatedResolution(
                                (pNegotiatedProfile->GetNegotiatedPayloadIndex() > 0)
                                        ? pNegotiatedProfile->GetPayloadAt(
                                                  pNegotiatedProfile->GetNegotiatedPayloadIndex())
                                        : pNegotiatedProfile->GetPayloadAt(0));

                        if (eTempResolution != VIDEO_RESOLUTION_NOT_USED &&
                                eTempResolution != VIDEO_RESOLUTION_INVALID)
                        {
                            IMS_TRACE_D("Negotiate() - Far Resolution is not \
                                    specified[%d] -> Temp use Prev. Negotiated Resolution[%d]",
                                    pPeerFmtp->GetResolution(), eTempResolution, 0);
                            pPeerFmtp->SetResolution(eTempResolution);
                        }
                        else
                        {
                            IMS_TRACE_D("Negotiate() - Far Resolution is not \
                                    specified[%d] -> Temp use Src Resolution[%d]",
                                    pPeerFmtp->GetResolution(), pPeerFmtp->GetResolution(), 0);
                            pPeerFmtp->SetResolution(pLocalFmtp->GetResolution());
                        }
                    }

                    if (pLocalFmtp->GetResolution() != pPeerFmtp->GetResolution())
                    {
                        IMS_TRACE_D("Negotiate() - NOT MATCHED HEVC Resolution\
                                [%d]<->[%d]",
                                pLocalFmtp->GetResolution(), pPeerFmtp->GetResolution(), 0);

                        if (pLocalFmtp->GetLevel() >= pPeerFmtp->GetLevel())
                        {
                            // Keep 1st payload(resolution mismatched) to be used
                            // when no strictly matched resolution is found
                            if (pTmpPayload == IMS_NULL)
                            {
                                IMS_TRACE_D("Negotiate() - Keep profile[%d]",
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
                                IMS_TRACE_D("Negotiate() - Keep dynamic resolution \
                                        profileID[%d]",
                                        pLocalFmtp->GetProfile(), 0, 0);
                                pTmpPayload = pLocalPayload;
                                pMatchedPeerPayload = pPeerPayload;
                            }
                        }
                        continue;
                    }

                    IMS_TRACE_D("Negotiate() - Matched payload found, \
                            Profile[%d], Level[%d], Resolution[%d]",
                            pLocalFmtp->GetProfile(), pLocalFmtp->GetLevel(),
                            pLocalFmtp->GetResolution());

                    // make nego payload
                    VideoProfile::Payload* pNegoPayload = new VideoProfile::Payload();

                    if (MakeNegotiatedPayload(pLocalPayload, pPeerPayload, pNegoPayload) ==
                            IMS_FALSE)
                    {
                        IMS_TRACE_E(0, "Negotiate() Cannot Make Nego payload", 0, 0, 0);
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

                        IMS_TRACE_D("Negotiate() - AVPF supported. \
                                NACK[%d], TMMBR[%d], PLI[%d]",
                                pNegoPayload->GetRtcpFbAttr().IsNackSupported(),
                                pNegoPayload->GetRtcpFbAttr().IsTmmbrSupported(),
                                pNegoPayload->GetRtcpFbAttr().IsPliSupported());
                        IMS_TRACE_D("Negotiate() - AVPF supported. FIR[%d]",
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
                        if (m_bIsOfferReceived == IMS_TRUE &&
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
            IMS_TRACE_D("Negotiate() UNSUPPORTED codec[%s]",
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
                IMS_TRACE_E(0, "Negotiate() - Cannot Make Nego payload", 0, 0, 0);
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

                IMS_TRACE_D("Negotiate() set profile[%s] set resolution[%d]",
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

                    IMS_TRACE_D("Negotiate() - AVPF NACK[%d], TMMBR[%d], PLI[%d]",
                            pNegoPayload->GetRtcpFbAttr().IsNackSupported(),
                            pNegoPayload->GetRtcpFbAttr().IsTmmbrSupported(),
                            pNegoPayload->GetRtcpFbAttr().IsPliSupported());
                    IMS_TRACE_D("Negotiate() - AVPF FIR[%d], TRR[%d], Trr-int[%d]",
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
                    if (m_bIsOfferReceived == IMS_TRUE &&
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

                    IMS_TRACE_D("Negotiate() - AVPF supported. \
                            NACK[%d], TMMBR[%d], PLI[%d]",
                            pNegoPayload->GetRtcpFbAttr().IsNackSupported(),
                            pNegoPayload->GetRtcpFbAttr().IsTmmbrSupported(),
                            pNegoPayload->GetRtcpFbAttr().IsPliSupported());
                    IMS_TRACE_D("Negotiate() - AVPF supported. FIR[%d]",
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
                    if (m_bIsOfferReceived == IMS_TRUE &&
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
            pNegotiatedProfile->SetDirection(UpdateDirectionToMine(pPeerProfile->GetDirection(),
                    pLocalProfile->GetDirection(), m_bIsOfferReceived));
        }

        // if the case using different interval in live and hold, set here.
        pNegotiatedProfile->SetBandwidthRs(pPeerProfile->GetBandwidthRs());
        pNegotiatedProfile->SetBandwidthRr(pPeerProfile->GetBandwidthRr());

        if (pNegotiatedProfile->GetBandwidthRs() == 0 && pNegotiatedProfile->GetBandwidthRr() == 0)
        {
            pNegotiatedProfile->SetRtcpInterval(0);
            IMS_TRACE_D("Negotiate() - negotiated rs and rr are 0, disable rtcp", 0, 0, 0);
        }
        else
        {
            pNegotiatedProfile->SetRtcpInterval(pConfig->GetRtcpInterval());

            if (pNegotiatedProfile->GetDirection() == MEDIA_DIRECTION_SEND_RECEIVE &&
                    pConfig->GetRtcpLiveInterval() > 0)
            {
                pNegotiatedProfile->SetRtcpInterval(pConfig->GetRtcpLiveInterval());
            }
        }

        // Setting bandwidth AS/RS/RR
        VideoProfileUtil::MakeNegotiatedBandwidth(static_cast<VideoConfiguration*>(pConfig),
                pLocalProfile, pPeerProfile, m_bIsOfferReceived, nNegotiatedMaxAs,
                pNegotiatedProfile);

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

        IMS_TRACE_D("Negotiate() CVO Id[%d]", pNegotiatedProfile->GetCvoId(), 0, 0);

        ret = IMS_TRUE;
    }
    else
    {
        if (pLocalProfile->GetPayloadList().GetSize() > 0)
        {
            IMS_TRACE_D("Negotiate() There's no negotiated payload. \
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

    IMS_TRACE_D("Negotiate() Ended - Negotiated srcIndex[%d], destIndex[%d]",
            pLocalProfile->GetNegotiatedPayloadIndex(), pPeerProfile->GetNegotiatedPayloadIndex(),
            0);

    return ret;
}

PRIVATE
void VideoSdpNegotiator::ResetNegotiatedProfile(
        IN const VideoProfile* pLocalProfile, OUT VideoProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return;
    }

    *pNegotiatedProfile = *pLocalProfile;
    pNegotiatedProfile->SetDataPort(0);
    pNegotiatedProfile->SetNegotiatedPayloadIndex(-1);
}

PRIVATE
IMS_SINT32 VideoSdpNegotiator::FindPayloadIndexFromProfile(
        IN VideoProfile* pProfile, IN const VideoProfile::Payload* pPayload)
{
    if (pProfile == IMS_NULL || pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "FindPayloadIndexFromProfile() - Null Input", 0, 0, 0);
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

PUBLIC
MEDIA_DIRECTION VideoSdpNegotiator::UpdateDirectionToMine(
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

PRIVATE IMS_BOOL VideoSdpNegotiator::MakeNegotiatedCapaNegoProfile(
        IN VideoProfile::CapaNego* pLocalCapaNego, IN VideoProfile::CapaNego* pPeerCapaNego,
        OUT VideoProfile::CapaNego* pNegotiatedCapaNego)
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

PUBLIC
VIDEO_RESOLUTION VideoSdpNegotiator::GetNegotiatedResolution(
        IN MediaBaseProfile::BasePayload* pPayload)
{
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

PRIVATE IMS_BOOL VideoSdpNegotiator::MakeNegotiatedPayload(IN VideoProfile::Payload* pLocalPayload,
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

PRIVATE
VIDEO_RESOLUTION VideoSdpNegotiator::GetAvcMaxResolutionFromLevel(IN IMS_UINT32 nLevel)
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
