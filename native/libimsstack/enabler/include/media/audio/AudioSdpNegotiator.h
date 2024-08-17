// Copyright 2024 Google LLC

#ifndef AUDIO_SDP_NEGOTIATOR_H_
#define AUDIO_SDP_NEGOTIATOR_H_

#include "SdpNegotiator.h"
#include "audio/AudioProfileUtil.h"

class MediaConfiguration;

class AudioSdpNegotiator : public SdpNegotiator
{
public:
    AudioSdpNegotiator();
    virtual ~AudioSdpNegotiator();

    IMS_BOOL Negotiate(IN AudioProfile* pLocalProfile, IN AudioProfile* pPeerProfile,
            IN IMS_BOOL bIsOfferReceived, OUT AudioProfile* pNegotiatedProfile,
            IN MediaConfiguration* pConfig);

private:
    void ResetNegotiatedProfile(
            IN const AudioProfile* pLocalProfile, OUT AudioProfile* AudioProfile);
    IMS_BOOL FindEvsInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            IN IMS_BOOL bIsOfferReceived, OUT IMS_UINT32* pBandwidthNegoList,
            OUT IMS_UINT32* pBitrateNegoList, OUT IMS_UINT32* pModeSetNegoList);
    IMS_BOOL FindAmrInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            IN IMS_BOOL bIsOfferReceived, OUT IMS_UINT32* pnNegoModeSetList,
            OUT IMS_UINT32* pnNegoDefaultRtpModeSet);
    IMS_BOOL FindMatchedAmrInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            IN IMS_BOOL bIsOfferReceived, IN IMS_BOOL bReturnMode,
            OUT IMS_UINT32* pnNegoModeSetList, OUT IMS_UINT32* pnNegoDefaultRtpModeSet);
    IMS_BOOL FindPcmInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload);
    IMS_SINT32 CompareModeSet(IN AudioProfile::AmrFmtp* pSrcFmtp,
            IN AudioProfile::AmrFmtp* pDestFmtp, IN IMS_BOOL bIsOfferReceived,
            IN IMS_BOOL bReturnMode, OUT IMS_UINT32* nNegoModeSet,
            OUT IMS_UINT32* nNegoDefaultRtpModeSet);
    IMS_BOOL CompareEvsBwBrMode(IN AudioProfile::EvsFmtp* pSrcFmtp,
            IN AudioProfile::EvsFmtp* pDestFmtp, IN IMS_BOOL bIsOfferReceived,
            OUT IMS_UINT32* nNegoBwList, OUT IMS_UINT32* nNegoBrList,
            OUT IMS_UINT32* nNegoModeList);
    IMS_BOOL CompareEvsBwBrModeLegacy(IN AudioProfile::EvsFmtp* pSrcFmtp,
            IN AudioProfile::EvsFmtp* pDestFmtp, OUT IMS_UINT32* nNegoBwList,
            OUT IMS_UINT32* nNegoBrList, OUT IMS_UINT32* nNegoModeList);
    IMS_SINT32 FindPayloadIndexFromProfile(IN const AString& strCodecName,
            IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            IN IMS_BOOL isOfferReceivedCase);
    IMS_SINT32 FindMatchedPayloadIndexFromProfile(IN const AString& strCodecName,
            IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            IN IMS_BOOL isOfferReceivedCase, IN IMS_BOOL bReturnMode);
    MEDIA_DIRECTION UpdateDirectionToMine(IN MEDIA_DIRECTION ePeerDirection,
            IN MEDIA_DIRECTION eLocalDirection, IN IMS_BOOL bIsMtCase);
};

#endif
