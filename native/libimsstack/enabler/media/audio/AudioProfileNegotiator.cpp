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

#include "audio/AudioProfileNegotiator.h"

#include <algorithm>
#include <array>

#include "MediaProfileUtil.h"
#include "ServiceTrace.h"
#include "audio/AudioDef.h"
#include "audio/AudioProfileUtil.h"

#define EVS_NEGO_RETRY_COUNT 2
#define RETURN_MODE_MATCHED  IMS_FALSE
#define RETURN_MODE_SIMILAR  IMS_TRUE

__IMS_TRACE_TAG_MEDIA__;

PUBLIC AudioProfileNegotiator::AudioProfileNegotiator() :
        MediaProfileNegotiator(MEDIA_TYPE_AUDIO)
{
    IMS_TRACE_I("+AudioProfileNegotiator()", 0, 0, 0);
}

PUBLIC VIRTUAL AudioProfileNegotiator::~AudioProfileNegotiator()
{
    IMS_TRACE_I("~AudioProfileNegotiator()", 0, 0, 0);
}

PUBLIC
IMS_BOOL AudioProfileNegotiator::Negotiate(IN AudioProfile* pLocalProfile,
        IN AudioProfile* pPeerProfile, IN IMS_BOOL bIsOfferReceived,
        OUT AudioProfile* pNegotiatedProfile, IN MediaConfiguration* pConfig)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL ||
            pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "Negotiate(): invalid argument", 0, 0, 0);
        return IMS_FALSE;
    }

    m_bIsOfferReceived = bIsOfferReceived;

    IMS_TRACE_I("Negotiate(): IsOfferReceived[%d]", m_bIsOfferReceived, 0, 0);

    NegotiateIpPort(pLocalProfile, pPeerProfile, pNegotiatedProfile);

    if (pPeerProfile->GetPayloadListSize() == 0)
    {
        IMS_TRACE_I("Negotiate(): empty peer payload list", 0, 0, 0);
        ResetNegotiatedProfile(IMS_FALSE, pLocalProfile, pPeerProfile,
                reinterpret_cast<MediaBaseProfile**>(&pNegotiatedProfile));
    }
    else
    {
        auto pNegotiatedPayload = NegotiatePayload(pLocalProfile, pPeerProfile, pNegotiatedProfile);

        if (pNegotiatedPayload == IMS_NULL)
        {
            IMS_TRACE_D("Negotiate(): null negotiated payload", 0, 0, 0);
            ResetNegotiatedProfile(IMS_FALSE, pLocalProfile, pPeerProfile,
                    reinterpret_cast<MediaBaseProfile**>(&pNegotiatedProfile));
            return IMS_FALSE;
        }

        if (!NegotiateDirection(pLocalProfile, pPeerProfile, pNegotiatedProfile))
        {
            IMS_TRACE_D("Negotiate(): fail to negotiate the direction", 0, 0, 0);
            ResetNegotiatedProfile(IMS_FALSE, pLocalProfile, pPeerProfile,
                    reinterpret_cast<MediaBaseProfile**>(&pNegotiatedProfile));
            return IMS_FALSE;
        }

        NegotiateRtcpXr(pLocalProfile, pNegotiatedProfile);
        NegotiatePtime(pNegotiatedProfile, pLocalProfile->GetPtime());
        NegotiateMaxPtime(pNegotiatedProfile, pLocalProfile->GetMaxPtime());
        NegotiateAnbr(pLocalProfile->IsAnbrSupported(), pPeerProfile->IsAnbrSupported(),
                pNegotiatedProfile);
        pNegotiatedProfile->SetCandidateAttr(pLocalProfile->GetCandidateAttr());
        NegotiateBandwidth(
                pLocalProfile, pPeerProfile, pNegotiatedProfile, pNegotiatedPayload, pConfig);
        NegotiateRtcpInterval(pNegotiatedProfile, pConfig->GetRtcpIntervalOnHold(),
                pConfig->GetRtcpIntervalOnActive());
    }

    if (pNegotiatedProfile->GetDataPort() == 0)
    {
        pNegotiatedProfile->SetDirection(MEDIA_DIRECTION_INACTIVE);
    }

    IMS_TRACE_D("Negotiate(): negotiated payload size[%d], port[%d], direction[%d], ",
            pNegotiatedProfile->GetPayloadListSize(), pNegotiatedProfile->GetDataPort(),
            pNegotiatedProfile->GetDirection());

    return IMS_TRUE;
}

PRIVATE
AudioProfile::Payload* AudioProfileNegotiator::NegotiatePayload(IN AudioProfile* pLocalProfile,
        IN AudioProfile* pPeerProfile, IN AudioProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiatePayload(): invalid arguments", 0, 0, 0);
        return IMS_NULL;
    }

    auto pNegotiatedPayload =
            NegotiateAudioPayload(pLocalProfile, pPeerProfile, pNegotiatedProfile);

    if (pNegotiatedPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiatePayload(): invalid payload", 0, 0, 0);
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

    if (!bTelephoneEventPayloadNegotiated)
    {
        NegotiateTelephoneEvent8000Payload(
                nNegotiatedSamplingRate, pPeerProfile, pNegotiatedProfile);
    }

    return pNegotiatedPayload;
}

PRIVATE
AudioProfile::Payload* AudioProfileNegotiator::NegotiateAudioPayload(IN AudioProfile* pLocalProfile,
        IN AudioProfile* pPeerProfile, IN AudioProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateAudioPayload(): invalid arguments", 0, 0, 0);
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < pPeerProfile->GetPayloadListSize(); i++)
    {
        AudioProfile::Payload* pPeerPayload = pPeerProfile->GetPayloadAt(i);
        if (pPeerPayload == IMS_NULL)
        {
            continue;
        }

        if (pPeerPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR") ||
                pPeerPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
        {
            IMS_UINT32 nNegoModeSetList = 0;
            IMS_UINT32 nNegoDefaultRtpModeSet = 0;

            if (FindAmrInProfile(
                        pLocalProfile, pPeerPayload, &nNegoModeSetList, &nNegoDefaultRtpModeSet))
            {
                return NegotiateAmr(pLocalProfile, pPeerProfile, pNegotiatedProfile, i,
                        nNegoModeSetList, nNegoDefaultRtpModeSet);
            }
        }
        else if (pPeerPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
        {
            if (FindMatchingEvsPayload(pLocalProfile, pPeerPayload))
            {
                pPeerProfile->SetNegotiatedPayloadIndex(i);
                return NegotiateEvs(
                        pLocalProfile->GetPayloadAt(pLocalProfile->GetNegotiatedPayloadIndex()),
                        pPeerPayload, pNegotiatedProfile);
            }
        }
        else if (pPeerPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMU") ||
                pPeerPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMA"))
        {
            if (FindPcmInProfile(pLocalProfile, pPeerPayload))
            {
                return NegotiatePcm(pLocalProfile, pPeerProfile, pNegotiatedProfile, i);
            }
        }
    }

    return IMS_NULL;
}

PRIVATE
AudioProfile::Payload* AudioProfileNegotiator::NegotiateAmr(IN AudioProfile* pLocalProfile,
        IN AudioProfile* pPeerProfile, IN AudioProfile* pNegotiatedProfile,
        IN IMS_UINT32 nPayloadIndex, IN IMS_UINT32 nNegoModeSetList,
        IN IMS_UINT32 nNegoDefaultRtpModeSet)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateAmr(): invalid arguments", 0, 0, 0);
        return IMS_NULL;
    }

    IMS_TRACE_D("NegotiateAmr(): payload index[%d]", nPayloadIndex, 0, 0);

    AudioProfile::Payload* pAmr = new AudioProfile::Payload();
    AudioProfile::Payload* pPeerPayload = pPeerProfile->GetPayloadAt(nPayloadIndex);

    if (pPeerPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateAmr(): invalid payload", 0, 0, 0);
        delete pAmr;
        return IMS_NULL;
    }

    pAmr->SetRtpMap(pPeerPayload->GetRtpMap());

    IMS_SINT32 nLocalPayloadIndex = -1;
    std::shared_ptr<AudioProfile::AmrFmtp> pAmrFmtp = NegotiateAmrFmtp(pLocalProfile, pPeerPayload,
            nNegoModeSetList, nNegoDefaultRtpModeSet, nLocalPayloadIndex);

    pAmr->SetFmtp(pAmrFmtp);
    pNegotiatedProfile->AddPayload(pAmr);

    if (pPeerProfile->GetNegotiatedPayloadIndex() == -1)
    {
        // Set the index of negotiated payload from the list.
        pPeerProfile->SetNegotiatedPayloadIndex(nPayloadIndex);
        // Set remote payload index at local profile
        pLocalProfile->SetNegotiatedPayloadIndex(nLocalPayloadIndex);

        // MT case : change src PT# to dest PT#
        if (m_bIsOfferReceived && pLocalProfile->GetNegotiatedPayloadIndex() != -1)
        {
            AudioProfile::Payload* pTempNegoSrcPayload =
                    pLocalProfile->GetPayloadAt(pLocalProfile->GetNegotiatedPayloadIndex());

            pTempNegoSrcPayload->GetRtpMap().SetPayloadNumber(
                    pPeerPayload->GetRtpMap().GetPayloadNumber());
        }
    }

    if (pNegotiatedProfile->GetNegotiatedPayloadIndex() == -1)
    {
        pNegotiatedProfile->SetNegotiatedPayloadIndex(pNegotiatedProfile->GetPayloadListSize() - 1);
    }

    return pAmr;
}

