/**
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
#include "audio/AudioProfileUtil.h"

class MediaConfiguration;

/**
 * This class is to generate a negotiated audio profile by negotiating a local audio profile and a
 * peer audio profile
 */
class AudioProfileNegotiator : public MediaProfileNegotiator
{
public:
    AudioProfileNegotiator();
    virtual ~AudioProfileNegotiator();

    IMS_BOOL Negotiate(IN AudioProfile* pLocalProfile, IN AudioProfile* pPeerProfile,
            IN IMS_BOOL bIsOfferReceived, OUT AudioProfile* pNegotiatedProfile,
            IN MediaConfiguration* pConfig);

private:
    void ResetNegotiatedProfile(
            IN const AudioProfile* pLocalProfile, OUT AudioProfile** AudioProfile);
    AudioProfile::Payload* NegotiatePayload(IN AudioProfile* pLocalProfile,
            IN AudioProfile* pPeerProfile, OUT AudioProfile* pNegotiatedProfile);
    AudioProfile::Payload* NegotiateAudioPayload(IN AudioProfile* pLocalProfile,
            IN AudioProfile* pPeerProfile, IN AudioProfile* pNegotiatedProfile);
    AudioProfile::Payload* NegotiateAmr(IN AudioProfile* pLocalProfile,
            IN AudioProfile* pPeerProfile, IN AudioProfile* pNegotiatedProfile,
            IN IMS_UINT32 nPayloadIndex, IN IMS_UINT32 nNegoModeSetList,
            IN IMS_UINT32 nNegoDefaultRtpModeSet);
    AudioProfile::AmrFmtp* NegotiateAmrFmtp(IN AudioProfile* pLocalProfile,
            IN AudioProfile::Payload* pDestPayload, IN IMS_UINT32 nNegoModeSetList,
            IN IMS_UINT32 nNegoDefaultRtpModeSet, OUT IMS_SINT32& nSrcPayloadIndex);
    AudioProfile::Payload* NegotiateEvs(IN AudioProfile* pLocalProfile,
            IN AudioProfile* pPeerProfile, IN AudioProfile* pNegotiatedProfile,
            IN IMS_UINT32 nPayloadIndex, IN IMS_UINT32 nBandwidthNegoList,
            IN IMS_UINT32 nBitrateNegoList, IN IMS_UINT32 nModeSetNegoList);
    AudioProfile::EvsFmtp* NegotiateEvsFmtp(IN AudioProfile* pLocalProfile,
            IN AudioProfile::Payload* pDestPayload, IN IMS_UINT32 nBandwidthNegoList,
            IN IMS_UINT32 nBitrateNegoList, IN IMS_UINT32 nModeSetNegoList,
            OUT IMS_SINT32& nSrcPayloadIndex);
    void NegotiateUniDirectionBrBw(OUT AudioProfile::EvsFmtp* pEvsFmtp,
            IN IMS_UINT32 nBandwidthNegoList, IN IMS_UINT32 nBitrateNegoList);
    void NegotiateCmr(IN AudioProfile::EvsFmtp* pSrcFmtp, OUT AudioProfile::EvsFmtp* pEvsFmtp);
    void NegotiateModeSet(OUT AudioProfile::EvsFmtp* pEvsFmtp);
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
    IMS_BOOL MakeNegotiatedBandwidth(IN AudioConfiguration* pConfig, IN AudioProfile* pLocalProfile,
            IN AudioProfile* pPeerProfile, IN IMS_SINT32 nAsValueOfNegoticatedCodec,
            OUT AudioProfile* pNegotiatedProfile);
    void NegotiateRtcpInterval(OUT AudioProfile* pNegotiatedProfile,
            IN IMS_SINT32 nRtcpIntervalOnHold, IN IMS_SINT32 nRtcpIntervalOnActive);
    IMS_BOOL FindEvsInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            OUT IMS_UINT32* pBandwidthNegoList, OUT IMS_UINT32* pBitrateNegoList,
            OUT IMS_UINT32* pModeSetNegoList);
    IMS_BOOL FindAmrInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            OUT IMS_UINT32* pnNegoModeSetList, OUT IMS_UINT32* pnNegoDefaultRtpModeSet);
    IMS_BOOL FindMatchedAmrInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            IN IMS_BOOL bReturnMode, OUT IMS_UINT32* pnNegoModeSetList,
            OUT IMS_UINT32* pnNegoDefaultRtpModeSet);
    IMS_BOOL FindPcmInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload);
    IMS_SINT32 CompareModeSet(IN AudioProfile::AmrFmtp* pSrcFmtp,
            IN AudioProfile::AmrFmtp* pDestFmtp, IN IMS_BOOL bReturnMode,
            OUT IMS_UINT32* nNegoModeSet, OUT IMS_UINT32* nNegoDefaultRtpModeSet);
    IMS_BOOL CompareEvsBwBrMode(IN AudioProfile::EvsFmtp* pSrcFmtp,
            IN AudioProfile::EvsFmtp* pDestFmtp, OUT IMS_UINT32* nNegoBwList,
            OUT IMS_UINT32* nNegoBrList, OUT IMS_UINT32* nNegoModeList);
    IMS_BOOL CompareEvsBwBrModeLegacy(IN AudioProfile::EvsFmtp* pSrcFmtp,
            IN AudioProfile::EvsFmtp* pDestFmtp, OUT IMS_UINT32* nNegoBwList,
            OUT IMS_UINT32* nNegoBrList, OUT IMS_UINT32* nNegoModeList);
    IMS_SINT32 FindPayloadIndexFromProfile(IN const AString& strCodecName,
            IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload);
    IMS_SINT32 FindMatchedPayloadIndexFromProfile(IN const AString& strCodecName,
            IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload, IN IMS_BOOL bReturnMode);
    MEDIA_DIRECTION UpdateDirectionToMine(
            IN MEDIA_DIRECTION ePeerDirection, IN MEDIA_DIRECTION eLocalDirection);
};

#endif
