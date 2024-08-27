// Copyright 2024 Google LLC

#include "ServiceTrace.h"

#include "MediaProfileUtil.h"
#include "audio/AudioDef.h"
#include "audio/AudioNegoAmr.h"
#include "audio/AudioNegoEvs.h"
#include "audio/AudioSdpNegotiator.h"

#define EVS_NEGO_RETRY_COUNT 2
#define RETURN_MODE_MATCHED  IMS_FALSE
#define RETURN_MODE_SIMILAR  IMS_TRUE

__IMS_TRACE_TAG_MEDIA__;

PUBLIC AudioSdpNegotiator::AudioSdpNegotiator() :
        SdpNegotiator(MEDIA_TYPE_AUDIO)
{
    IMS_TRACE_I("+AudioSdpNegotiator()", 0, 0, 0);
}

PUBLIC VIRTUAL AudioSdpNegotiator::~AudioSdpNegotiator()
{
    IMS_TRACE_I("~AudioSdpNegotiator()", 0, 0, 0);
}

PUBLIC
IMS_BOOL AudioSdpNegotiator::Negotiate(IN AudioProfile* pLocalProfile,
        IN AudioProfile* pPeerProfile, IN IMS_BOOL bIsOfferReceived,
        OUT AudioProfile* pNegotiatedProfile, IN MediaConfiguration* pConfig)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL ||
            pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "Negotiate() invalid argument", 0, 0, 0);
        return IMS_FALSE;
    }

    m_bIsOfferReceived = bIsOfferReceived;

    IMS_TRACE_I("Negotiate() - IsOfferReceived[%d]", m_bIsOfferReceived, 0, 0);

    if (NegotiateIpPort(pLocalProfile, pPeerProfile, pNegotiatedProfile) != IMS_TRUE)
    {
        ResetNegotiatedProfile(pLocalProfile, pNegotiatedProfile);
    }

    AudioProfile::Payload* pNegotiatedPayload = IMS_NULL;
    pNegotiatedPayload = NegotiatePayload(pLocalProfile, pPeerProfile, pNegotiatedProfile);

    if (pNegotiatedPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (NegotiateDirection(pLocalProfile, pPeerProfile, pNegotiatedProfile) != IMS_TRUE)
    {
        return IMS_FALSE;
    }

    NegotiateRtcpXr(pLocalProfile, pNegotiatedProfile);
    NegotiatePtime(pNegotiatedProfile, pLocalProfile->GetPtime());
    NegotiateMaxPtime(pNegotiatedProfile, pLocalProfile->GetMaxPtime());
    NegotiateAnbr(
            pLocalProfile->IsAnbrSupported(), pPeerProfile->IsAnbrSupported(), pNegotiatedProfile);
    pNegotiatedProfile->SetCandidateAttr(pLocalProfile->GetCandidateAttr());
    NegotiateBandwidth(
            pLocalProfile, pPeerProfile, pNegotiatedProfile, pNegotiatedPayload, pConfig);
    NegotiateRtcpInterval(
            pNegotiatedProfile, pConfig->GetRtcpInterval(), pConfig->GetRtcpLiveInterval());

    return IMS_TRUE;
}

PRIVATE
void AudioSdpNegotiator::ResetNegotiatedProfile(
        IN const AudioProfile* pLocalProfile, OUT AudioProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return;
    }

    *pNegotiatedProfile = *pLocalProfile;
    pNegotiatedProfile->SetDataPort(0);
}

PRIVATE
AudioProfile::Payload* AudioSdpNegotiator::NegotiatePayload(IN AudioProfile* pLocalProfile,
        IN AudioProfile* pPeerProfile, IN AudioProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_D("NegotiatePayload()", 0, 0, 0);

    AudioProfile::Payload* pNegotiatedPayload = IMS_NULL;
    pNegotiatedPayload = NegotiateAudioPayload(pLocalProfile, pPeerProfile, pNegotiatedProfile);

    if (pNegotiatedPayload == IMS_NULL)
    {
        IMS_TRACE_D("NegotiatePayload() - No Negotiated Payload", 0, 0, 0);
        return IMS_NULL;
    }

    IMS_SINT32 nNegotiatedSamplingRate = 0;
    nNegotiatedSamplingRate = pNegotiatedPayload->GetRtpMap().GetSamplingRate();

    IMS_BOOL bTelephoneEventPayloadNegotiated = IMS_FALSE;
    if (nNegotiatedSamplingRate > 0)
    {
        bTelephoneEventPayloadNegotiated = NegotiateTelephoneEventPayload(
                nNegotiatedSamplingRate, pPeerProfile, pNegotiatedProfile);
    }

    if (bTelephoneEventPayloadNegotiated != IMS_TRUE)
    {
        NegotiateTelephoneEvent8000Payload(
                nNegotiatedSamplingRate, pPeerProfile, pNegotiatedProfile);
    }

    return pNegotiatedPayload;
}