PRIVATE
std::shared_ptr<AudioProfile::AmrFmtp> AudioProfileNegotiator::NegotiateAmrFmtp(
        IN AudioProfile* pLocalProfile, IN AudioProfile::Payload* pPeerPayload,
        IN IMS_UINT32 nNegoModeSetList, IN IMS_UINT32 nNegoDefaultRtpModeSet,
        OUT IMS_SINT32& nLocalPayloadIndex)
{
    if (pLocalProfile == IMS_NULL || pPeerPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateAmrFmtp(): invalid arguments", 0, 0, 0);
        return IMS_NULL;
    }

    nLocalPayloadIndex = FindPayloadIndexFromProfile(
            pPeerPayload->GetRtpMap().GetPayloadType(), pLocalProfile, pPeerPayload);

    auto pLocalFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(
            pLocalProfile->GetPayloadAt(nLocalPayloadIndex)->GetFmtp());
    auto pAmrFmtp = std::make_shared<AudioProfile::AmrFmtp>(
            *std::static_pointer_cast<AudioProfile::AmrFmtp>(pPeerPayload->GetFmtp()));

    pAmrFmtp->SetModeSetList(nNegoModeSetList);
    pAmrFmtp->SetDefaultRtpModeSet(nNegoDefaultRtpModeSet);
    pAmrFmtp->SetVisibleModeSet(
            pAmrFmtp->GetModeSetList() != 0 ? IMS_TRUE : pAmrFmtp->IsModeSetVisible());
    pAmrFmtp->SetDtx(pLocalFmtp->IsDtxEnabled());
    pAmrFmtp->SetVisibleModeChangeCapability(pLocalFmtp->IsModeChangeCapabilityVisible());
    pAmrFmtp->SetModeChangeCapability(pLocalFmtp->GetModeChangeCapability());
    pAmrFmtp->SetVisibleModeChangeNeighbor(pLocalFmtp->IsModeChangeNeighborVisible());
    pAmrFmtp->SetModeChangeNeighbor(pLocalFmtp->GetModeChangeNeighbor());
    pAmrFmtp->SetVisibleModeChangePeriod(pLocalFmtp->IsModeChangePeriodVisible());
    pAmrFmtp->SetModeChangePeriod(pLocalFmtp->GetModeChangePeriod());
    pAmrFmtp->SetOctetAlign(pLocalFmtp->GetOctetAlign());
    pAmrFmtp->SetVisibleOctetAlign(
            pLocalFmtp->GetOctetAlign() == 1 ? IMS_TRUE : pLocalFmtp->IsOctetAlignVisible());

    return pAmrFmtp;
}

PRIVATE
AudioProfile::Payload* AudioProfileNegotiator::NegotiateEvs(IN AudioProfile::Payload* pLocalPayload,
        IN AudioProfile::Payload* pPeerPayload, IN AudioProfile* pNegotiatedProfile)
{
    if (pLocalPayload == IMS_NULL || pPeerPayload == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateEvs(): invalid arguments", 0, 0, 0);
        return IMS_NULL;
    }

    std::shared_ptr<AudioProfile::EvsFmtp> pLocalFmtp =
            std::static_pointer_cast<AudioProfile::EvsFmtp>(pLocalPayload->GetFmtp());
    std::shared_ptr<AudioProfile::EvsFmtp> pPeerFmtp =
            std::static_pointer_cast<AudioProfile::EvsFmtp>(pPeerPayload->GetFmtp());

    auto pNegotiatedPayload = new AudioProfile::Payload();
    pNegotiatedPayload->SetRtpMap(pPeerPayload->GetRtpMap());

    if (pPeerFmtp == IMS_NULL)
    {
        IMS_TRACE_I("NegotiateEvsFmtp(): peer fmtp is invalid, use local fmtp", 0, 0, 0);
        pNegotiatedPayload->SetFmtp(pLocalFmtp);
        pNegotiatedProfile->AddPayload(pNegotiatedPayload);
        return pNegotiatedPayload;
    }

    IMS_UINT32 nBandwidthNegoList, nBitrateNegoList, nModeSetNegoList;
    IMS_BOOL bModeMatched = CompareEvsBwBrMode(
            pLocalFmtp, pPeerFmtp, &nBandwidthNegoList, &nBitrateNegoList, &nModeSetNegoList);

    if (!bModeMatched)
    {
        bModeMatched = CompareEvsBwBrModeLegacy(
                pLocalFmtp, pPeerFmtp, &nBandwidthNegoList, &nBitrateNegoList, &nModeSetNegoList);
    }

    if (!bModeMatched)
    {
        IMS_TRACE_E(0, "NegotiateEvs(): no matching fmtp", 0, 0, 0);
        delete pNegotiatedPayload;
        return IMS_NULL;
    }

    std::shared_ptr<AudioProfile::EvsFmtp> pEvsFmtp = NegotiateEvsFmtp(
            pLocalPayload, pPeerPayload, nBandwidthNegoList, nBitrateNegoList, nModeSetNegoList);

    pNegotiatedPayload->SetFmtp(pEvsFmtp);
    pNegotiatedProfile->AddPayload(pNegotiatedPayload);

    if (pNegotiatedProfile->GetNegotiatedPayloadIndex() == -1)
    {
        pNegotiatedProfile->SetNegotiatedPayloadIndex(pNegotiatedProfile->GetPayloadListSize() - 1);
    }

    return pNegotiatedPayload;
}
PRIVATE
std::shared_ptr<AudioProfile::EvsFmtp> AudioProfileNegotiator::NegotiateEvsFmtp(
        IN AudioProfile::Payload* pLocalPayload, IN AudioProfile::Payload* pPeerPayload,
        IN IMS_UINT32 nBandwidthNegoList, IN IMS_UINT32 nBitrateNegoList,
        IN IMS_UINT32 nModeSetNegoList)
{
    if (pLocalPayload == IMS_NULL || pPeerPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateEvsFmtp(): invalid arguments", 0, 0, 0);
        return IMS_NULL;
    }

    auto pLocalFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pLocalPayload->GetFmtp());
    auto pNegotiatedFmtp = std::make_shared<AudioProfile::EvsFmtp>(
            *std::static_pointer_cast<AudioProfile::EvsFmtp>(pPeerPayload->GetFmtp()));
    pNegotiatedFmtp->SetBwList(nBandwidthNegoList);
    pNegotiatedFmtp->SetBrList(nBitrateNegoList);
    pNegotiatedFmtp->SetModeSetList(nModeSetNegoList);

    IMS_TRACE_D("NegotiateEvsFmtp(): peer DTX[%d], local DTX[%d]", pNegotiatedFmtp->IsDtxEnabled(),
            pLocalFmtp->IsDtxEnabled(), 0);

    if (pNegotiatedFmtp->IsDtxEnabled() != pLocalFmtp->IsDtxEnabled())
    {
        IMS_TRACE_D("NegotiateEvsFmtp(): DTX updated in the destination profile", 0, 0, 0);
    }

    pNegotiatedFmtp->SetVisibleModeChangeCapability(pLocalFmtp->IsModeChangeCapabilityVisible());
    pNegotiatedFmtp->SetModeChangeCapability(pLocalFmtp->GetModeChangeCapability());
    pNegotiatedFmtp->SetVisibleModeChangeNeighbor(pLocalFmtp->IsModeChangeNeighborVisible());
    pNegotiatedFmtp->SetModeChangeNeighbor(pLocalFmtp->GetModeChangeNeighbor());
    pNegotiatedFmtp->SetVisibleModeChangePeriod(pLocalFmtp->IsModeChangePeriodVisible());
    pNegotiatedFmtp->SetModeChangePeriod(pLocalFmtp->GetModeChangePeriod());

    NegotiateUniDirectionBrBw(pNegotiatedFmtp, nBitrateNegoList, nBandwidthNegoList);
    NegotiateCmr(pLocalFmtp, pNegotiatedFmtp);
    NegotiateModeSet(pNegotiatedFmtp);

    return pNegotiatedFmtp;
}

