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

#ifndef AUDIO_PROFILE_NEGOTIATOR_H_
#define AUDIO_PROFILE_NEGOTIATOR_H_

#include "MediaProfileNegotiator.h"
#include "audio/AudioProfile.h"

class AudioConfiguration;
class MediaConfiguration;

/**
 * This class is to generate a negotiated audio profile by negotiating a local audio profile and a
 * peer audio profile
 */
class AudioProfileNegotiator : public MediaProfileNegotiator
{
public:
    AudioProfileNegotiator();
    virtual ~AudioProfileNegotiator() override;

    /**
     * @brief Make the negotiated profile using the local and peer profiles
     *
     * @param pLocalProfile The local profile
     * @param pPeerProfile The peer profile
     * @param bIsOfferReceived The option to check that the case is in sdp offer received
     * @param pNegotiatedProfile The negotiated profile to update
     * @param pConfig The configuration set
     * @return IMS_BOOL Return IMS_TRUE when there is no error in negotiation vise versa when there
     * is invalid parameter and the negotiation is failed
     */
    virtual IMS_BOOL Negotiate(IN AudioProfile* pLocalProfile, IN AudioProfile* pPeerProfile,
            IN IMS_BOOL bIsOfferReceived, OUT AudioProfile* pNegotiatedProfile,
            IN MediaConfiguration* pConfig);

private:
    AudioProfile::Payload* NegotiatePayload(IN AudioProfile* pLocalProfile,
            IN AudioProfile* pPeerProfile, OUT AudioProfile* pNegotiatedProfile,
            IN IMS_BOOL bAmrPayloadFormatRelaxedMatching);
    AudioProfile::Payload* NegotiateAudioPayload(IN AudioProfile* pLocalProfile,
            IN AudioProfile* pPeerProfile, IN AudioProfile* pNegotiatedProfile,
            IN IMS_BOOL bAmrPayloadFormatRelaxedMatching);
    AudioProfile::Payload* NegotiateAmr(IN AudioProfile* pLocalProfile,
            IN AudioProfile* pPeerProfile, IN AudioProfile* pNegotiatedProfile,
            IN IMS_UINT32 nPayloadIndex, IN IMS_BOOL bAmrPayloadFormatRelaxedMatching,
            IN IMS_UINT32 nNegoModeSetList, IN IMS_UINT32 nNegoDefaultRtpModeSet);
    std::shared_ptr<AudioProfile::AmrFmtp> NegotiateAmrFmtp(IN AudioProfile* pLocalProfile,
            IN AudioProfile::Payload* pPeerPayload, IN IMS_BOOL bAmrPayloadFormatRelaxedMatching,
            IN IMS_UINT32 nNegoModeSetList, IN IMS_UINT32 nNegoDefaultRtpModeSet,
            OUT IMS_SINT32& nLocalPayloadIndex);
    IMS_EVS_CONFIG CompareEVSBwBrWithIR92(IN AudioProfile* pLocalProfile,
            IN AudioProfile::Payload* pPeerPayload, IN AudioProfile::Payload* pNextPeerPayload);
    AudioProfile::Payload* NegotiateEvs(IN AudioProfile::Payload* pLocalPayload,
            IN AudioProfile::Payload* pPeerPayload, IN AudioProfile* pNegotiatedProfile);
    std::shared_ptr<AudioProfile::EvsFmtp> NegotiateEvsFmtp(IN AudioProfile::Payload* pLocalPayload,
            IN AudioProfile::Payload* pPeerPayload, IN IMS_UINT32 nBandwidthNegoList,
            IN IMS_UINT32 nBitrateNegoList, IN IMS_UINT32 nModeSetNegoList);
    void NegotiateUniDirectionBrBw(OUT std::shared_ptr<AudioProfile::EvsFmtp> pEvsFmtp,
            IN IMS_UINT32 nBandwidthNegoList, IN IMS_UINT32 nBitrateNegoList);
    void NegotiateCmr(IN std::shared_ptr<AudioProfile::EvsFmtp> pLocalFmtp,
            OUT std::shared_ptr<AudioProfile::EvsFmtp> pEvsFmtp);
    void NegotiateModeSet(OUT std::shared_ptr<AudioProfile::EvsFmtp> pEvsFmtp);
    AudioProfile::Payload* NegotiatePcm(IN AudioProfile* pLocalProfile,
            IN AudioProfile* pPeerProfile, IN AudioProfile* pNegotiatedProfile,
            IN IMS_UINT32 nPayloadIndex);
    IMS_BOOL NegotiateTelephoneEventPayload(IN IMS_SINT32 nNegotiatedSamplingRate,
            IN AudioProfile* pPeerProfile, OUT AudioProfile* pNegotiatedProfile);
    void NegotiateTelephoneEvent8000Payload(IN IMS_SINT32 nNegotiatedSamplingRate,
            IN AudioProfile* pPeerProfile, OUT AudioProfile* pNegotiatedProfile);
    IMS_BOOL NegotiateDirection(IN AudioProfile* pLocalProfile, IN AudioProfile* pPeerProfile,
            OUT AudioProfile* pNegotiatedProfile);
    void NegotiateRtcpXr(IN AudioProfile* pLocalProfile, OUT AudioProfile* pNegotiatedProfile);
    void NegotiatePtime(OUT AudioProfile* pNegotiatedProfile, IN IMS_SINT32 nLocalPtime);
    void NegotiateMaxPtime(OUT AudioProfile* pNegotiatedProfile, IN IMS_SINT32 nLocalMaxPtime);
    void NegotiateAnbr(IN IMS_BOOL nSupportAnbrLocal, IN IMS_BOOL nSupportAnbrPeer,
            OUT AudioProfile* pNegotiatedProfile);
    void NegotiateBandwidth(IN AudioProfile* pLocalProfile, IN AudioProfile* pPeerProfile,
            OUT AudioProfile* pNegotiatedProfile, IN AudioProfile::Payload* pNegotiatedPayload,
            IN MediaConfiguration* pConfig);
    IMS_SINT32 NegotiateAs(IN AudioProfile::Payload* pNegotiatedPayload, IN IMS_BOOL bIpv6);
    IMS_BOOL MakeNegotiatedBandwidth(IN const AudioConfiguration* pConfig,
            IN AudioProfile* pLocalProfile, IN AudioProfile* pPeerProfile,
            IN IMS_SINT32 nAsValueOfNegotiatedCodec, OUT AudioProfile* pNegotiatedProfile);
    void NegotiateRtcpInterval(OUT AudioProfile* pNegotiatedProfile,
            IN IMS_SINT32 nRtcpIntervalOnHold, IN IMS_SINT32 nRtcpIntervalOnActive);
    IMS_BOOL FindMatchingEvsPayload(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload);
    IMS_BOOL FindMatchedEvsFmtp(
            IN AudioProfile::Payload* pComparedPayload, IN AudioProfile::Payload* pPeerPayload);
    IMS_BOOL FindAmrInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            IN IMS_BOOL bAmrPayloadFormatRelaxedMatching, OUT IMS_UINT32* pnNegoModeSetList,
            OUT IMS_UINT32* pnNegoDefaultRtpModeSet);
    IMS_BOOL FindMatchedAmrInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            IN IMS_BOOL bReturnMode, IN IMS_BOOL bAmrPayloadFormatRelaxedMatching,
            OUT IMS_UINT32* pnNegoModeSetList, OUT IMS_UINT32* pnNegoDefaultRtpModeSet);
    IMS_BOOL FindPcmInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload);
    IMS_SINT32 CompareModeSet(IN std::shared_ptr<AudioProfile::AmrFmtp> pLocalFmtp,
            IN std::shared_ptr<AudioProfile::AmrFmtp> pPeerFmtp, IN IMS_BOOL bReturnMode,
            OUT IMS_UINT32* nNegoModeSet, OUT IMS_UINT32* nNegoDefaultRtpModeSet);
    IMS_BOOL CompareEvsBw(IN std::shared_ptr<AudioProfile::EvsFmtp> pLocalFmtp,
            IN std::shared_ptr<AudioProfile::EvsFmtp> pPeerFmtp, OUT IMS_UINT32* nNegoBwList);
    IMS_BOOL CompareEvsBr(IN std::shared_ptr<AudioProfile::EvsFmtp> pLocalFmtp,
            IN std::shared_ptr<AudioProfile::EvsFmtp> pPeerFmtp, IN IMS_UINT32 nNegoBwList,
            OUT IMS_UINT32* nNegoBrList);
    IMS_BOOL CompareEvsMode(IN std::shared_ptr<AudioProfile::EvsFmtp> pLocalFmtp,
            IN std::shared_ptr<AudioProfile::EvsFmtp> pPeerFmtp, OUT IMS_UINT32* nNegoModeList);
    IMS_BOOL CompareEvsBwBrMode(IN std::shared_ptr<AudioProfile::EvsFmtp> pLocalFmtp,
            IN std::shared_ptr<AudioProfile::EvsFmtp> pPeerFmtp, OUT IMS_UINT32* nNegoBwList,
            OUT IMS_UINT32* nNegoBrList, OUT IMS_UINT32* nNegoModeList);
    IMS_BOOL CompareEvsBwBrModeLegacy(IN std::shared_ptr<AudioProfile::EvsFmtp> pLocalFmtp,
            IN std::shared_ptr<AudioProfile::EvsFmtp> pPeerFmtp, OUT IMS_UINT32* nNegoBwList,
            OUT IMS_UINT32* nNegoBrList, OUT IMS_UINT32* nNegoModeList);
    IMS_SINT32 FindPayloadIndexFromProfile(IN const AString& strCodecName,
            IN AudioProfile* pLocalProfile, IN AudioProfile::Payload* pPeerPayload,
            IN IMS_BOOL bAmrPayloadFormatRelaxedMatching);
    IMS_SINT32 FindMatchedPayloadIndexFromProfile(IN const AString& strCodecName,
            IN AudioProfile* pLocalProfile, IN AudioProfile::Payload* pPeerPayload,
            IN IMS_BOOL bReturnMode, IN IMS_BOOL bAmrPayloadFormatRelaxedMatching);
    MEDIA_DIRECTION UpdateDirectionToMine(
            IN MEDIA_DIRECTION ePeerDirection, IN MEDIA_DIRECTION eLocalDirection);
    IMS_BOOL IsValidEvsBwList(IN IMS_UINT32 nBwList);
};

#endif