PRIVATE
AudioProfile::Payload* AudioSdpNegotiator::NegotiateAudioPayload(IN AudioProfile* pLocalProfile,
        IN AudioProfile* pPeerProfile, IN AudioProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_D("NegotiateAudioPayload()", 0, 0, 0);

    for (IMS_UINT32 i = 0; i < pPeerProfile->GetPayloadList().GetSize(); i++)
    {
        AudioProfile::Payload* pDestPayload = pPeerProfile->GetPayloadAt(i);
        if (pDestPayload == IMS_NULL)
        {
            continue;
        }

        if (pDestPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR") ||
                pDestPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
        {
            IMS_UINT32 nNegoModeSetList = 0;
            IMS_UINT32 nNegoDefaultRtpModeSet = 0;

            if (FindAmrInProfile(pLocalProfile, pDestPayload, &nNegoModeSetList,
                        &nNegoDefaultRtpModeSet) == IMS_TRUE)
            {
                return NegotiateAmr(pLocalProfile, pPeerProfile, pNegotiatedProfile, i,
                        nNegoModeSetList, nNegoDefaultRtpModeSet);
            }
        }
        else if (pDestPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
        {
            IMS_UINT32 nBandwidthNegoList = 0;
            IMS_UINT32 nBitrateNegoList = 0;
            IMS_UINT32 nModeSetNegoList = 0;
            // need to modify FindEvsInProfile() func..
            if (FindEvsInProfile(pLocalProfile, pDestPayload, &nBandwidthNegoList,
                        &nBitrateNegoList, &nModeSetNegoList) == IMS_TRUE)
            {
                return NegotiateEvs(pLocalProfile, pPeerProfile, pNegotiatedProfile, i,
                        nBandwidthNegoList, nBitrateNegoList, nModeSetNegoList);
            }
        }
        else if (pDestPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMU") ||
                pDestPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMA"))
        {
            if (FindPcmInProfile(pLocalProfile, pDestPayload) == IMS_TRUE)
            {
                return NegotiatePcm(pLocalProfile, pPeerProfile, pNegotiatedProfile, i);
            }
        }
    }

    return IMS_NULL;
}

PRIVATE
AudioProfile::Payload* AudioSdpNegotiator::NegotiateAmr(IN AudioProfile* pLocalProfile,
        IN AudioProfile* pPeerProfile, IN AudioProfile* pNegotiatedProfile,
        IN IMS_UINT32 nPayloadIndex, IN IMS_UINT32 nNegoModeSetList,
        IN IMS_UINT32 nNegoDefaultRtpModeSet)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_D("NegotiateAmr() - Payload Index[%d]", nPayloadIndex, 0, 0);

    AudioProfile::Payload* pAmr = new AudioProfile::Payload();
    AudioProfile::Payload* pDestPayload = pPeerProfile->GetPayloadAt(nPayloadIndex);

    if (pAmr == IMS_NULL || pDestPayload == IMS_NULL)
    {
        return IMS_NULL;
    }

    pAmr->SetRtpMap(pDestPayload->GetRtpMap());

    IMS_SINT32 nSrcPayloadIndex = -1;
    AudioProfile::AmrFmtp* pAmrFmtp = NegotiateAmrFmtp(pLocalProfile, pDestPayload,
            nNegoModeSetList, nNegoDefaultRtpModeSet, nSrcPayloadIndex);

    pAmr->SetFmtp(pAmrFmtp);
    pNegotiatedProfile->GetPayloadList().Append(pAmr);

    if (pPeerProfile->GetNegotiatedPayloadIndex() == -1)
    {
        // Set the index of negotiated payload from the list.
        pPeerProfile->SetNegotiatedPayloadIndex(nPayloadIndex);
        // set nego payload index at src profile
        pLocalProfile->SetNegotiatedPayloadIndex(nSrcPayloadIndex);
        IMS_TRACE_D("NegotiateAmr() - nego payload index[%d]",
                pLocalProfile->GetNegotiatedPayloadIndex(), 0, 0);

        // MT case : change src PT# to dest PT#
        if (m_bIsOfferReceived == IMS_TRUE && pLocalProfile->GetNegotiatedPayloadIndex() != -1)
        {
            AudioProfile::Payload* pTempNegoSrcPayload =
                    pLocalProfile->GetPayloadAt(pLocalProfile->GetNegotiatedPayloadIndex());

            pTempNegoSrcPayload->GetRtpMap().SetPayloadNumber(
                    pDestPayload->GetRtpMap().GetPayloadNumber());
        }
    }

    if (pNegotiatedProfile->GetNegotiatedPayloadIndex() == -1)
    {
        pNegotiatedProfile->SetNegotiatedPayloadIndex(
                pNegotiatedProfile->GetPayloadList().GetSize() - 1);
    }

    return pAmr;
}

PRIVATE
AudioProfile::AmrFmtp* AudioSdpNegotiator::NegotiateAmrFmtp(IN AudioProfile* pLocalProfile,
        IN AudioProfile::Payload* pDestPayload, IN IMS_UINT32 nNegoModeSetList,
        IN IMS_UINT32 nNegoDefaultRtpModeSet, OUT IMS_SINT32& nSrcPayloadIndex)
{
    if (pLocalProfile == IMS_NULL || pDestPayload == IMS_NULL)
    {
        return IMS_NULL;
    }

    nSrcPayloadIndex = FindPayloadIndexFromProfile(
            pDestPayload->GetRtpMap().GetPayloadType(), pLocalProfile, pDestPayload);

    IMS_TRACE_D("NegotiateAmrFmtp() - Src Payload Index[%d]", nSrcPayloadIndex, 0, 0);

    AudioProfile::AmrFmtp* pSrcFmtp =
            (AudioProfile::AmrFmtp*)pLocalProfile->GetPayloadAt(nSrcPayloadIndex)->GetFmtp();
    AudioProfile::AmrFmtp* pAmrFmtp = new AudioProfile::AmrFmtp(
            *static_cast<AudioProfile::AmrFmtp*>(pDestPayload->GetFmtp()));

    pAmrFmtp->SetModeSetList(nNegoModeSetList);
    pAmrFmtp->SetDefaultRtpModeSet(nNegoDefaultRtpModeSet);
    pAmrFmtp->SetDtx(pSrcFmtp->IsDtxEnabled());
    pAmrFmtp->SetShowModeChangeCapability(pSrcFmtp->IsModeChangeCapabilityVisible());
    pAmrFmtp->SetModeChangeCapability(pSrcFmtp->GetModeChangeCapability());
    pAmrFmtp->SetShowModeChangeNeighbor(pSrcFmtp->IsModeChangeNeighborVisible());
    pAmrFmtp->SetModeChangeNeighbor(pSrcFmtp->GetModeChangeNeighbor());
    pAmrFmtp->SetShowModeChangePeriod(pSrcFmtp->IsModeChangePeriodVisible());
    pAmrFmtp->SetModeChangePeriod(pSrcFmtp->GetModeChangePeriod());

    return pAmrFmtp;
}

PRIVATE
AudioProfile::Payload* AudioSdpNegotiator::NegotiateEvs(IN AudioProfile* pLocalProfile,
        IN AudioProfile* pPeerProfile, IN AudioProfile* pNegotiatedProfile,
        IN IMS_UINT32 nPayloadIndex, IN IMS_UINT32 nBandwidthNegoList,
        IN IMS_UINT32 nBitrateNegoList, IN IMS_UINT32 nModeSetNegoList)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_D("NegotiateEvs() - Payload Index[%d]", nPayloadIndex, 0, 0);

    AudioProfile::Payload* pEvs = new AudioProfile::Payload();
    AudioProfile::Payload* pDestPayload = pPeerProfile->GetPayloadAt(nPayloadIndex);

    if (pEvs == IMS_NULL || pDestPayload == IMS_NULL)
    {
        return IMS_NULL;
    }

    pEvs->SetRtpMap(pDestPayload->GetRtpMap());

    IMS_SINT32 nSrcPayloadIndex = -1;
    AudioProfile::EvsFmtp* pEvsFmtp = NegotiateEvsFmtp(pLocalProfile, pDestPayload,
            nBandwidthNegoList, nBitrateNegoList, nModeSetNegoList, nSrcPayloadIndex);

    pEvs->SetFmtp(pEvsFmtp);
    pNegotiatedProfile->GetPayloadList().Append(pEvs);

    if (pPeerProfile->GetNegotiatedPayloadIndex() == -1)
    {
        // Set the index of negotiated payload from the list
        pPeerProfile->SetNegotiatedPayloadIndex(nPayloadIndex);
        // set nego payload index at src profile
        pLocalProfile->SetNegotiatedPayloadIndex(nSrcPayloadIndex);

        // MT case : change src PT# to dest PT#
        if (m_bIsOfferReceived == IMS_TRUE && pLocalProfile->GetNegotiatedPayloadIndex() != -1)
        {
            AudioProfile::Payload* pTempNegoSrcPayload =
                    pLocalProfile->GetPayloadAt(pLocalProfile->GetNegotiatedPayloadIndex());

            pTempNegoSrcPayload->GetRtpMap().SetPayloadNumber(
                    pDestPayload->GetRtpMap().GetPayloadNumber());
        }
    }
    if (pNegotiatedProfile->GetNegotiatedPayloadIndex() == -1)
    {
        pNegotiatedProfile->SetNegotiatedPayloadIndex(
                pNegotiatedProfile->GetPayloadList().GetSize() - 1);
    }

    return pEvs;
}
PRIVATE
AudioProfile::EvsFmtp* AudioSdpNegotiator::NegotiateEvsFmtp(IN AudioProfile* pLocalProfile,
        IN AudioProfile::Payload* pDestPayload, IN IMS_UINT32 nBandwidthNegoList,
        IN IMS_UINT32 nBitrateNegoList, IN IMS_UINT32 nModeSetNegoList,
        OUT IMS_SINT32& nSrcPayloadIndex)
{
    if (pLocalProfile == IMS_NULL || pDestPayload == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_D("NegotiateEvsFmtp() - Src Payload Index[%d]", nSrcPayloadIndex, 0, 0);

    nSrcPayloadIndex = FindPayloadIndexFromProfile(
            pDestPayload->GetRtpMap().GetPayloadType(), pLocalProfile, pDestPayload);
    AudioProfile::EvsFmtp* pSrcFmtp =
            (AudioProfile::EvsFmtp*)pLocalProfile->GetPayloadAt(nSrcPayloadIndex)->GetFmtp();
    AudioProfile::EvsFmtp* pEvsFmtp = new AudioProfile::EvsFmtp(
            *reinterpret_cast<AudioProfile::EvsFmtp*>(pDestPayload->GetFmtp()));
    pEvsFmtp->SetBwList(nBandwidthNegoList);
    pEvsFmtp->SetBrList(nBitrateNegoList);
    pEvsFmtp->SetModeSetList(nModeSetNegoList);

    if (pEvsFmtp->IsDtxEnabled() != pSrcFmtp->IsDtxEnabled())
    {
        IMS_TRACE_D("NegotiateEvsFmtp() - DTX updated in the destination profile", 0, 0, 0);
    }

    pEvsFmtp->SetShowModeChangeCapability(pSrcFmtp->IsModeChangeCapabilityVisible());
    pEvsFmtp->SetModeChangeCapability(pSrcFmtp->GetModeChangeCapability());
    pEvsFmtp->SetShowModeChangeNeighbor(pSrcFmtp->IsModeChangeNeighborVisible());
    pEvsFmtp->SetModeChangeNeighbor(pSrcFmtp->GetModeChangeNeighbor());
    pEvsFmtp->SetShowModeChangePeriod(pSrcFmtp->IsModeChangePeriodVisible());
    pEvsFmtp->SetModeChangePeriod(pSrcFmtp->GetModeChangePeriod());

    NegotiateUniDirectionBrBw(pEvsFmtp, nBitrateNegoList, nBandwidthNegoList);
    NegotiateCmr(pSrcFmtp, pEvsFmtp);
    NegotiateModeSet(pEvsFmtp);

    return pEvsFmtp;
}

PRIVATE
void AudioSdpNegotiator::NegotiateUniDirectionBrBw(OUT AudioProfile::EvsFmtp* pEvsFmtp,
        IN IMS_UINT32 nBandwidthNegoList, IN IMS_UINT32 nBitrateNegoList)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    if (pEvsFmtp->GetBrSend() != 0)
    {
        pEvsFmtp->SetBrSend(nBitrateNegoList);
    }
    if (pEvsFmtp->GetBrRecv() != 0)
    {
        pEvsFmtp->SetBrRecv(nBitrateNegoList);
    }
    if (pEvsFmtp->GetBwSend() != 0)
    {
        pEvsFmtp->SetBwSend(nBandwidthNegoList);
    }
    if (pEvsFmtp->GetBwRecv() != 0)
    {
        pEvsFmtp->SetBwRecv(nBandwidthNegoList);
    }
}

PRIVATE
void AudioSdpNegotiator::NegotiateCmr(
        IN AudioProfile::EvsFmtp* pSrcFmtp, OUT AudioProfile::EvsFmtp* pEvsFmtp)
{
    if (pSrcFmtp == IMS_NULL || pEvsFmtp == IMS_NULL)
    {
        return;
    }

    pEvsFmtp->SetSendCmr(pSrcFmtp->IsSendCmrEnabled());

    // CMR on/off, if bitrate is not range set, disable CMR send option
    IMS_UINT32 nCount = 0;
    IMS_UINT32 nTempBrList = pEvsFmtp->GetBrList();

    for (IMS_UINT32 i = 0; i < 16; i++)
    {
        if (nTempBrList & 0x01)
        {
            nCount++;
        }

        nTempBrList = nTempBrList >> 1;

        if (nTempBrList == 0)
        {
            break;
        }
    }

    if (nCount <= 1)
    {
        pEvsFmtp->SetSendCmr(IMS_FALSE);
    }
}

PRIVATE
void AudioSdpNegotiator::NegotiateModeSet(OUT AudioProfile::EvsFmtp* pEvsFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    /** fixed for IR92 ver.12 newaly spec as below comment. If the selected EVS
     * configuration is A1, B0 or B1 then"mode set = 0,1,2" must be included in the SDP
     * answer.*/
    if (m_bIsOfferReceived == IMS_TRUE && pEvsFmtp->GetEvsModeSwitch() != 1)
    {
        // if max BR is 13.2kbps, then set a"mode-set" attribute
        if (((pEvsFmtp->GetBrList() & 0x10) != 0) && ((pEvsFmtp->GetBrList() & 0xFFE0) == 0))
        {
            pEvsFmtp->SetModeSetList(0x07);  // mode-set = 0,1,2;
            pEvsFmtp->SetShowModeSet(IMS_TRUE);
            IMS_TRACE_D("NegotiateModeSet() - add EVS mode-set", 0, 0, 0);
        }
    }
}

PRIVATE
AudioProfile::Payload* AudioSdpNegotiator::NegotiatePcm(IN AudioProfile* pLocalProfile,
        IN AudioProfile* pPeerProfile, IN AudioProfile* pNegotiatedProfile,
        IN IMS_UINT32 nPayloadIndex)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_D("NegotiatePcm() - Payload Index[%d]", nPayloadIndex, 0, 0);

    AudioProfile::Payload* pPcm = new AudioProfile::Payload();
    AudioProfile::Payload* pDestPayload = pPeerProfile->GetPayloadAt(nPayloadIndex);

    if (pPcm == IMS_NULL || pDestPayload == IMS_NULL)
    {
        return IMS_NULL;
    }

    pPcm->SetRtpMap(pDestPayload->GetRtpMap());
    pNegotiatedProfile->GetPayloadList().Append(pPcm);

    if (pPeerProfile->GetNegotiatedPayloadIndex() == -1)
    {
        // Set the index of negotiated payload from the list
        pPeerProfile->SetNegotiatedPayloadIndex(nPayloadIndex);
        pLocalProfile->SetNegotiatedPayloadIndex(FindPayloadIndexFromProfile(
                pDestPayload->GetRtpMap().GetPayloadType(), pLocalProfile, pDestPayload));
    }

    if (pNegotiatedProfile->GetNegotiatedPayloadIndex() == -1)
    {
        pNegotiatedProfile->SetNegotiatedPayloadIndex(
                pNegotiatedProfile->GetPayloadList().GetSize() - 1);
    }

    return pPcm;
}