PRIVATE
void AudioProfileNegotiator::NegotiateUniDirectionBrBw(
        OUT std::shared_ptr<AudioProfile::EvsFmtp> pEvsFmtp, IN IMS_UINT32 nBandwidthNegoList,
        IN IMS_UINT32 nBitrateNegoList)
{
    if (pEvsFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateUniDirectionBrBw(): invalid fmtp", 0, 0, 0);
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
void AudioProfileNegotiator::NegotiateCmr(IN std::shared_ptr<AudioProfile::EvsFmtp> pLocalFmtp,
        OUT std::shared_ptr<AudioProfile::EvsFmtp> pEvsFmtp)
{
    if (pLocalFmtp == IMS_NULL || pEvsFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateCmr(): invalid fmtp", 0, 0, 0);
        return;
    }

    pEvsFmtp->SetSendCmr(pLocalFmtp->IsSendCmrEnabled());

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
void AudioProfileNegotiator::NegotiateModeSet(OUT std::shared_ptr<AudioProfile::EvsFmtp> pEvsFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateCmr(): invalid fmtp", 0, 0, 0);
        return;
    }

    /** fixed for IR92 ver.12 new spec as if the selected EVS configuration is A1, B0 or B1
     * then"mode set = 0,1,2" must be included in the SDP answer.*/
    if (m_bIsOfferReceived && pEvsFmtp->GetEvsModeSwitch() != 1)
    {
        // if max BR is 13.2kbps, then set a"mode-set" attribute
        if (((pEvsFmtp->GetBrList() & 0x10) != 0) && ((pEvsFmtp->GetBrList() & 0xFFE0) == 0))
        {
            pEvsFmtp->SetModeSetList(0x07);  // mode-set = 0,1,2;
            pEvsFmtp->SetVisibleModeSet(IMS_TRUE);
            IMS_TRACE_D("NegotiateModeSet(): add EVS mode-set", 0, 0, 0);
        }
    }
}

PRIVATE
AudioProfile::Payload* AudioProfileNegotiator::NegotiatePcm(IN AudioProfile* pLocalProfile,
        IN AudioProfile* pPeerProfile, IN AudioProfile* pNegotiatedProfile,
        IN IMS_UINT32 nPayloadIndex)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiatePcm(): invalid arguments", 0, 0, 0);
        return IMS_NULL;
    }

    IMS_TRACE_D("NegotiatePcm(): payload index[%d]", nPayloadIndex, 0, 0);

    AudioProfile::Payload* pPcm = new AudioProfile::Payload();
    AudioProfile::Payload* pPeerPayload = pPeerProfile->GetPayloadAt(nPayloadIndex);

    if (pPeerPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiatePcm(): invalid payload", 0, 0, 0);
        delete pPcm;
        return IMS_NULL;
    }

    pPcm->SetRtpMap(pPeerPayload->GetRtpMap());
    pNegotiatedProfile->AddPayload(pPcm);

    if (pPeerProfile->GetNegotiatedPayloadIndex() == -1)
    {
        // Set the index of negotiated payload from the list
        pPeerProfile->SetNegotiatedPayloadIndex(nPayloadIndex);
        pLocalProfile->SetNegotiatedPayloadIndex(FindPayloadIndexFromProfile(
                pPeerPayload->GetRtpMap().GetPayloadType(), pLocalProfile, pPeerPayload));
    }

    if (pNegotiatedProfile->GetNegotiatedPayloadIndex() == -1)
    {
        pNegotiatedProfile->SetNegotiatedPayloadIndex(pNegotiatedProfile->GetPayloadListSize() - 1);
    }

    return pPcm;
}

PRIVATE
IMS_BOOL AudioProfileNegotiator::NegotiateTelephoneEventPayload(
        IN IMS_SINT32 nNegotiatedSamplingRate, IN AudioProfile* pPeerProfile,
        OUT AudioProfile* pNegotiatedProfile)
{
    if (nNegotiatedSamplingRate == 0 || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateTelephoneEventPayload(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_D(
            "NegotiateTelephoneEventPayload(): sampling rate[%d]", nNegotiatedSamplingRate, 0, 0);

    for (IMS_UINT32 i = 0; i < pPeerProfile->GetPayloadListSize(); i++)
    {
        AudioProfile::Payload* pPeerPayload = pPeerProfile->GetPayloadAt(i);

        if (pPeerPayload == IMS_NULL)
        {
            continue;
        }

        if (pPeerPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("telephone-event"))
        {
            if (pPeerPayload->GetRtpMap().GetSamplingRate() == nNegotiatedSamplingRate)
            {
                AudioProfile::Payload* pTelephoneEvent = new AudioProfile::Payload();
                pTelephoneEvent->SetRtpMap(pPeerPayload->GetRtpMap());
                pTelephoneEvent->SetFmtp(std::make_shared<AudioProfile::TelephoneEventFmtp>(
                        *std::static_pointer_cast<AudioProfile::TelephoneEventFmtp>(
                                pPeerPayload->GetFmtp())));
                pNegotiatedProfile->AddPayload(pTelephoneEvent);

                IMS_TRACE_D("NegotiateTelephoneEventPayload(): payload[%d]",
                        pTelephoneEvent->GetRtpMap().GetPayloadNumber(), 0, 0);

                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PRIVATE
void AudioProfileNegotiator::NegotiateTelephoneEvent8000Payload(
        IN IMS_SINT32 nNegotiatedSamplingRate, IN AudioProfile* pPeerProfile,
        OUT AudioProfile* pNegotiatedProfile)
{
    if (pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateTelephoneEvent8000Payload(): invalid arguments", 0, 0, 0);
        return;
    }

    IMS_TRACE_D(
            "NegotiateTelephoneEvent8000Payload(): need to accept 8K DTMF, no proper DTMF Payload",
            0, 0, 0);

    for (IMS_UINT32 i = 0; i < pPeerProfile->GetPayloadListSize(); i++)
    {
        AudioProfile::Payload* pPeerPayload = pPeerProfile->GetPayloadAt(i);

        if (pPeerPayload == IMS_NULL)
        {
            continue;
        }
        if (pPeerPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("telephone-event"))
        {
            // for acceptable 8K DTMF when AMR-WB calling
            if (nNegotiatedSamplingRate > pPeerPayload->GetRtpMap().GetSamplingRate())
            {
                IMS_TRACE_D("NegotiateTelephoneEvent8000Payload(): Accept sampling rate[%d]->[%d]",
                        nNegotiatedSamplingRate, pPeerPayload->GetRtpMap().GetSamplingRate(), 0);
                AudioProfile::Payload* pTelephoneEvent = new AudioProfile::Payload();
                pTelephoneEvent->SetRtpMap(pPeerPayload->GetRtpMap());
                pTelephoneEvent->SetFmtp(std::make_shared<AudioProfile::TelephoneEventFmtp>(
                        *std::static_pointer_cast<AudioProfile::TelephoneEventFmtp>(
                                pPeerPayload->GetFmtp())));
                pNegotiatedProfile->AddPayload(pTelephoneEvent);

                return;
            }
        }
    }
}

PRIVATE
IMS_BOOL AudioProfileNegotiator::NegotiateDirection(IN AudioProfile* pLocalProfile,
        IN AudioProfile* pPeerProfile, IN AudioProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateDirection(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    pNegotiatedProfile->SetDirection(
            UpdateDirectionToMine(pPeerProfile->GetDirection(), pLocalProfile->GetDirection()));

    IMS_TRACE_D("NegotiateDirection(): direction[%d]", pNegotiatedProfile->GetDirection(), 0, 0);

    if (pNegotiatedProfile->GetDirection() == MEDIA_DIRECTION_INVALID)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
void AudioProfileNegotiator::NegotiateRtcpXr(
        IN AudioProfile* pLocalProfile, OUT AudioProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateRtcpXr(): invalid arguments", 0, 0, 0);
        return;
    }

    if (pLocalProfile->IsRtcpXrSupported() &&
            pNegotiatedProfile->GetDirection() == MEDIA_DIRECTION_SEND_RECEIVE)
    {
        pNegotiatedProfile->SetSupportRtcpXr(IMS_TRUE);
        pNegotiatedProfile->SetRtcpXrAttr(pLocalProfile->GetRtcpXrAttr());
    }

    IMS_TRACE_D("NegotiateRtcpXr(): support[%d]", pNegotiatedProfile->IsRtcpXrSupported(), 0, 0);
}

PRIVATE
void AudioProfileNegotiator::NegotiatePtime(
        OUT AudioProfile* pNegotiatedProfile, IN IMS_SINT32 nLocalPtime)
{
    if (pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiatePtime(): invalid arguments", 0, 0, 0);
        return;
    }

    IMS_SINT32 nPtime = (nLocalPtime < 20) ? 20 : nLocalPtime;
    pNegotiatedProfile->SetPtime(nPtime);

    IMS_TRACE_D("NegotiatePtime(): Ptime[%d], Local Ptime[%d]", pNegotiatedProfile->GetPtime(),
            nLocalPtime, 0);
}

PRIVATE
void AudioProfileNegotiator::NegotiateMaxPtime(
        OUT AudioProfile* pNegotiatedProfile, IN IMS_SINT32 nLocalMaxPtime)
{
    if (pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateMaxPtime(): invalid arguments", 0, 0, 0);
        return;
    }

    IMS_SINT32 nMaxPtime = (nLocalMaxPtime < 20) ? 240 : nLocalMaxPtime;
    pNegotiatedProfile->SetMaxPtime(nMaxPtime);

    IMS_TRACE_D("NegotiateMaxPtime(): Max Ptime[%d], Local Ptime[%d]",
            pNegotiatedProfile->GetMaxPtime(), nLocalMaxPtime, 0);
}

PRIVATE
void AudioProfileNegotiator::NegotiateAnbr(IN IMS_BOOL nSupportAnbrLocal,
        IN IMS_BOOL nSupportAnbrPeer, OUT AudioProfile* pNegotiatedProfile)
{
    if (pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateAnbr(): invalid arguments", 0, 0, 0);
        return;
    }

    IMS_BOOL bEnableAnbr = (nSupportAnbrLocal && nSupportAnbrPeer);
    pNegotiatedProfile->SetAnbr(bEnableAnbr);

    IMS_TRACE_D("NegotiateAnbr(): supported local[%d], peer[%d], nego[%d]", nSupportAnbrLocal,
            nSupportAnbrPeer, pNegotiatedProfile->IsAnbrSupported());
}

PRIVATE
void AudioProfileNegotiator::NegotiateBandwidth(IN AudioProfile* pLocalProfile,
        IN AudioProfile* pPeerProfile, OUT AudioProfile* pNegotiatedProfile,
        IN AudioProfile::Payload* pNegotiatedPayload, IN MediaConfiguration* pConfig)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL ||
            pNegotiatedPayload == IMS_NULL || pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateBandwidth(): invalid arguments", 0, 0, 0);
        return;
    }

    pNegotiatedProfile->SetBandwidthRs(pPeerProfile->GetBandwidthRs());
    pNegotiatedProfile->SetBandwidthRr(pPeerProfile->GetBandwidthRr());

    IMS_SINT32 nAsValueOfNegotiatedCodec =
            NegotiateAs(pNegotiatedPayload, pNegotiatedProfile->GetIpAddress().IsIPv6Address());

    MakeNegotiatedBandwidth(static_cast<AudioConfiguration*>(pConfig), pLocalProfile, pPeerProfile,
            nAsValueOfNegotiatedCodec, pNegotiatedProfile);

    IMS_TRACE_D("NegotiateBandwidth(): AS[%d], RS[%d], RR[%d]",
            pNegotiatedProfile->GetBandwidthAs(), pNegotiatedProfile->GetBandwidthRs(),
            pNegotiatedProfile->GetBandwidthRr());
}

PRIVATE
IMS_SINT32 AudioProfileNegotiator::NegotiateAs(
        IN AudioProfile::Payload* pNegotiatedPayload, IN IMS_BOOL bIpv6)
{
    if (pNegotiatedPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateAs(): invalid arguments", 0, 0, 0);
        return 0;
    }

    AUDIO_CODEC nCurrCodec;
    IMS_SINT32 nModeSet;
    IMS_SINT32 nAs = 0;

    if (pNegotiatedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR") ||
            pNegotiatedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
    {
        auto pAmrFmtp =
                std::static_pointer_cast<AudioProfile::AmrFmtp>(pNegotiatedPayload->GetFmtp());
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
        auto pEvsFmtp =
                std::static_pointer_cast<AudioProfile::EvsFmtp>(pNegotiatedPayload->GetFmtp());
        nCurrCodec = AUDIO_CODEC_EVS;
        nModeSet = AudioProfileUtil::GetLargestModesetInFmtp("EVS", pNegotiatedPayload);
        nAs = AudioProfileUtil::ConvertToBandwidthAS(
                nCurrCodec, bIpv6, pEvsFmtp->GetEvsModeSwitch(), nModeSet);
    }

    return nAs;
}

PRIVATE
IMS_BOOL AudioProfileNegotiator::MakeNegotiatedBandwidth(IN const AudioConfiguration* pConfig,
        IN AudioProfile* pLocalProfile, IN AudioProfile* pPeerProfile,
        IN IMS_SINT32 nAsValueOfNegotiatedCodec, OUT AudioProfile* pNegotiatedProfile)
{
    IMS_TRACE_D(
            "MakeNegotiatedBandwidth(): nego option[%d]", pConfig->GetBandwidthNegoOption(), 0, 0);

    if (!m_bIsOfferReceived)
    {
        // MO's Bandwidth Setting
        //  1. Set AS Value
        if (pPeerProfile->GetBandwidthAs() > 0)
        {
            if (pPeerProfile->GetBandwidthAs() > nAsValueOfNegotiatedCodec)
            {
                pNegotiatedProfile->SetBandwidthAs(nAsValueOfNegotiatedCodec);
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

            IMS_TRACE_D("MakeNegotiatedBandwidth(): Nego AS[%d] RS[%d] RR[%d]",
                    pLocalProfile->GetBandwidthAs(), pLocalProfile->GetBandwidthRs(),
                    pLocalProfile->GetBandwidthRr());
            return IMS_TRUE;
        }

        // 2.2 Normal Case
        if (pConfig->GetBandwidthNegoOption() == MediaConfiguration::BW_OPTION_REMOTE_VALUE)
        {
            // if RS/RR is used for RTCP remote value
            pNegotiatedProfile->SetBandwidthRs(pPeerProfile->GetBandwidthRs());
            pNegotiatedProfile->SetBandwidthRr(pPeerProfile->GetBandwidthRr());
        }
        else
        {
            // default case (Local value is used instead of remote value.)
            pNegotiatedProfile->SetBandwidthRs(pLocalProfile->GetBandwidthRs());
            pNegotiatedProfile->SetBandwidthRr(pLocalProfile->GetBandwidthRr());
        }
    }
    else
    {
        // MT's Bandwidth Setting
        // 1. Set Negotiated AS Value
        if (nAsValueOfNegotiatedCodec > 0)
        {
            pNegotiatedProfile->SetBandwidthAs(nAsValueOfNegotiatedCodec);

            // if GetBandwidthNegoOption is BW_OPTION_REMOTE_VALUE, use lower AS value
            if ((pConfig->GetBandwidthNegoOption() == MediaConfiguration::BW_OPTION_REMOTE_VALUE) &&
                    (nAsValueOfNegotiatedCodec > pPeerProfile->GetBandwidthAs()) &&
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
            MediaProfileUtil::SetRtcpRsRr(pNegotiatedProfile, pConfig, IMS_TRUE);
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

                IMS_TRACE_D("MakeNegotiatedBandwidth(): AS[%d] RS[%d] RR[%d]",
                        pLocalProfile->GetBandwidthAs(), pLocalProfile->GetBandwidthRs(),
                        pLocalProfile->GetBandwidthRr());
                return IMS_TRUE;
            }

            // 3.2.2 Normal Case
            if (pConfig->GetBandwidthNegoOption() == MediaConfiguration::BW_OPTION_REMOTE_VALUE)
            {
                // if RS/RR is used for RTCP remote  value
                IMS_TRACE_D("MakeNegotiatedBandwidth(): use peer RS[%d] RR[%d]",
                        pPeerProfile->GetBandwidthRs(), pPeerProfile->GetBandwidthRr(), 0);
                pNegotiatedProfile->SetBandwidthRs(pPeerProfile->GetBandwidthRs());
                pNegotiatedProfile->SetBandwidthRr(pPeerProfile->GetBandwidthRr());
            }
            else
            {
                // default case (Local value is used instead of remote value.)
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

PRIVATE
void AudioProfileNegotiator::NegotiateRtcpInterval(IN AudioProfile* pNegotiatedProfile,
        IN IMS_SINT32 nRtcpIntervalOnHold, IN IMS_SINT32 nRtcpIntervalOnActive)
{
    if (pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateRtcpInterval(): invalid arguments", 0, 0, 0);
        return;
    }

    if (pNegotiatedProfile->GetBandwidthRs() == 0 && pNegotiatedProfile->GetBandwidthRr() == 0)
    {
        pNegotiatedProfile->SetRtcpInterval(0);
        IMS_TRACE_D("NegotiateRtcpInterval(): negotiated rs and rr are 0, disable rtcp", 0, 0, 0);
    }
    else
    {
        pNegotiatedProfile->SetRtcpInterval(nRtcpIntervalOnHold);

        if (pNegotiatedProfile->GetDirection() == MEDIA_DIRECTION_SEND_RECEIVE &&
                nRtcpIntervalOnActive > 0)
        {
            pNegotiatedProfile->SetRtcpInterval(nRtcpIntervalOnActive);
        }
    }

    IMS_TRACE_D("NegotiateRtcpInterval(): Rtcp Interval[%d]", pNegotiatedProfile->GetRtcpInterval(),
            0, 0);
}

PRIVATE
IMS_BOOL AudioProfileNegotiator::FindMatchedEvsFmtp(
        IN AudioProfile::Payload* pComparedPayload, IN AudioProfile::Payload* pPeerPayload)
{
    auto pCompareFmtp =
            std::static_pointer_cast<AudioProfile::EvsFmtp>(pComparedPayload->GetFmtp());
    auto pReceivedFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pPeerPayload->GetFmtp());
    if (pCompareFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_UINT32 nBandwidthNegoList, nBitrateNegoList, nModeSetNegoList;
    IMS_BOOL bModeMatched = CompareEvsBwBrMode(
            pCompareFmtp, pReceivedFmtp, &nBandwidthNegoList, &nBitrateNegoList, &nModeSetNegoList);

    if (!bModeMatched)
    {
        bModeMatched = CompareEvsBwBrModeLegacy(pCompareFmtp, pReceivedFmtp, &nBandwidthNegoList,
                &nBitrateNegoList, &nModeSetNegoList);
    }

    if (!bModeMatched)
    {
        return IMS_FALSE;
    }

    // check channel aware mode received param
    if (pReceivedFmtp->GetReceivedChAwRecv() > 0)
    {
        IMS_SINT32 nTempReceivedChAw = pReceivedFmtp->GetReceivedChAwRecv();

        if ((nTempReceivedChAw != 2) && (nTempReceivedChAw != 3) && (nTempReceivedChAw != 5) &&
                (nTempReceivedChAw != 7))
        {
            return IMS_FALSE;
        }
    }

    // fixed for IR92 ver.12 newaly spec as below comment. If ther SDP parameter
    // ch-aw-recv is present in the corresponding received and accepted SDP offer,
    // then the SDP parameter ch-aw-recv must be included in the SDP answer with
    // the same value as received. In Offered case, set ChAw Mode for mine.
    if (((pReceivedFmtp->GetBwList() & 0x06) != 0 && (pReceivedFmtp->GetBrList() & 0x10) != 0) ||
            ((nBandwidthNegoList & 0x06) != 0 && (nBitrateNegoList & 0x10) != 0))
    {
        if (!m_bIsOfferReceived)
        {
            pReceivedFmtp->SetChAwRecv(pCompareFmtp->GetChAwRecv());
            pReceivedFmtp->SetShowChannelAwMode(pCompareFmtp->IsChannelAwModeVisible());
        }
    }

    IMS_TRACE_D("FindMatchingEvsPayload(): Found BandwidthNegoList[0x%04x], "
                "BitrateNegoList[0x%04x]",
            nBandwidthNegoList, nBitrateNegoList, 0);

    IMS_TRACE_D("FindMatchingEvsPayload(): EVS ModeSwitch[%d], "
                "ModeSetNegoList[0x%04x], SendCmr[%d]",
            pReceivedFmtp->GetEvsModeSwitch(), nModeSetNegoList,
            std::static_pointer_cast<AudioProfile::EvsFmtp>(pComparedPayload->GetFmtp())
                    ->IsSendCmrEnabled());

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AudioProfileNegotiator::FindMatchingEvsPayload(
        IN AudioProfile* pLocalProfile, IN AudioProfile::Payload* pPeerPayload)
{
    if (pLocalProfile == IMS_NULL || pPeerPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "FindMatchingEvsPayload(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < pLocalProfile->GetPayloadListSize(); i++)
    {
        AudioProfile::Payload* pLocalPayload = pLocalProfile->GetPayloadAt(i);

        if (pLocalPayload == IMS_NULL)
        {
            continue;
        }

        if (pLocalPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
        {
            if (pLocalPayload->GetRtpMap().GetPayloadNumber() ==
                            pPeerPayload->GetRtpMap().GetPayloadNumber() ||
                    FindMatchedEvsFmtp(pLocalPayload, pPeerPayload))
            {
                pLocalPayload->GetRtpMap().SetPayloadNumber(
                        pPeerPayload->GetRtpMap().GetPayloadNumber());
                pLocalProfile->SetNegotiatedPayloadIndex(i);
                IMS_TRACE_D(
                        "FindMatchingEvsPayload(): Found, Local Payload index[%d], local PT[%d]", i,
                        pLocalPayload->GetRtpMap().GetPayloadNumber(), 0);
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioProfileNegotiator::FindAmrInProfile(IN AudioProfile* pProfile,
        IN AudioProfile::Payload* pPayload, OUT IMS_UINT32* pnNegoModeSetList,
        OUT IMS_UINT32* pnNegoDefaultRtpModeSet)
{
    if (pProfile == IMS_NULL || pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "FindAmrInProfile(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bRetModeSetFound = FindMatchedAmrInProfile(
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
IMS_BOOL AudioProfileNegotiator::FindMatchedAmrInProfile(IN AudioProfile* pProfile,
        IN AudioProfile::Payload* pPayload, IN IMS_BOOL bReturnMode,
        OUT IMS_UINT32* pnNegoModeSetList, OUT IMS_UINT32* pnNegoDefaultRtpModeSet)
{
    IMS_UINT32 nTempNegoModeSetList = 0;
    IMS_UINT32 nTempDefaultNegoModeSetList = 0;
    IMS_BOOL bModeSetFound = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadListSize(); i++)
    {
        AudioProfile::Payload* pComparedPayload = pProfile->GetPayloadAt(i);

        if (pComparedPayload == IMS_NULL)
        {
            continue;
        }

        if (pComparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR") ||
                pComparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
        {
            auto pCompareFmtp =
                    std::static_pointer_cast<AudioProfile::AmrFmtp>(pComparedPayload->GetFmtp());
            auto pReceivedFmtp =
                    std::static_pointer_cast<AudioProfile::AmrFmtp>(pPayload->GetFmtp());

            if (pCompareFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
            {
                continue;
            }
            if (!pComparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                        pPayload->GetRtpMap().GetPayloadType()))
            {
                continue;
            }
            if (pComparedPayload->GetRtpMap().GetSamplingRate() !=
                    pPayload->GetRtpMap().GetSamplingRate())
            {
                continue;
            }

            if (pCompareFmtp->GetOctetAlign() != pReceivedFmtp->GetOctetAlign())
            {
                continue;
            }

            /* AMR Open Offer (MO): In cases where the mode-set received from the MT device does not
             * align with the offered capabilities, the mode-set determined through SDP negotiation
             * is nevertheless preserved. */
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
                if (!bModeSetFound && bReturnMode == RETURN_MODE_SIMILAR)
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
                        pComparedPayload->GetRtpMap().GetPayloadType().GetStr(),
                        pCompareFmtp->GetOctetAlign());
                IMS_TRACE_D("FindAmrInProfile() Local/Peer is exactly matched[0x%04x][0x%04x] \
                        =>[0x%04x]",
                        pCompareFmtp->GetModeSetList(), pReceivedFmtp->GetModeSetList(),
                        *pnNegoModeSetList);

                return IMS_TRUE;
            }
        }
    }

    // for AMR Open Offer
    if (bModeSetFound)
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
IMS_BOOL AudioProfileNegotiator::FindPcmInProfile(
        IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload)
{
    if (pProfile == IMS_NULL || pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "FindPcmInProfile(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadListSize(); i++)
    {
        AudioProfile::Payload* pComparedPayload = pProfile->GetPayloadAt(i);

        if (pComparedPayload == IMS_NULL)
        {
            continue;
        }

        if (pComparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMU") ||
                pComparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMA"))
        {
            if (!pComparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                        pPayload->GetRtpMap().GetPayloadType()))
            {
                continue;
            }
            if (pComparedPayload->GetRtpMap().GetSamplingRate() !=
                    pPayload->GetRtpMap().GetSamplingRate())
            {
                continue;
            }

            IMS_TRACE_D("FindPcmInProfile() Found G.711 at[%d], Codec[%s], nSamplingRate[%d]", i,
                    pComparedPayload->GetRtpMap().GetPayloadType().GetStr(),
                    pPayload->GetRtpMap().GetSamplingRate());

            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_SINT32 AudioProfileNegotiator::CompareModeSet(
        IN std::shared_ptr<AudioProfile::AmrFmtp> pLocalFmtp,
        IN std::shared_ptr<AudioProfile::AmrFmtp> pPeerFmtp, IN IMS_BOOL bReturnMode,
        OUT IMS_UINT32* nNegoModeSet, OUT IMS_UINT32* nNegoDefaultRtpModeSet)
{
    IMS_TRACE_I("CompareModeSet(): Src modeSet[0x%04x] Dest modeSet[0x%04x]",
            pLocalFmtp->GetModeSetList(), pPeerFmtp->GetModeSetList(), 0);
    IMS_TRACE_I("CompareModeSet(): Src defaultmodeSet[0x%04x] Dest defaultmodeSet[0x%04x]",
            pLocalFmtp->GetDefaultRtpModeSet(), pPeerFmtp->GetDefaultRtpModeSet(), 0);

    IMS_SINT32 nResult = 1;  // -1 : no matched, 0 : similar, 1 : exactly matched

    if (m_bIsOfferReceived)  // MT Case
    {
        if (pPeerFmtp->GetModeSetList() != 0)
        {
            *nNegoModeSet = pPeerFmtp->GetModeSetList();
        }
        else if (pLocalFmtp->GetModeSetList() != 0)
        {
            *nNegoModeSet = pLocalFmtp->GetModeSetList();
        }
        else
        {
            *nNegoModeSet = 0;  // MO, MT both has no mode-set
            *nNegoDefaultRtpModeSet = pLocalFmtp->GetDefaultRtpModeSet();
        }
    }
    else  // MO Case
    {
        if ((pLocalFmtp->GetModeSetList() == 0) && (pPeerFmtp->GetModeSetList() == 0))
        {
            *nNegoModeSet = 0;
            *nNegoDefaultRtpModeSet = pLocalFmtp->GetDefaultRtpModeSet();
        }
        else if ((pLocalFmtp->GetModeSetList() != 0) && (pPeerFmtp->GetModeSetList() != 0))
        {
            *nNegoModeSet = pLocalFmtp->GetModeSetList() & pPeerFmtp->GetModeSetList();

            if (pLocalFmtp->GetModeSetList() != pPeerFmtp->GetModeSetList())
            {
                nResult = 0;
            }

            if (*nNegoModeSet == 0)
            {
                IMS_TRACE_D(
                        "CompareModeSet(): ModeSet Not Matched - isFinal: %d", bReturnMode, 0, 0);
                if (bReturnMode == RETURN_MODE_SIMILAR)
                {
                    *nNegoModeSet = pPeerFmtp->GetModeSetList();
                    IMS_TRACE_D("CompareModeSet(): Copy Dest Mode-set", 0, 0, 0);
                }
                else
                {
                    IMS_TRACE_E(0, "CompareModeSet(): ModeSet Not Matched", 0, 0, 0);
                    return -1;
                }
            }
        }
        else  // one of two has no modeset
        {
            *nNegoModeSet = pLocalFmtp->GetModeSetList() | pPeerFmtp->GetModeSetList();
        }
    }

    IMS_TRACE_I("CompareModeSet(): Result[%d] Negotiated modeSet[0x%04x] "
                "nNegoDefaultRtpModeSet[0x%04x]",
            nResult, *nNegoModeSet, *nNegoDefaultRtpModeSet);

    return nResult;
}

PRIVATE
IMS_BOOL AudioProfileNegotiator::CompareEvsBw(IN std::shared_ptr<AudioProfile::EvsFmtp> pLocalFmtp,
        IN std::shared_ptr<AudioProfile::EvsFmtp> pPeerFmtp, OUT IMS_UINT32* nNegoBwList)
{
    IMS_UINT32 nVerifyPeerBwList = (pPeerFmtp->GetBwList() > 0)
            ? pPeerFmtp->GetBwList()
            : pPeerFmtp->GetBwRecv() & pPeerFmtp->GetBwSend();
    IMS_TRACE_D("CompareEvsBw(): peer Bw:[0x%04x]", nVerifyPeerBwList, 0, 0);

    if (!IsValidEvsBwList(nVerifyPeerBwList))
    {
        IMS_TRACE_D("CompareEvsBw(): Primary Mode - invalid received BW", 0, 0, 0);
        return IMS_FALSE;
    }

    if ((pLocalFmtp->GetBwList() == 0) && (pPeerFmtp->GetBwList() == 0))
    {
        *nNegoBwList = 0;
    }
    else if ((pLocalFmtp->GetBwList() != 0) && (pPeerFmtp->GetBwList() != 0))
    {
        if (m_bIsOfferReceived)
        {
            if (pPeerFmtp->GetBwList() == 0x04)
            {
                if (pPeerFmtp->GetBrList() == 0x10)
                {
                    IMS_TRACE_D("CompareEvsBw(): Primary Mode - Config B0 Type Nego", 0, 0, 0);
                    *nNegoBwList = pLocalFmtp->GetBwList() & pPeerFmtp->GetBwList();
                }
                else if (pLocalFmtp->GetBwList() != 0x04)
                {
                    IMS_TRACE_D("CompareEvsBw(): Primary Mode - Not Config B Type Nego", 0, 0, 0);
                    return IMS_FALSE;
                }
                else
                {
                    IMS_TRACE_D("CompareEvsBw(): Primary Mode - Config B Type Nego", 0, 0, 0);
                    *nNegoBwList = pPeerFmtp->GetBwList();
                }
            }
            else
            {
                IMS_TRACE_D("CompareEvsBw(): Primary Mode: B Type Nego", 0, 0, 0);
                *nNegoBwList = pLocalFmtp->GetBwList() & pPeerFmtp->GetBwList();
            }
        }
        else
        {
            if (pLocalFmtp->GetBwList() == 0x04 && pPeerFmtp->GetBwList() != 0x04)
            {
                IMS_TRACE_D("CompareEvsBw(): check next payload", 0, 0, 0);
                return IMS_FALSE;
            }
            *nNegoBwList = pLocalFmtp->GetBwList() & pPeerFmtp->GetBwList();
        }

        if (*nNegoBwList == 0)
        {
            IMS_TRACE_D("CompareEvsBw(): Primary Mode - Bandwidth Not Matched", 0, 0, 0);
            return IMS_FALSE;
        }
    }
    else if ((pLocalFmtp->GetBwList() != 0) &&
            ((pPeerFmtp->GetBwRecv() != 0) || (pPeerFmtp->GetBwSend() != 0)))
    {
        if (pPeerFmtp->GetBwRecv() == 0)
            pPeerFmtp->SetBwRecv(0x0f);
        if (pPeerFmtp->GetBwSend() == 0)
            pPeerFmtp->SetBwSend(0x0f);

        *nNegoBwList = pLocalFmtp->GetBwList() & (pPeerFmtp->GetBwRecv() & pPeerFmtp->GetBwSend());
        if (*nNegoBwList == 0)
        {
            IMS_TRACE_D("CompareEvsBw(): Primary Mode - Bandwidth Not Matched", 0, 0, 0);
            return IMS_FALSE;
        }
    }
    else
    {
        *nNegoBwList = pLocalFmtp->GetBwList() | pPeerFmtp->GetBwList();
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AudioProfileNegotiator::CompareEvsBr(IN std::shared_ptr<AudioProfile::EvsFmtp> pLocalFmtp,
        IN std::shared_ptr<AudioProfile::EvsFmtp> pPeerFmtp, IN IMS_UINT32 nNegoBwList,
        OUT IMS_UINT32* nNegoBrList)
{
    if ((pLocalFmtp->GetBrList() == 0) && (pPeerFmtp->GetBrList() == 0))
    {
        *nNegoBrList = 0;
    }
    else if ((pLocalFmtp->GetBrList() != 0) && (pPeerFmtp->GetBrList() != 0))
    {
        if (m_bIsOfferReceived)
        {
            *nNegoBrList = pLocalFmtp->GetBrList() & pPeerFmtp->GetBrList();
            if ((nNegoBwList == 0x04) && (*nNegoBrList == 0x10))
            {
                IMS_TRACE_D("CompareEvsBr(): Primary Mode - Config B0,B1 Type Nego", 0, 0, 0);
                *nNegoBrList = (pPeerFmtp->GetBrList()) & 0x1f;
            }
        }
        else
        {
            *nNegoBrList = pLocalFmtp->GetBrList() & pPeerFmtp->GetBrList();
        }

        if (*nNegoBrList == 0)
        {
            IMS_TRACE_D("CompareEvsBr(): Primary Mode - Bitrate Not Matched", 0, 0, 0);
            return IMS_FALSE;
        }
    }
    else if ((pLocalFmtp->GetBrList() != 0) &&
            (pPeerFmtp->GetBrRecv() != 0 || (pPeerFmtp->GetBrSend() != 0)))
    {
        if (pPeerFmtp->GetBrRecv() == 0)
            pPeerFmtp->SetBrRecv(0x0fff);
        if (pPeerFmtp->GetBrSend() == 0)
            pPeerFmtp->SetBrSend(0x0fff);

        *nNegoBrList = pLocalFmtp->GetBrList() & (pPeerFmtp->GetBrRecv() & pPeerFmtp->GetBrSend());
        if (*nNegoBrList == 0)
        {
            IMS_TRACE_D("CompareEvsBr(): Primary Mode - Bitrate Not Matched", 0, 0, 0);
            return IMS_FALSE;
        }
    }
    else
    {
        *nNegoBrList = pLocalFmtp->GetBrList() | pPeerFmtp->GetBrList();
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AudioProfileNegotiator::CompareEvsMode(
        IN std::shared_ptr<AudioProfile::EvsFmtp> pLocalFmtp,
        IN std::shared_ptr<AudioProfile::EvsFmtp> pPeerFmtp, OUT IMS_UINT32* nNegoModeList)
{
    if ((pLocalFmtp->GetModeSetList() == 0) && (pPeerFmtp->GetModeSetList() == 0))
    {
        *nNegoModeList = 0;
    }
    else if ((pLocalFmtp->GetModeSetList() != 0) && (pPeerFmtp->GetModeSetList() != 0))
    {
        *nNegoModeList = pLocalFmtp->GetModeSetList() & pPeerFmtp->GetModeSetList();

        if (*nNegoModeList == 0)
        {
            IMS_TRACE_D("CompareEvsMode(): AMR IO Mode - ModeSet Not Matched", 0, 0, 0);
            return IMS_FALSE;
        }
    }
    else
    {
        *nNegoModeList = pLocalFmtp->GetModeSetList() | pPeerFmtp->GetModeSetList();
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AudioProfileNegotiator::CompareEvsBwBrMode(
        IN std::shared_ptr<AudioProfile::EvsFmtp> pLocalFmtp,
        IN std::shared_ptr<AudioProfile::EvsFmtp> pPeerFmtp, OUT IMS_UINT32* nNegoBwList,
        OUT IMS_UINT32* nNegoBrList, OUT IMS_UINT32* nNegoModeList)
{
    IMS_TRACE_D("CompareEvsBwBrMode(): Src Bandwidth[0x%04x], Bitrate[0x%04x], modeSet[0x%04x]",
            pLocalFmtp->GetBwList(), pLocalFmtp->GetBrList(), pLocalFmtp->GetModeSetList());
    IMS_TRACE_D("CompareEvsBwBrMode(): Dest Bandwidth[0x%04x], Bitrate[0x%04x], modeSet[0x%04x]",
            pPeerFmtp->GetBwList(), pPeerFmtp->GetBrList(), pPeerFmtp->GetModeSetList());

    if (pPeerFmtp->GetEvsModeSwitch() == 1)  // AMR IO Mode
    {
        if (!CompareEvsMode(pLocalFmtp, pPeerFmtp, nNegoModeList))
        {
            return IMS_FALSE;
        }

        *nNegoBwList = pPeerFmtp->GetBwList();
        *nNegoBrList = pPeerFmtp->GetBrList();
    }
    else  // Primary Mode
    {
        if (!CompareEvsBw(pLocalFmtp, pPeerFmtp, nNegoBwList))
        {
            return IMS_FALSE;
        }

        if (!CompareEvsBr(pLocalFmtp, pPeerFmtp, *nNegoBwList, nNegoBrList))
        {
            return IMS_FALSE;
        }

        if (pPeerFmtp->GetModeSetList() != 0)
        {
            *nNegoModeList = pPeerFmtp->GetModeSetList();
        }
    }
    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AudioProfileNegotiator::CompareEvsBwBrModeLegacy(
        IN std::shared_ptr<AudioProfile::EvsFmtp> pLocalFmtp,
        IN std::shared_ptr<AudioProfile::EvsFmtp> pPeerFmtp, OUT IMS_UINT32* nNegoBwList,
        OUT IMS_UINT32* nNegoBrList, OUT IMS_UINT32* nNegoModeList)
{
    IMS_TRACE_D("CompareEvsBwBrModeLegacy(): Src BW[0x%04x], BR[0x%04x], modeSet[0x%04x]",
            pLocalFmtp->GetBwList(), pLocalFmtp->GetBrList(), pLocalFmtp->GetModeSetList());
    IMS_TRACE_D("CompareEvsBwBrModeLegacy(): Dest BW[0x%04x], BR[0x%04x], modeSet[0x%04x]",
            pPeerFmtp->GetBwList(), pPeerFmtp->GetBrList(), pPeerFmtp->GetModeSetList());

    if (pPeerFmtp->GetEvsModeSwitch() == 1)  // AMR IO Mode
    {
        // Set AMR ModeSet lis.
        if ((pLocalFmtp->GetModeSetList() == 0) && (pPeerFmtp->GetModeSetList() == 0))
        {
            *nNegoModeList = 0;
        }
        else if ((pLocalFmtp->GetModeSetList() != 0) && (pPeerFmtp->GetModeSetList() != 0))
        {
            *nNegoModeList = pLocalFmtp->GetModeSetList() & pPeerFmtp->GetModeSetList();

            if (*nNegoModeList == 0)
            {
                IMS_TRACE_D(
                        "CompareEvsBwBrModeLegacy(): AMR IO Mode - ModeSet Not Matched", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoModeList = pLocalFmtp->GetModeSetList() | pPeerFmtp->GetModeSetList();
        }

        // in case of EVS AMR IO Mode, dest bw/br list added
        {
            *nNegoBwList = pPeerFmtp->GetBwList();
            *nNegoBrList = pPeerFmtp->GetBrList();
        }
    }
    else  // Primary Mode
    {
        IMS_UINT32 nVerifyPeerBwList = (pPeerFmtp->GetBwList() > 0)
                ? pPeerFmtp->GetBwList()
                : pPeerFmtp->GetBwRecv() & pPeerFmtp->GetBwSend();
        IMS_TRACE_D("CompareEvsBwBrMode(): peer Bw:[0x%04x]", nVerifyPeerBwList, 0, 0);

        if (!IsValidEvsBwList(nVerifyPeerBwList))
        {
            IMS_TRACE_D("CompareEvsBwBrModeLegacy(): Primary Mode - invalid received BW", 0, 0, 0);
            return IMS_FALSE;
        }

        // Set Bandwidth/Bitrate list.
        if ((pLocalFmtp->GetBwList() == 0) && (pPeerFmtp->GetBwList() == 0))
        {
            *nNegoBwList = 0;
        }
        else if ((pLocalFmtp->GetBwList() != 0) && (pPeerFmtp->GetBwList() != 0))
        {
            *nNegoBwList = pLocalFmtp->GetBwList() & pPeerFmtp->GetBwList();

            if (*nNegoBwList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy(): Primary Mode - Bandwidth Not Matched", 0,
                        0, 0);
                return IMS_FALSE;
            }
        }
        else if ((pLocalFmtp->GetBwList() != 0) &&
                ((pPeerFmtp->GetBwRecv() != 0) || (pPeerFmtp->GetBwSend() != 0)))
        {
            if (pPeerFmtp->GetBwRecv() == 0)
            {
                pPeerFmtp->SetBwRecv(0x0f);
            }
            if (pPeerFmtp->GetBwSend() == 0)
            {
                pPeerFmtp->SetBwSend(0x0f);
            }

            IMS_UINT32 nPeerBwList = pPeerFmtp->GetBwRecv() & pPeerFmtp->GetBwSend();

            *nNegoBwList = pLocalFmtp->GetBwList() & nPeerBwList;
            if (*nNegoBwList == 0)
            {
                IMS_TRACE_D("CompareEvsBwBrModeLegacy(): Primary Mode - Bandwidth Not Matched", 0,
                        0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoBwList = pLocalFmtp->GetBwList() | pPeerFmtp->GetBwList();
        }

        if ((pLocalFmtp->GetBrList() == 0) && (pPeerFmtp->GetBrList() == 0))
        {
            *nNegoBrList = 0;
        }
        else if ((pLocalFmtp->GetBrList() != 0) && (pPeerFmtp->GetBrList() != 0))
        {
            *nNegoBrList = pLocalFmtp->GetBrList() & pPeerFmtp->GetBrList();

            if (*nNegoBrList == 0)
            {
                IMS_TRACE_D(
                        "CompareEvsBwBrModeLegacy(): Primary Mode - Bitrate Not Matched", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        else if ((pLocalFmtp->GetBrList() != 0) &&
                (pPeerFmtp->GetBrRecv() != 0 || (pPeerFmtp->GetBrSend() != 0)))
        {
            if (pPeerFmtp->GetBrRecv() == 0)
            {
                pPeerFmtp->SetBrRecv(0x0fff);
            }
            if (pPeerFmtp->GetBrSend() == 0)
            {
                pPeerFmtp->SetBrSend(0x0fff);
            }

            IMS_UINT32 nPeerBRList = pPeerFmtp->GetBrRecv() & pPeerFmtp->GetBrSend();

            *nNegoBrList = pLocalFmtp->GetBrList() & nPeerBRList;

            if (*nNegoBrList == 0)
            {
                IMS_TRACE_D(
                        "CompareEvsBwBrModeLegacy(): Primary Mode - Bitrate Not Matched", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        else
        {
            *nNegoBrList = pLocalFmtp->GetBrList() | pPeerFmtp->GetBrList();
        }

        // in case of EVS Primary Mode, dest modeset list added
        if (pPeerFmtp->GetModeSetList() != 0)
        {
            *nNegoModeList = pPeerFmtp->GetModeSetList();
        }
    }
    return IMS_TRUE;
}

PRIVATE IMS_SINT32 AudioProfileNegotiator::FindPayloadIndexFromProfile(
        IN const AString& strCodecName, IN AudioProfile* pLocalProfile,
        IN AudioProfile::Payload* pPeerPayload)
{
    if (pLocalProfile == IMS_NULL || pPeerPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "FindPayloadIndexFromProfile(): invalid arguments", 0, 0, 0);
        return -1;
    }

    IMS_SINT32 nRetIndex = -1;

    nRetIndex = FindMatchedPayloadIndexFromProfile(
            strCodecName, pLocalProfile, pPeerPayload, RETURN_MODE_MATCHED);

    IMS_TRACE_D("FindPayloadIndexFromProfile() the 1st nRetIndex: %d", nRetIndex, 0, 0);

    if (nRetIndex == -1)
    {
        nRetIndex = FindMatchedPayloadIndexFromProfile(
                strCodecName, pLocalProfile, pPeerPayload, RETURN_MODE_SIMILAR);
        IMS_TRACE_D("FindPayloadIndexFromProfile() the 2nd nRetIndex: %d", nRetIndex, 0, 0);
    }

    return nRetIndex;
}

PRIVATE IMS_SINT32 AudioProfileNegotiator::FindMatchedPayloadIndexFromProfile(
        IN const AString& strCodecName, IN AudioProfile* pLocalProfile,
        IN AudioProfile::Payload* pPeerPayload, IN IMS_BOOL bReturnMode)
{
    IMS_SINT32 nTempIndex = -1;

    for (IMS_UINT32 nNegoEntry = 0; nNegoEntry < EVS_NEGO_RETRY_COUNT; nNegoEntry++)
    {
        for (IMS_UINT32 i = 0; i < pLocalProfile->GetPayloadListSize(); i++)
        {
            AudioProfile::Payload* pComparedPayload = pLocalProfile->GetPayloadAt(i);

            if (pComparedPayload == IMS_NULL)
            {
                continue;
            }

            if (strCodecName.EqualsIgnoreCase("AMR") || strCodecName.EqualsIgnoreCase("AMR-WB"))
            {
                if (nNegoEntry >= 1)
                {
                    continue;
                }
                if (pComparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR") ||
                        pComparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"))
                {
                    IMS_UINT32 pnNegoModeSetList = 0;
                    IMS_UINT32 pnNegoDefaultRtpModeSet = 0;
                    auto pCompareFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(
                            pComparedPayload->GetFmtp());
                    auto pReceivedFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(
                            pPeerPayload->GetFmtp());

                    if (pCompareFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
                    {
                        continue;
                    }

                    if (pComparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                                pPeerPayload->GetRtpMap().GetPayloadType()) == IMS_FALSE)
                    {
                        continue;
                    }
                    if (pComparedPayload->GetRtpMap().GetSamplingRate() !=
                            pPeerPayload->GetRtpMap().GetSamplingRate())
                    {
                        continue;
                    }

                    if (pCompareFmtp->GetOctetAlign() != pReceivedFmtp->GetOctetAlign())
                    {
                        continue;
                    }

                    /* AMR Open Offer (MO): In cases where the mode-set received from the MT device
                     * does not align with the offered capabilities, the mode-set determined through
                     * SDP negotiation is nevertheless preserved. */
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
                                    i, pComparedPayload->GetRtpMap().GetPayloadType().GetStr(),
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
                                i, pComparedPayload->GetRtpMap().GetPayloadType().GetStr(),
                                pCompareFmtp->GetOctetAlign());

                        return i;
                    }
                }
            }
            else if (strCodecName.EqualsIgnoreCase("EVS"))
            {
                if (pComparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
                {
                    IMS_UINT32 nBandwidthNegoList;
                    IMS_UINT32 nBitrateNegoList;
                    IMS_UINT32 nModeSetNegoList;
                    auto pCompareFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(
                            pComparedPayload->GetFmtp());
                    auto pReceivedFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(
                            pPeerPayload->GetFmtp());

                    if (pCompareFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
                    {
                        continue;
                    }

                    if (pComparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                                pPeerPayload->GetRtpMap().GetPayloadType()) == IMS_FALSE)
                    {
                        continue;
                    }
                    // need to return remote BW and BR, ModeSet(AMR IO Mode.). also default values.
                    if (nNegoEntry == 0)
                    {
                        // IR92 rel15 EVS Br/Bw check.
                        if (!CompareEvsBwBrMode(pCompareFmtp, pReceivedFmtp, &nBandwidthNegoList,
                                    &nBitrateNegoList, &nModeSetNegoList))
                        {
                            continue;
                        }
                    }
                    else
                    {
                        // legacy EVS BR/BW check
                        if (!CompareEvsBwBrModeLegacy(pCompareFmtp, pReceivedFmtp,
                                    &nBandwidthNegoList, &nBitrateNegoList, &nModeSetNegoList))
                        {
                            continue;
                        }
                    }

                    IMS_TRACE_D("FindMatchedPayloadIndexFromProfile() Found EVS at[%d], "
                                "Codec[%s],EvsModeSwitch[%d]",
                            i, pComparedPayload->GetRtpMap().GetPayloadType().GetStr(),
                            pCompareFmtp->GetEvsModeSwitch());

                    return i;
                }
            }
            //[G711]
            else if (strCodecName.EqualsIgnoreCase("PCMU") || strCodecName.EqualsIgnoreCase("PCMA"))
            {
                if (nNegoEntry >= 1)
                {
                    continue;
                }
                if (pComparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                            pPeerPayload->GetRtpMap().GetPayloadType()))
                {
                    IMS_TRACE_D("FindMatchedPayloadIndexFromProfile() Found G.711[%s] Found at[%d]",
                            pComparedPayload->GetRtpMap().GetPayloadType().GetStr(), i, 0);

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
MEDIA_DIRECTION AudioProfileNegotiator::UpdateDirectionToMine(
        IN MEDIA_DIRECTION ePeerDir, IN MEDIA_DIRECTION eSrcDir)
{
    if (ePeerDir < MEDIA_DIRECTION_INACTIVE || ePeerDir > MEDIA_DIRECTION_SEND_RECEIVE)
    {
        return MEDIA_DIRECTION_INVALID;
    }

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

    if (!m_bIsOfferReceived)
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

PRIVATE
IMS_BOOL AudioProfileNegotiator::IsValidEvsBwList(IN IMS_UINT32 nBwList)
{
    static const std::array<IMS_UINT32, 7> nValidBwLists = {EVS_BW_NB, EVS_BW_WB, EVS_BW_SWB,
            EVS_BW_FB, (EVS_BW_NB | EVS_BW_WB), (EVS_BW_NB | EVS_BW_WB | EVS_BW_SWB),
            (EVS_BW_NB | EVS_BW_WB | EVS_BW_SWB | EVS_BW_FB)};

    return std::any_of(nValidBwLists.begin(), nValidBwLists.end(),
            [nBwList](IMS_UINT32 validBw)
            {
                return nBwList == validBw;
            });
}