PRIVATE
IMS_BOOL AudioSdpNegotiator::NegotiateTelephoneEventPayload(IN IMS_SINT32 nNegotiatedSamplingRate,
        IN AudioProfile* pPeerProfile, OUT AudioProfile* pNegotiatedProfile)
{
    if (nNegotiatedSamplingRate == 0 || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D(
            "NegotiateTelephoneEventPayload() - sampling rate[%d]", nNegotiatedSamplingRate, 0, 0);

    for (IMS_UINT32 i = 0; i < pPeerProfile->GetPayloadList().GetSize(); i++)
    {
        AudioProfile::Payload* pDestPayload = pPeerProfile->GetPayloadAt(i);

        if (pDestPayload == IMS_NULL)
        {
            continue;
        }

        if (pDestPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("telephone-event"))
        {
            if (pDestPayload->GetRtpMap().GetSamplingRate() == nNegotiatedSamplingRate)
            {
                AudioProfile::Payload* pTelephoneEvent = new AudioProfile::Payload();
                pTelephoneEvent->SetRtpMap(pDestPayload->GetRtpMap());
                pTelephoneEvent->SetFmtp(new AudioProfile::TelephoneEventFmtp(
                        *static_cast<AudioProfile::TelephoneEventFmtp*>(pDestPayload->GetFmtp())));
                pNegotiatedProfile->GetPayloadList().Append(pTelephoneEvent);

                IMS_TRACE_D("NegotiateTelephoneEventPayload() - payload[%d]",
                        pTelephoneEvent->GetRtpMap().GetPayloadNumber(), 0, 0);

                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PRIVATE
void AudioSdpNegotiator::NegotiateTelephoneEvent8000Payload(IN IMS_SINT32 nNegotiatedSamplingRate,
        IN AudioProfile* pPeerProfile, OUT AudioProfile* pNegotiatedProfile)
{
    if (pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D(
            "NegotiateTelephoneEvent8000Payload() - need to accept 8K DTMF, no proper DTMF Payload",
            0, 0, 0);

    for (IMS_UINT32 i = 0; i < pPeerProfile->GetPayloadList().GetSize(); i++)
    {
        AudioProfile::Payload* pDestPayload = pPeerProfile->GetPayloadAt(i);

        if (pDestPayload == IMS_NULL)
        {
            continue;
        }
        if (pDestPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("telephone-event"))
        {
            // for acceptable 8K DTMF when AMR-WB calling
            if (nNegotiatedSamplingRate > pDestPayload->GetRtpMap().GetSamplingRate())
            {
                IMS_TRACE_D("NegotiateTelephoneEvent8000Payload() - Accept sampling rate[%d]->[%d]",
                        nNegotiatedSamplingRate, pDestPayload->GetRtpMap().GetSamplingRate(), 0);
                AudioProfile::Payload* pTelephoneEvent = new AudioProfile::Payload();
                pTelephoneEvent->SetRtpMap(pDestPayload->GetRtpMap());
                pTelephoneEvent->SetFmtp(new AudioProfile::TelephoneEventFmtp(
                        *static_cast<AudioProfile::TelephoneEventFmtp*>(pDestPayload->GetFmtp())));
                pNegotiatedProfile->GetPayloadList().Append(pTelephoneEvent);

                return;
            }
        }
    }
}

PRIVATE
IMS_BOOL AudioSdpNegotiator::NegotiateDirection(IN AudioProfile* pLocalProfile,
        IN AudioProfile* pPeerProfile, IN AudioProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pNegotiatedProfile->SetDirection(
            UpdateDirectionToMine(pPeerProfile->GetDirection(), pLocalProfile->GetDirection()));

    IMS_TRACE_D("NegotiateDirection() - direction[%d]", pNegotiatedProfile->GetDirection(), 0, 0);

    if (pNegotiatedProfile->GetDirection() == MEDIA_DIRECTION_INVALID)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
void AudioSdpNegotiator::NegotiateRtcpXr(
        IN AudioProfile* pLocalProfile, OUT AudioProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return;
    }

    if (pLocalProfile->IsRtcpXrSupported() == IMS_TRUE &&
            pNegotiatedProfile->GetDirection() == MEDIA_DIRECTION_SEND_RECEIVE)
    {
        pNegotiatedProfile->SetSupportRtcpXr(IMS_TRUE);
        pNegotiatedProfile->SetRtcpXrAttr(pLocalProfile->GetRtcpXrAttr());
    }

    IMS_TRACE_D("NegotiateRtcpXr() - support[%d]", pNegotiatedProfile->IsRtcpXrSupported(), 0, 0);
}

PRIVATE
void AudioSdpNegotiator::NegotiatePtime(
        OUT AudioProfile* pNegotiatedProfile, IN IMS_SINT32 nLocalPtime)
{
    if (pNegotiatedProfile == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nPtime = (nLocalPtime < 20) ? 20 : nLocalPtime;
    pNegotiatedProfile->SetPtime(nPtime);

    IMS_TRACE_D("NegotiatePtime() - Ptime[%d], Local Ptime[%d]", pNegotiatedProfile->GetPtime(),
            nLocalPtime, 0);
}

PRIVATE
void AudioSdpNegotiator::NegotiateMaxPtime(
        OUT AudioProfile* pNegotiatedProfile, IN IMS_SINT32 nLocalMaxPtime)
{
    if (pNegotiatedProfile == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nMaxPtime = (nLocalMaxPtime < 20) ? 240 : nLocalMaxPtime;
    pNegotiatedProfile->SetMaxPtime(nMaxPtime);

    IMS_TRACE_D("NegotiateMaxPtime() - Max Ptime[%d], Local Ptime[%d]",
            pNegotiatedProfile->GetMaxPtime(), nLocalMaxPtime, 0);
}

PRIVATE
void AudioSdpNegotiator::NegotiateAnbr(IN IMS_BOOL nSupportAnbrLocal, IN IMS_BOOL nSupportAnbrPeer,
        OUT AudioProfile* pNegotiatedProfile)
{
    if (pNegotiatedProfile == IMS_NULL)
    {
        return;
    }

    IMS_BOOL bEnableAnbr = IMS_FALSE;
    bEnableAnbr = (nSupportAnbrLocal && nSupportAnbrPeer);

    pNegotiatedProfile->SetAnbr(bEnableAnbr);

    IMS_TRACE_D("NegotiateAnbr() - supported local[%d], peer[%d], nego[%d]", nSupportAnbrLocal,
            nSupportAnbrPeer, pNegotiatedProfile->IsAnbrSupported());
}

PRIVATE
void AudioSdpNegotiator::NegotiateBandwidth(IN AudioProfile* pLocalProfile,
        IN AudioProfile* pPeerProfile, OUT AudioProfile* pNegotiatedProfile,
        IN AudioProfile::Payload* pNegotiatedPayload, IN MediaConfiguration* pConfig)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL ||
            pNegotiatedPayload == IMS_NULL || pConfig == IMS_NULL)
    {
        return;
    }

    pNegotiatedProfile->SetBandwidthRs(pPeerProfile->GetBandwidthRs());
    pNegotiatedProfile->SetBandwidthRr(pPeerProfile->GetBandwidthRr());

    IMS_SINT32 nAsValueOfNegoticatedCodec =
            NegotiateAs(pNegotiatedPayload, pNegotiatedProfile->GetIpAddress().IsIPv6Address());

    MakeNegotiatedBandwidth(static_cast<AudioConfiguration*>(pConfig), pLocalProfile, pPeerProfile,
            nAsValueOfNegoticatedCodec, pNegotiatedProfile);

    IMS_TRACE_D("NegotiateBandwidth() - AS[%d], RS[%d], RR[%d]",
            pNegotiatedProfile->GetBandwidthAs(), pNegotiatedProfile->GetBandwidthRs(),
            pNegotiatedProfile->GetBandwidthRr());
}

PRIVATE
IMS_SINT32 AudioSdpNegotiator::NegotiateAs(
        IN AudioProfile::Payload* pNegotiatedPayload, IN IMS_BOOL bIpv6)
{
    if (pNegotiatedPayload == IMS_NULL)
    {
        return 0;
    }

    AUDIO_CODEC nCurrCodec;
    IMS_SINT32 nModeSet;
    IMS_SINT32 nAs = 0;

    if (pNegotiatedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR") ||
            pNegotiatedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
    {
        AudioProfile::AmrFmtp* pAmrFmtp = (AudioProfile::AmrFmtp*)pNegotiatedPayload->GetFmtp();
        if (pNegotiatedPayload->GetRtpMap().GetSamplingRate() == 8000)
        {
            nCurrCodec = AUDIO_CODEC_AMR;
            nModeSet = AudioProfileUtil::GetLargestModesetInFmtp("AMR", pNegotiatedPayload);
        }
        else
        {
            nCurrCodec = AUDIO_CODEC_AMRWB;
            nModeSet = AudioProfileUtil::GetLargestModesetInFmtp("AMR-WB", pNegotiatedPayload);
        }

        nAs = AudioProfileUtil::ConvertToBandwidthAS(
                nCurrCodec, pAmrFmtp->GetOctetAlign(), bIpv6, nModeSet);
    }
    else if (pNegotiatedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
    {
        AudioProfile::EvsFmtp* pEvsFmtp = (AudioProfile::EvsFmtp*)pNegotiatedPayload->GetFmtp();
        nCurrCodec = AUDIO_CODEC_EVS;
        nModeSet = AudioProfileUtil::GetLargestModesetInFmtp("EVS", pNegotiatedPayload);
        nAs = AudioProfileUtil::ConvertToBandwidthAS(
                nCurrCodec, bIpv6, pEvsFmtp->GetEvsModeSwitch(), nModeSet);
    }

    return nAs;
}

PRIVATE
IMS_BOOL AudioSdpNegotiator::MakeNegotiatedBandwidth(IN AudioConfiguration* pConfig,
        IN AudioProfile* pLocalProfile, IN AudioProfile* pPeerProfile,
        IN IMS_SINT32 nAsValueOfNegoticatedCodec, OUT AudioProfile* pNegotiatedProfile)
{
    IMS_TRACE_D("MakeNegotiatedBandwidth() - BW_NEGO_OPTION_VALUE[%d]",
            pConfig->GetBandwidthNegoOption(), 0, 0);

    if (m_bIsOfferReceived == IMS_FALSE)
    {
        // MO's Bandwidth Setting
        //  1. Set AS Value
        if (pPeerProfile->GetBandwidthAs() > 0)
        {
            if (pPeerProfile->GetBandwidthAs() > nAsValueOfNegoticatedCodec)
            {
                pNegotiatedProfile->SetBandwidthAs(nAsValueOfNegoticatedCodec);
            }
            else
            {
                pNegotiatedProfile->SetBandwidthAs(pPeerProfile->GetBandwidthAs());
            }
        }
        else
        {  // Exception Handling (b= AS line is not included in Answer SDP)
            pNegotiatedProfile->SetBandwidthAs(pLocalProfile->GetBandwidthAs());
        }

        // 2. Set RS/RR Value
        // 2.1 Exception Handling (b=RS/RR line is not included in Answer SDP)
        if (pNegotiatedProfile->GetBandwidthRs() < 0 || pNegotiatedProfile->GetBandwidthRr() < 0)
        {
            pNegotiatedProfile->SetBandwidthRs(pLocalProfile->GetBandwidthRs());
            pNegotiatedProfile->SetBandwidthRr(pLocalProfile->GetBandwidthRr());

            IMS_TRACE_D("MakeNegotiatedBandwidth() - Nego AS[%d] RS[%d] RR[%d]",
                    pLocalProfile->GetBandwidthAs(), pLocalProfile->GetBandwidthRs(),
                    pLocalProfile->GetBandwidthRr());
            return IMS_TRUE;
        }

        // 2.2 Normal Case
        if (pConfig->GetBandwidthNegoOption() == MediaConfiguration::BW_OPTION_NEGOTIATED_VALUE)
        {
            // if RS/RR is used for RTCP Nego value
            pNegotiatedProfile->SetBandwidthRs(pPeerProfile->GetBandwidthRs());
            pNegotiatedProfile->SetBandwidthRr(pPeerProfile->GetBandwidthRr());
        }
        else
        {
            // default case (RS/RR is not negotiated value)
            pNegotiatedProfile->SetBandwidthRs(pLocalProfile->GetBandwidthRs());
            pNegotiatedProfile->SetBandwidthRr(pLocalProfile->GetBandwidthRr());
        }
    }
    else
    {
        // MT's Bandwidth Setting
        // 1. Set Negotiated AS Value
        if (nAsValueOfNegoticatedCodec > 0)
        {
            pNegotiatedProfile->SetBandwidthAs(nAsValueOfNegoticatedCodec);

            // if GetBandwidthNegoOption is BW_OPTION_NEGOTIATED_VALUE, use lower AS value
            if ((pConfig->GetBandwidthNegoOption() ==
                        MediaConfiguration::BW_OPTION_NEGOTIATED_VALUE) &&
                    (nAsValueOfNegoticatedCodec > pPeerProfile->GetBandwidthAs()) &&
                    (pPeerProfile->GetBandwidthAs() > 0))
            {
                pNegotiatedProfile->SetBandwidthAs(pPeerProfile->GetBandwidthAs());
            }
        }
        else
        {
            pNegotiatedProfile->SetBandwidthAs(pLocalProfile->GetBandwidthAs());
        }
        // 3. Set RS/RR Value
        if (pPeerProfile->GetDirection() != MEDIA_DIRECTION_SEND_RECEIVE)
        {  // Hold Case
            // 3.1 Hold Case
            MediaProfileUtil::SetRtcpRsRr(pNegotiatedProfile, pConfig);
        }
        else
        {
            // 3.2 Active Call Case
            // 3.2.1 Exception Handling (b=RS/RR line is not included in Answer SDP)
            if (pNegotiatedProfile->GetBandwidthRs() < 0 ||
                    pNegotiatedProfile->GetBandwidthRr() < 0)
            {
                pNegotiatedProfile->SetBandwidthRs(pLocalProfile->GetBandwidthRs());
                pNegotiatedProfile->SetBandwidthRr(pLocalProfile->GetBandwidthRr());

                IMS_TRACE_D("MakeNegotiatedBandwidth() - AS[%d] RS[%d] RR[%d]",
                        pLocalProfile->GetBandwidthAs(), pLocalProfile->GetBandwidthRs(),
                        pLocalProfile->GetBandwidthRr());
                return IMS_TRUE;
            }

            // 3.2.2 Normal Case
            if (pConfig->GetBandwidthNegoOption() == MediaConfiguration::BW_OPTION_NEGOTIATED_VALUE)
            {
                // if RS/RR is used for RTCP Nego value
                IMS_TRACE_D("MakeNegotiatedBandwidth() - use peer RS[%d] RR[%d]",
                        pPeerProfile->GetBandwidthRs(), pPeerProfile->GetBandwidthRr(), 0);
                pNegotiatedProfile->SetBandwidthRs(pPeerProfile->GetBandwidthRs());
                pNegotiatedProfile->SetBandwidthRr(pPeerProfile->GetBandwidthRr());
            }
            else
            {
                // default case (RS/RR is not negotiated value)
                pNegotiatedProfile->SetBandwidthRs(pLocalProfile->GetBandwidthRs());
                pNegotiatedProfile->SetBandwidthRr(pLocalProfile->GetBandwidthRr());
            }
        }
    }

    IMS_TRACE_D("MakeNegotiatedBandwidth() - Negotiated Profile AS[%d] RS[%d] RR[%d]",
            pNegotiatedProfile->GetBandwidthAs(), pNegotiatedProfile->GetBandwidthRs(),
            pLocalProfile->GetBandwidthRr());
    return IMS_TRUE;
}

PRIVATE
void AudioSdpNegotiator::NegotiateRtcpInterval(IN AudioProfile* pNegotiatedProfile,
        IN IMS_SINT32 nRtcpInterval, IN IMS_SINT32 nRtcpLiveInterval)
{
    if (pNegotiatedProfile == IMS_NULL)
    {
        return;
    }

    if (pNegotiatedProfile->GetBandwidthRs() == 0 && pNegotiatedProfile->GetBandwidthRr() == 0)
    {
        pNegotiatedProfile->SetRtcpInterval(0);
        IMS_TRACE_D("NegotiateRtcpInterval() - negotiated rs and rr are 0, disable rtcp", 0, 0, 0);
    }
    else
    {
        pNegotiatedProfile->SetRtcpInterval(nRtcpInterval);

        if (pNegotiatedProfile->GetDirection() == MEDIA_DIRECTION_SEND_RECEIVE &&
                nRtcpLiveInterval > 0)
        {
            pNegotiatedProfile->SetRtcpInterval(nRtcpLiveInterval);
        }
    }

    IMS_TRACE_D("NegotiateRtcpInterval() - Rtcp Interval[%d]",
            pNegotiatedProfile->GetRtcpInterval(), 0, 0);
}

PRIVATE
IMS_BOOL AudioSdpNegotiator::FindEvsInProfile(IN AudioProfile* pLocalProfile,
        IN AudioProfile::Payload* pDstPayload, OUT IMS_UINT32* pBandwidthNegoList,
        OUT IMS_UINT32* pBitrateNegoList, OUT IMS_UINT32* pModeSetNegoList)
{
    if (pLocalProfile == IMS_NULL || pDstPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 legacyCheck = 0; legacyCheck < EVS_NEGO_RETRY_COUNT; legacyCheck++)
    {
        for (IMS_UINT32 i = 0; i < pLocalProfile->GetPayloadList().GetSize(); i++)
        {
            AudioProfile::Payload* comparedPayload = pLocalProfile->GetPayloadAt(i);

            if (comparedPayload == IMS_NULL)
            {
                continue;
            }

            if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
            {
                AudioProfile::EvsFmtp* pCompareFmtp =
                        (AudioProfile::EvsFmtp*)comparedPayload->GetFmtp();
                AudioProfile::EvsFmtp* pReceivedFmtp =
                        (AudioProfile::EvsFmtp*)pDstPayload->GetFmtp();
                if (pCompareFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
                {
                    continue;
                }

                if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                            pDstPayload->GetRtpMap().GetPayloadType()) == IMS_FALSE)
                {
                    continue;
                }

                // condition check....
                // need to return nego BW and BR, ModeSet(AMR IO Mode.). also default values...
                if (legacyCheck == 0)
                {
                    // IR92 rel15 EVS Br/Bw check.
                    if (CompareEvsBwBrMode(pCompareFmtp, pReceivedFmtp, pBandwidthNegoList,
                                pBitrateNegoList, pModeSetNegoList) == IMS_FALSE)
                    {
                        continue;
                    }
                }
                else
                {
                    // legacy EVS BR/BW check
                    if (CompareEvsBwBrModeLegacy(pCompareFmtp, pReceivedFmtp, pBandwidthNegoList,
                                pBitrateNegoList, pModeSetNegoList) == IMS_FALSE)
                    {
                        continue;
                    }
                }

                // TODO - 20220415 - Need to implement this requirement later
                /*if (IMS_OPERATOR(VZW, GetSlotId()) && ((VZWProperty::GetInstance()->GetTestMask()
                        & TEST_MASK_UNLOCK_EVS_NEGO_LIMIT) == 0))
                {
                    //let channel aware mode option to be a negotitation item
                    if (pReceivedFmtp->GetBwList() == 1)
                    {
                        continue;      //vzw req. if offer is nb only, no evs negotiation
                }
                    if (pDstPayload->GetRtpMap().GetPayloadNumber()
                            != comparedPayload->GetRtpMap().GetPayloadNumber())
                    {
                        continue;     //vzw req. payload number based negotiation
                    }
                }*/

                // check channel aware mode received param
                if (pReceivedFmtp->GetReceivedChAwRecv() > 0)
                {
                    IMS_SINT32 tempReceivedChAw = pReceivedFmtp->GetReceivedChAwRecv();

                    if ((tempReceivedChAw != 2) && (tempReceivedChAw != 3) &&
                            (tempReceivedChAw != 5) && (tempReceivedChAw != 7))
                    {
                        continue;
                    }
                }

                // fixed for IR92 ver.12 newaly spec as below comment.
                // If ther SDP parameter ch-aw-recv is present
                // in the corresponding received and accepted SDP offer,
                // then the SDP parameter ch-aw-recv must be included
                // in the SDP answer with the same value as received.
                // In Offered case, set ChAw Mode for mine.
                if (((pReceivedFmtp->GetBwList() & 0x06) != 0 &&
                            (pReceivedFmtp->GetBrList() & 0x10) != 0) ||
                        ((*pBandwidthNegoList & 0x06) != 0 && (*pBitrateNegoList & 0x10) != 0))
                {
                    if (m_bIsOfferReceived != IMS_TRUE)
                    {
                        pReceivedFmtp->SetChAwRecv(pCompareFmtp->GetChAwRecv());
                        pReceivedFmtp->SetShowChannelAwMode(pCompareFmtp->IsChannelAwModeVisible());
                    }
                }

                IMS_TRACE_D("FindEvsInProfile() Found EVS at[%d], nBandwidthNegoList[0x%04x], \
                        nBitrateNegoList[0x%04x]",
                        i, *pBandwidthNegoList, *pBitrateNegoList);
                IMS_TRACE_D("FindEvsInProfile() EVS ModeSwitch[%d], nModeSetNegoList[0x%04x], \
                        SendCmr[%d]",
                        pReceivedFmtp->GetEvsModeSwitch(), *pModeSetNegoList,
                        pCompareFmtp->IsSendCmrEnabled());
                IMS_TRACE_D("FindEvsInProfile() EVS ChAwMode[%d], opposite ChAwMode[0x%04x], \
                        legacyCheck[%d]",
                        pReceivedFmtp->GetChAwRecv(), pReceivedFmtp->GetReceivedChAwRecv(),
                        legacyCheck);

                return IMS_TRUE;
            }
        }
    }
    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioSdpNegotiator::FindAmrInProfile(IN AudioProfile* pProfile,
        IN AudioProfile::Payload* pPayload, OUT IMS_UINT32* pnNegoModeSetList,
        OUT IMS_UINT32* pnNegoDefaultRtpModeSet)
{
    if (pProfile == IMS_NULL || pPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_BOOL bRetModeSetFound = IMS_FALSE;

    bRetModeSetFound = FindMatchedAmrInProfile(
            pProfile, pPayload, RETURN_MODE_MATCHED, pnNegoModeSetList, pnNegoDefaultRtpModeSet);
    IMS_TRACE_D("FindMatchedAmrInProfile() Ended. the 1st bRetModeSetFound: %d", bRetModeSetFound,
            0, 0);

    if (!bRetModeSetFound)
    {
        bRetModeSetFound = FindMatchedAmrInProfile(pProfile, pPayload, RETURN_MODE_SIMILAR,
                pnNegoModeSetList, pnNegoDefaultRtpModeSet);
        IMS_TRACE_D("FindMatchedAmrInProfile() Ended. the 2nd bRetModeSetFound: %d",
                bRetModeSetFound, 0, 0);
    }

    return bRetModeSetFound;
}

PRIVATE
IMS_BOOL AudioSdpNegotiator::FindMatchedAmrInProfile(IN AudioProfile* pProfile,
        IN AudioProfile::Payload* pPayload, IN IMS_BOOL bReturnMode,
        OUT IMS_UINT32* pnNegoModeSetList, OUT IMS_UINT32* pnNegoDefaultRtpModeSet)
{
    IMS_UINT32 nTempNegoModeSetList = 0;
    IMS_UINT32 nTempDefaultNegoModeSetList = 0;
    IMS_BOOL bModeSetFound = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        AudioProfile::Payload* comparedPayload = pProfile->GetPayloadAt(i);

        if (comparedPayload == IMS_NULL)
        {
            continue;
        }

        if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR") ||
                comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
        {
            AudioProfile::AmrFmtp* pCompareFmtp =
                    (AudioProfile::AmrFmtp*)comparedPayload->GetFmtp();
            AudioProfile::AmrFmtp* pReceivedFmtp = (AudioProfile::AmrFmtp*)pPayload->GetFmtp();

            if (pCompareFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
            {
                continue;
            }
            if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                        pPayload->GetRtpMap().GetPayloadType()) == IMS_FALSE)
            {
                continue;
            }
            if (comparedPayload->GetRtpMap().GetSamplingRate() !=
                    pPayload->GetRtpMap().GetSamplingRate())
            {
                continue;
            }

            if (pCompareFmtp->GetOctetAlign() != pReceivedFmtp->GetOctetAlign())
            {
                continue;
            }

            /** Fix for AMR Open Offer In case of MO, mode-set from MT could be mismatched Keep
            negotiated mode-set and try to find exact one */
            IMS_SINT32 nCompareResult = CompareModeSet(pCompareFmtp, pReceivedFmtp, bReturnMode,
                    pnNegoModeSetList, pnNegoDefaultRtpModeSet);

            if (nCompareResult == -1)
            {
                IMS_TRACE_D("FindAmrInProfile() nCompareResult - not match", 0, 0, 0);
                continue;  // mismatched
            }
            else if (nCompareResult == 0)  // similarly matched
            {
                IMS_TRACE_D("FindAmrInProfile() Enter similar match - bReturnMode: %d", bReturnMode,
                        0, 0);
                if (bModeSetFound == IMS_FALSE && bReturnMode == RETURN_MODE_SIMILAR)
                {
                    nTempNegoModeSetList = *pnNegoModeSetList;
                    nTempDefaultNegoModeSetList = *pnNegoDefaultRtpModeSet;
                    bModeSetFound = IMS_TRUE;
                    IMS_TRACE_I("FindAmrInProfile() Local/Peer is not exactly matched\
                            [0x%04x][0x%04x] =>[0x%04x]. Try next",
                            pCompareFmtp->GetModeSetList(), pReceivedFmtp->GetModeSetList(),
                            *pnNegoModeSetList);
                }
                continue;
            }
            else  // exactly matched
            {
                IMS_TRACE_D("FindAmrInProfile() Found AMR at[%d], Codec[%s], OctetAlign[%d]", i,
                        comparedPayload->GetRtpMap().GetPayloadType().GetStr(),
                        pCompareFmtp->GetOctetAlign());
                IMS_TRACE_D("FindAmrInProfile() Local/Peer is exactly matched[0x%04x][0x%04x] \
                        =>[0x%04x]",
                        pCompareFmtp->GetModeSetList(), pReceivedFmtp->GetModeSetList(),
                        *pnNegoModeSetList);

                return IMS_TRUE;
            }
        }
    }

    // 20160517 Fix for AMR Open Offer
    if (bModeSetFound == IMS_TRUE)
    {
        *pnNegoModeSetList = nTempNegoModeSetList;
        *pnNegoDefaultRtpModeSet = nTempDefaultNegoModeSetList;

        IMS_TRACE_D("FindAmrInProfile() Found Similar AMR with ModeSet List[0x%04x], "
                    "nDefaultModeSetList[0x%04x]",
                *pnNegoModeSetList, *pnNegoDefaultRtpModeSet, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioSdpNegotiator::FindPcmInProfile(
        IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload)
{
    if (pProfile == IMS_NULL || pPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        AudioProfile::Payload* comparedPayload = pProfile->GetPayloadAt(i);

        if (comparedPayload == IMS_NULL)
        {
            continue;
        }

        if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMU") ||
                comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMA"))
        {
            if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                        pPayload->GetRtpMap().GetPayloadType()) == IMS_FALSE)
            {
                continue;
            }
            if (comparedPayload->GetRtpMap().GetSamplingRate() !=
                    pPayload->GetRtpMap().GetSamplingRate())
            {
                continue;
            }

            IMS_TRACE_D("FindPcmInProfile() Found G.711 at[%d], Codec[%s], nSamplingRate[%d]", i,
                    comparedPayload->GetRtpMap().GetPayloadType().GetStr(),
                    pPayload->GetRtpMap().GetSamplingRate());

            return IMS_TRUE;
        }
    }

    // IMS_TRACE_D("FindPcmInProfile() Not Found G.711[%s], SamplingRate[%d]",
    // pPayload->GetRtpMap().GetPayloadType().GetStr(), pPayload->GetRtpMap().GetSamplingRate(), 0);

    return IMS_FALSE;
}

PRIVATE
IMS_SINT32 AudioSdpNegotiator::CompareModeSet(IN AudioProfile::AmrFmtp* pSrcFmtp,
        IN AudioProfile::AmrFmtp* pDestFmtp, IN IMS_BOOL bReturnMode, OUT IMS_UINT32* nNegoModeSet,
        OUT IMS_UINT32* nNegoDefaultRtpModeSet)
{
    IMS_TRACE_I("CompareModeSet() - Src modeSet[0x%04x] Dest modeSet[0x%04x]",
            pSrcFmtp->GetModeSetList(), pDestFmtp->GetModeSetList(), 0);
    IMS_TRACE_I("CompareModeSet() - Src defaultmodeSet[0x%04x] Dest defaultmodeSet[0x%04x]",
            pSrcFmtp->GetDefaultRtpModeSet(), pDestFmtp->GetDefaultRtpModeSet(), 0);

    IMS_SINT32 nResult = 1;  // -1 : no matched, 0 : similar, 1 : exactly matched

    if (m_bIsOfferReceived == IMS_TRUE)  // MT Case
    {
        if (pDestFmtp->GetModeSetList() != 0)
        {
            *nNegoModeSet = pDestFmtp->GetModeSetList();
        }
        else if (pSrcFmtp->GetModeSetList() != 0)
        {
            *nNegoModeSet = pSrcFmtp->GetModeSetList();
        }
        else
        {
            *nNegoModeSet = 0;  // MO, MT both has no mode-set
            *nNegoDefaultRtpModeSet = pSrcFmtp->GetDefaultRtpModeSet();
        }
    }
    else  // MO Case
    {
        if ((pSrcFmtp->GetModeSetList() == 0) && (pDestFmtp->GetModeSetList() == 0))
        {
            *nNegoModeSet = 0;
            *nNegoDefaultRtpModeSet = pSrcFmtp->GetDefaultRtpModeSet();
        }
        else if ((pSrcFmtp->GetModeSetList() != 0) && (pDestFmtp->GetModeSetList() != 0))
        {
            *nNegoModeSet = pSrcFmtp->GetModeSetList() & pDestFmtp->GetModeSetList();

            if (pSrcFmtp->GetModeSetList() != pDestFmtp->GetModeSetList())
            {
                nResult = 0;
            }

            if (*nNegoModeSet == 0)
            {
                IMS_TRACE_D(
                        "CompareModeSet() - ModeSet Not Matched - isFinal: %d", bReturnMode, 0, 0);
                if (bReturnMode == RETURN_MODE_SIMILAR)
                {
                    *nNegoModeSet =
                            pDestFmtp->GetModeSetList();  // Copy Dest Mode-set to Nego Mode-set
                    IMS_TRACE_D("CompareModeSet() - Copy Dest Mode-set", 0, 0, 0);
                }
                else
                {
                    IMS_TRACE_E(0, "CompareModeSet() - ModeSet Not Matched...", 0, 0, 0);
                    return -1;
                }
            }
        }
        else  // one of two has no modeset
        {
            *nNegoModeSet = pSrcFmtp->GetModeSetList() | pDestFmtp->GetModeSetList();
        }
    }

    IMS_TRACE_I(
            "CompareModeSet() Result[%d] Negotiated modeSet[0x%04x] nNegoDefaultRtpModeSet[0x%04x]",
            nResult, *nNegoModeSet, *nNegoDefaultRtpModeSet);

    return nResult;
}

PRIVATE
IMS_BOOL AudioSdpNegotiator::CompareEvsBwBrMode(IN AudioProfile::EvsFmtp* pSrcFmtp,
        IN AudioProfile::EvsFmtp* pDestFmtp, OUT IMS_UINT32* nNegoBwList,
        OUT IMS_UINT32* nNegoBrList, OUT IMS_UINT32* nNegoModeList)
{
    IMS_TRACE_D("CompareEvsBwBrMode() - Src Bandwidth[0x%04x], Bitrate[0x%04x], modeSet[0x%04x]",
            pSrcFmtp->GetBwList(), pSrcFmtp->GetBrList(), pSrcFmtp->GetModeSetList());
    IMS_TRACE_D("CompareEvsBwBrMode() - Dest Bandwidth[0x%04x], Bitrate[0x%04x], modeSet[0x%04x]",
            pDestFmtp->GetBwList(), pDestFmtp->GetBrList(), pDestFmtp->GetModeSetList());

    // check EVS ModeSwitch
    if (pDestFmtp->GetEvsModeSwitch() == 1)  // AMR IO Mode
    {
        // Set AMR ModeSet lis.
        if ((pSrcFmtp->GetModeSetList() == 0) && (pDestFmtp->GetModeSetList() == 0))
        {
            *nNegoModeList = 0;
        }
        else if ((pSrcFmtp->GetModeSetList() != 0) && (pDestFmtp->GetModeSetList() != 0))
        {
            *nNegoModeList = pSrcFmtp->GetModeSetList() & pDestFmtp->GetModeSetList();

            if (*nNegoModeList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrMode() - AMR IO Mode - ModeSet Not Matched...", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoModeList = pSrcFmtp->GetModeSetList() | pDestFmtp->GetModeSetList();
        }

        // in case of EVS AMR IO Mode, dest bw/br list added
        *nNegoBwList = pDestFmtp->GetBwList();
        *nNegoBrList = pDestFmtp->GetBrList();
    }
    else  // Primary Mode
    {
        // Set Bandwidth/Bitrate list.
        // 01. check Bandwidth
        if ((pSrcFmtp->GetBwList() == 0) && (pDestFmtp->GetBwList() == 0))
        {
            *nNegoBwList = 0;
        }
        else if ((pSrcFmtp->GetBwList() != 0) && (pDestFmtp->GetBwList() != 0))
        {
            // IR92 release15 EVS Answer Case.
            if (m_bIsOfferReceived == IMS_TRUE)
            {
                // check received EVS SWB only case (category B0, B1, B2 case)
                if (pDestFmtp->GetBwList() == 0x04)
                {
                    if (pDestFmtp->GetBrList() == 0x10)  // B0 received case.
                    {
                        IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Config B0 Type Nego", 0,
                                0, 0);
                        *nNegoBwList = pSrcFmtp->GetBwList() & pDestFmtp->GetBwList();
                    }
                    else if (pSrcFmtp->GetBwList() != 0x04)
                    {  // own EVS category is A. Do not negotiate with received category B.
                        IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Not Config B Type Nego",
                                0, 0, 0);
                        return IMS_FALSE;
                    }
                    else
                    {
                        IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Config B Type Nego", 0,
                                0, 0);
                        *nNegoBwList = pDestFmtp->GetBwList();
                    }
                }
                else  // received EVS category A case
                {
                    // TODO - 20220415 - Need to implement this requirement later
                    /*// except for VZW operator. (only supported category B0 case.)
                    if (IMS_OPERATOR(VZW, GetSlotId()) == IMS_TRUE)
                    {
                        *nNegoBwList = pSrcFmtp->GetBwList() & pDestFmtp->GetBwList();
                    }
                    else // GSMA IR92 case
                    {
                        // check own EVS SWB only capa. (category B0, B1, B2)
                        if (pSrcFmtp->GetBwList() == 0x04)
                        {
                            // this case, Do not negotiate with own category B. check next paylaod.
                            IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - \
                                    not support B Type Nego", 0, 0, 0);
                            return IMS_FALSE;
                        }
                        else
                        {
                            *nNegoBwList = pSrcFmtp->GetBwList() & pDestFmtp->GetBwList();
                        }
                    }*/
                    // check own EVS SWB only capa. (category B0, B1, B2)
                    if (pSrcFmtp->GetBwList() == 0x04)
                    {
                        // this case, Do not negotiate with own category B. check next paylaod.
                        IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - \
                                not support B Type Nego",
                                0, 0, 0);
                        return IMS_FALSE;
                    }
                    else
                    {
                        *nNegoBwList = pSrcFmtp->GetBwList() & pDestFmtp->GetBwList();
                    }
                }
            }
            else
            {
                // TODO - 20220415 - Need to implement this requirement later
                /*// IR92 release15 EVS Answer Received Case.
                if (IMS_OPERATOR(VZW, GetSlotId()) == IMS_TRUE)
                {
                    *nNegoBwList = pSrcFmtp->GetBwList() & pDestFmtp->GetBwList();
                }
                else
                {
                    if (pSrcFmtp->GetBwList() == 0x04 && pDestFmtp->GetBwList() != 0x04)
                    {
                        IMS_TRACE_D("CompareEvsBwBrMode() - check next payload", 0, 0, 0);
                        return IMS_FALSE;
                    }
                    *nNegoBwList = pSrcFmtp->GetBwList() & pDestFmtp->GetBwList();
                }*/
                if (pSrcFmtp->GetBwList() == 0x04 && pDestFmtp->GetBwList() != 0x04)
                {
                    IMS_TRACE_D("CompareEvsBwBrMode() - check next payload", 0, 0, 0);
                    return IMS_FALSE;
                }
                *nNegoBwList = pSrcFmtp->GetBwList() & pDestFmtp->GetBwList();
            }

            if (*nNegoBwList == 0)
            {
                IMS_TRACE_D(
                        "CompareEvsBwBrMode() - Primary Mode - Bandwidth Not Matched...", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        // uni direction received case
        else if ((pSrcFmtp->GetBwList() != 0) &&
                ((pDestFmtp->GetBwRecv() != 0) || (pDestFmtp->GetBwSend() != 0)))
        {
            if (pDestFmtp->GetBwRecv() == 0)
            {
                pDestFmtp->SetBwRecv(0x0f);
            }
            if (pDestFmtp->GetBwSend() == 0)
            {
                pDestFmtp->SetBwSend(0x0f);
            }

            IMS_UINT32 nPeerBWList = pDestFmtp->GetBwRecv() & pDestFmtp->GetBwSend();

            *nNegoBwList = pSrcFmtp->GetBwList() & nPeerBWList;
            if (*nNegoBwList == 0)
            {
                IMS_TRACE_D(
                        "CompareEvsBwBrMode() - Primary Mode - Bandwidth Not Matched...", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoBwList = pSrcFmtp->GetBwList() | pDestFmtp->GetBwList();
        }

        // 02. check Bitrate

        if ((pSrcFmtp->GetBrList() == 0) && (pDestFmtp->GetBrList() == 0))
        {
            *nNegoBrList = 0;
        }
        else if ((pSrcFmtp->GetBrList() != 0) && (pDestFmtp->GetBrList() != 0))
        {
            // IR92 release15 EVS Answer Case.
            if (m_bIsOfferReceived == IMS_TRUE)
            {
                *nNegoBrList = pSrcFmtp->GetBrList() & pDestFmtp->GetBrList();
                if ((*nNegoBwList == 0x04) && (*nNegoBrList == 0x10))
                {  // 13.2kbsp swb only case
                    IMS_TRACE_D("CompareEvsBwBrMode() - Primary Mode - Config B0,B1 Type Nego", 0,
                            0, 0);
                    *nNegoBrList = (pDestFmtp->GetBrList()) & 0x1f;  // negotiated ~13.2kbps
                }
            }
            else
            {
                *nNegoBrList = pSrcFmtp->GetBrList() & pDestFmtp->GetBrList();
            }

            if (*nNegoBrList == 0)
            {
                IMS_TRACE_D(
                        "CompareEvsBwBrMode() - Primary Mode - Bitrate Not Matched...", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        // uni direction received case
        else if ((pSrcFmtp->GetBrList() != 0) &&
                (pDestFmtp->GetBrRecv() != 0 || (pDestFmtp->GetBrSend() != 0)))
        {
            if (pDestFmtp->GetBrRecv() == 0)
            {
                pDestFmtp->SetBrRecv(0x0fff);
            }
            if (pDestFmtp->GetBrSend() == 0)
            {
                pDestFmtp->SetBrSend(0x0fff);
            }

            IMS_UINT32 nPeerBRList = pDestFmtp->GetBrRecv() & pDestFmtp->GetBrSend();

            *nNegoBrList = pSrcFmtp->GetBrList() & nPeerBRList;

            if (*nNegoBrList == 0)
            {
                IMS_TRACE_D(
                        "CompareEvsBwBrMode() - Primary Mode - Bitrate Not Matched...", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoBrList = pSrcFmtp->GetBrList() | pDestFmtp->GetBrList();
        }

        // in case of EVS Primary Mode, dest modeset list added
        if (pDestFmtp->GetModeSetList() != 0)
        {
            *nNegoModeList = pDestFmtp->GetModeSetList();
        }
    }
    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AudioSdpNegotiator::CompareEvsBwBrModeLegacy(IN AudioProfile::EvsFmtp* pSrcFmtp,
        IN AudioProfile::EvsFmtp* pDestFmtp, OUT IMS_UINT32* nNegoBwList,
        OUT IMS_UINT32* nNegoBrList, OUT IMS_UINT32* nNegoModeList)
{
    IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Src BW[0x%04x], BR[0x%04x], modeSet[0x%04x]",
            pSrcFmtp->GetBwList(), pSrcFmtp->GetBrList(), pSrcFmtp->GetModeSetList());
    IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Dest BW[0x%04x], BR[0x%04x], modeSet[0x%04x]",
            pDestFmtp->GetBwList(), pDestFmtp->GetBrList(), pDestFmtp->GetModeSetList());

    // check EVS ModeSwitch
    if (pDestFmtp->GetEvsModeSwitch() == 1)  // AMR IO Mode
    {
        // Set AMR ModeSet lis.
        if ((pSrcFmtp->GetModeSetList() == 0) && (pDestFmtp->GetModeSetList() == 0))
        {
            *nNegoModeList = 0;
        }
        else if ((pSrcFmtp->GetModeSetList() != 0) && (pDestFmtp->GetModeSetList() != 0))
        {
            *nNegoModeList = pSrcFmtp->GetModeSetList() & pDestFmtp->GetModeSetList();

            if (*nNegoModeList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - AMR IO Mode - ModeSet Not Matched...", 0,
                        0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoModeList = pSrcFmtp->GetModeSetList() | pDestFmtp->GetModeSetList();
        }

        // in case of EVS AMR IO Mode, dest bw/br list added
        {
            *nNegoBwList = pDestFmtp->GetBwList();
            *nNegoBrList = pDestFmtp->GetBrList();
        }
    }
    else  // Primary Mode
    {
        // Set Bandwidth/Bitrate list.
        // 01. check Bandwidth
        if ((pSrcFmtp->GetBwList() == 0) && (pDestFmtp->GetBwList() == 0))
        {
            *nNegoBwList = 0;
        }
        else if ((pSrcFmtp->GetBwList() != 0) && (pDestFmtp->GetBwList() != 0))
        {
            *nNegoBwList = pSrcFmtp->GetBwList() & pDestFmtp->GetBwList();

            if (*nNegoBwList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Primary Mode - Bandwidth Not Matched...",
                        0, 0, 0);
                return IMS_FALSE;
            }
        }
        // uni direction received case
        else if ((pSrcFmtp->GetBwList() != 0) &&
                ((pDestFmtp->GetBwRecv() != 0) || (pDestFmtp->GetBwSend() != 0)))
        {
            if (pDestFmtp->GetBwRecv() == 0)
            {
                pDestFmtp->SetBwRecv(0x0f);
            }
            if (pDestFmtp->GetBwSend() == 0)
            {
                pDestFmtp->SetBwSend(0x0f);
            }

            IMS_UINT32 nPeerBWList = pDestFmtp->GetBwRecv() & pDestFmtp->GetBwSend();

            *nNegoBwList = pSrcFmtp->GetBwList() & nPeerBWList;
            if (*nNegoBwList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Primary Mode - Bandwidth Not Matched...",
                        0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoBwList = pSrcFmtp->GetBwList() | pDestFmtp->GetBwList();
        }

        // 02. check Bitrate

        if ((pSrcFmtp->GetBrList() == 0) && (pDestFmtp->GetBrList() == 0))
        {
            *nNegoBrList = 0;
        }
        else if ((pSrcFmtp->GetBrList() != 0) && (pDestFmtp->GetBrList() != 0))
        {
            *nNegoBrList = pSrcFmtp->GetBrList() & pDestFmtp->GetBrList();

            if (*nNegoBrList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Primary Mode - Bitrate Not Matched...", 0,
                        0, 0);
                return IMS_FALSE;
            }
        }
        // uni direction received case
        else if ((pSrcFmtp->GetBrList() != 0) &&
                (pDestFmtp->GetBrRecv() != 0 || (pDestFmtp->GetBrSend() != 0)))
        {
            if (pDestFmtp->GetBrRecv() == 0)
            {
                pDestFmtp->SetBrRecv(0x0fff);
            }
            if (pDestFmtp->GetBrSend() == 0)
            {
                pDestFmtp->SetBrSend(0x0fff);
            }

            IMS_UINT32 nPeerBRList = pDestFmtp->GetBrRecv() & pDestFmtp->GetBrSend();

            *nNegoBrList = pSrcFmtp->GetBrList() & nPeerBRList;

            if (*nNegoBrList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy() - Primary Mode - Bitrate Not Matched...", 0,
                        0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoBrList = pSrcFmtp->GetBrList() | pDestFmtp->GetBrList();
        }

        // in case of EVS Primary Mode, dest modeset list added
        if (pDestFmtp->GetModeSetList() != 0)
        {
            *nNegoModeList = pDestFmtp->GetModeSetList();
        }
    }
    return IMS_TRUE;
}

PRIVATE IMS_SINT32 AudioSdpNegotiator::FindPayloadIndexFromProfile(IN const AString& strCodecName,
        IN AudioProfile* pLocalProfile, IN AudioProfile::Payload* pDstPayload)
{
    if (pLocalProfile == IMS_NULL || pDstPayload == IMS_NULL)
    {
        return -1;
    }

    IMS_SINT32 nRetIndex = -1;

    IMS_TRACE_D("FindPayloadIndexFromProfile()", 0, 0, 0);

    nRetIndex = FindMatchedPayloadIndexFromProfile(
            strCodecName, pLocalProfile, pDstPayload, RETURN_MODE_MATCHED);

    IMS_TRACE_D("FindPayloadIndexFromProfile() the 1st nRetIndex: %d", nRetIndex, 0, 0);

    if (nRetIndex == -1)
    {
        nRetIndex = FindMatchedPayloadIndexFromProfile(
                strCodecName, pLocalProfile, pDstPayload, RETURN_MODE_SIMILAR);
        IMS_TRACE_D("FindPayloadIndexFromProfile() the 2nd nRetIndex: %d", nRetIndex, 0, 0);
    }

    return nRetIndex;
}

PRIVATE IMS_SINT32 AudioSdpNegotiator::FindMatchedPayloadIndexFromProfile(
        IN const AString& strCodecName, IN AudioProfile* pLocalProfile,
        IN AudioProfile::Payload* pDstPayload, IN IMS_BOOL bReturnMode)
{
    IMS_SINT32 nTempIndex = -1;

    for (IMS_UINT32 legacyCheck = 0; legacyCheck < EVS_NEGO_RETRY_COUNT; legacyCheck++)
    {
        for (IMS_UINT32 i = 0; i < pLocalProfile->GetPayloadList().GetSize(); i++)
        {
            AudioProfile::Payload* comparedPayload = pLocalProfile->GetPayloadAt(i);

            if (comparedPayload == IMS_NULL)
            {
                continue;
            }

            if (strCodecName.EqualsIgnoreCase("AMR") || strCodecName.EqualsIgnoreCase("AMR-WB"))
            {
                if (legacyCheck >= 1)
                {
                    continue;
                }
                if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR") ||
                        comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
                {
                    IMS_UINT32 pnNegoModeSetList = 0;
                    IMS_UINT32 pnNegoDefaultRtpModeSet = 0;
                    AudioProfile::AmrFmtp* pCompareFmtp =
                            (AudioProfile::AmrFmtp*)comparedPayload->GetFmtp();
                    AudioProfile::AmrFmtp* pReceivedFmtp =
                            (AudioProfile::AmrFmtp*)pDstPayload->GetFmtp();

                    if (pCompareFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
                    {
                        continue;
                    }

                    if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                                pDstPayload->GetRtpMap().GetPayloadType()) == IMS_FALSE)
                    {
                        continue;
                    }
                    if (comparedPayload->GetRtpMap().GetSamplingRate() !=
                            pDstPayload->GetRtpMap().GetSamplingRate())
                    {
                        continue;
                    }

                    // TODO - 20220415 - Need to implement this requirement later
                    /*if (IMS_OPERATOR(SKT, GetSlotId()) && m_bIsOfferReceived == IMS_TRUE)
                    {
                        IMS_TRACE_D("FindPayloadIndexFromProfile - ignore AMR octet align case",
                                0, 0, 0);
                    }
                    else
                    {
                        if (pCompareFmtp->GetOctetAlign() != pReceivedFmtp->GetOctetAlign() )
                        {
                            continue;
                        }
                    }*/
                    if (pCompareFmtp->GetOctetAlign() != pReceivedFmtp->GetOctetAlign())
                    {
                        continue;
                    }

                    // 20160517 Fix for AMR Open Offer
                    // In case of MO, mode-set from MT could be mismatched
                    // => Keep negotiated mode-set and try to find exact one

                    IMS_SINT32 nCompareResult = CompareModeSet(pCompareFmtp, pReceivedFmtp,
                            bReturnMode, &pnNegoModeSetList, &pnNegoDefaultRtpModeSet);
                    if (nCompareResult == -1)
                    {
                        continue;  // mismatched
                    }
                    else if (nCompareResult == 0)  // similarly matched
                    {
                        IMS_TRACE_D("isFinal: %d", bReturnMode, 0, 0);
                        if (nTempIndex == -1 && bReturnMode == RETURN_MODE_SIMILAR)
                        {
                            nTempIndex = i;
                            IMS_TRACE_I("FindMatchedPayloadIndexFromProfile() Found Similar AMR "
                                        "at[%d], Codec[%s], OctetAlign[%d]",
                                    i, comparedPayload->GetRtpMap().GetPayloadType().GetStr(),
                                    pCompareFmtp->GetOctetAlign());
                            IMS_TRACE_I("FindMatchedPayloadIndexFromProfile() Local/Peer is not "
                                        "exactly matched[0x%04x][0x%04x] =>[0x%04x]. Try next",
                                    pCompareFmtp->GetModeSetList(), pReceivedFmtp->GetModeSetList(),
                                    pnNegoModeSetList);
                        }
                        continue;
                    }
                    else  // exactly matched
                    {
                        IMS_TRACE_D("FindMatchedPayloadIndexFromProfile() Found AMR at[%d], "
                                    "Codec[%s], OctetAlign[%d]",
                                i, comparedPayload->GetRtpMap().GetPayloadType().GetStr(),
                                pCompareFmtp->GetOctetAlign());

                        return i;
                    }
                }
            }
            else if (strCodecName.EqualsIgnoreCase("EVS"))
            {
                if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
                {
                    IMS_UINT32 nBandwidthNegoList;
                    IMS_UINT32 nBitrateNegoList;
                    IMS_UINT32 nModeSetNegoList;
                    AudioProfile::EvsFmtp* pCompareFmtp =
                            (AudioProfile::EvsFmtp*)comparedPayload->GetFmtp();
                    AudioProfile::EvsFmtp* pReceivedFmtp =
                            (AudioProfile::EvsFmtp*)pDstPayload->GetFmtp();

                    if (pCompareFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
                    {
                        continue;
                    }

                    if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                                pDstPayload->GetRtpMap().GetPayloadType()) == IMS_FALSE)
                    {
                        continue;
                    }
                    // need to return nego BW and BR, ModeSet(AMR IO Mode.). also default values...
                    if (legacyCheck == 0)
                    {
                        // IR92 rel15 EVS Br/Bw check.
                        if (CompareEvsBwBrMode(pCompareFmtp, pReceivedFmtp, &nBandwidthNegoList,
                                    &nBitrateNegoList, &nModeSetNegoList) == IMS_FALSE)
                        {
                            continue;
                        }
                    }
                    else
                    {
                        // legacy EVS BR/BW check
                        if (CompareEvsBwBrModeLegacy(pCompareFmtp, pReceivedFmtp,
                                    &nBandwidthNegoList, &nBitrateNegoList,
                                    &nModeSetNegoList) == IMS_FALSE)
                        {
                            continue;
                        }
                    }

                    // TODO - 20220415 - Need to implement this requirement later
                    /*if (IMS_OPERATOR(VZW, GetSlotId()) &&
                            ((VZWProperty::GetInstance()->GetTestMask()
                            & TEST_MASK_UNLOCK_EVS_NEGO_LIMIT) == 0))
                    {
                        //let channel aware mode option to be a negotitation item
                        if (pReceivedFmtp->GetBwList() == 1)
                        {
                            continue;      //vzw req. if offer is nb only, no evs negotiation
                        }
                        if (pDstPayload->GetRtpMap().GetPayloadNumber() !=
                                comparedPayload->GetRtpMap().GetPayloadNumber())
                        {
                            continue;     //vzw req. payload number based negotiation
                        }
                    }*/

                    IMS_TRACE_D("FindMatchedPayloadIndexFromProfile() Found EVS at[%d], "
                                "Codec[%s],EvsModeSwitch[%d]",
                            i, comparedPayload->GetRtpMap().GetPayloadType().GetStr(),
                            pCompareFmtp->GetEvsModeSwitch());

                    return i;
                }
            }
            //[G711]
            else if (strCodecName.EqualsIgnoreCase("PCMU") || strCodecName.EqualsIgnoreCase("PCMA"))
            {
                if (legacyCheck >= 1)
                {
                    continue;
                }
                if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                            pDstPayload->GetRtpMap().GetPayloadType()))
                {
                    IMS_TRACE_D("FindMatchedPayloadIndexFromProfile() Found G.711[%s] Found at[%d]",
                            comparedPayload->GetRtpMap().GetPayloadType().GetStr(), i, 0);

                    return i;
                }
            }
        }
    }

    // Fix for AMR Open Offer
    if (nTempIndex > -1)
    {
        IMS_TRACE_D(
                "FindMatchedPayloadIndexFromProfile() Found Similar AMR at[%d]", nTempIndex, 0, 0);
        return nTempIndex;
    }

    return -1;
}

PRIVATE
MEDIA_DIRECTION AudioSdpNegotiator::UpdateDirectionToMine(
        IN MEDIA_DIRECTION ePeerDir, IN MEDIA_DIRECTION eSrcDir)
{
    if (ePeerDir < MEDIA_DIRECTION_INACTIVE || ePeerDir > MEDIA_DIRECTION_SEND_RECEIVE)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_TRACE_D("UpdateDirectionToMine() ePeerDir[%d], eSrcDir[%d]", ePeerDir, eSrcDir, 0);

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

    if (m_bIsOfferReceived == IMS_FALSE)
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
