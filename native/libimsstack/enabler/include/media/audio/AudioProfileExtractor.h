// Copyright 2024 Google LLC

#ifndef AUDIO_PROFILE_EXTRACTOR_H_
#define AUDIO_PROFILE_EXTRACTOR_H_

#include "ProfileExtractor.h"
#include "audio/AudioProfileUtil.h"

class SdpAvCodec;

class AudioProfileExtractor : public ProfileExtractor
{
public:
    explicit AudioProfileExtractor();
    virtual ~AudioProfileExtractor();

    IMS_BOOL Extract(IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
            OUT AudioProfile* pProfile);

private:
    void ExtractPayloads(IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile);
    void ExtractRtpMap(IN const SdpAvCodec* pSdpCodec, OUT AudioProfile::Payload* pPayload,
            OUT IMS_SINT32& nPayloadTypeNumber, OUT AString& strCodecName);
    IMS_BOOL ExtractFmtp(IN const AString& strFmtp, OUT AudioProfile::Payload* pPayload,
            IN const AString& strCodecName);
    IMS_BOOL ExtractAudioBaseFmtp(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp);
    void ExtractAmrFmtp(IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AmrFmtp* pFmtp);
    void ExtractEvsFmtp(IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ExtractFmtpModeSet(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp);
    IMS_BOOL ExtractFmtpModeChangeCapability(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp);
    IMS_BOOL ExtractFmtpModeChangePeriod(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp);
    IMS_BOOL ExtractFmtpModeChangeNeighbor(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp);
    IMS_BOOL ExtractFmtpMaxRed(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp);
    IMS_BOOL ExtractFmtpPtime(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp);
    IMS_BOOL ExtractFmtpMaxPtime(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp);
    void ExtractFmtpOctetAlign(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AmrFmtp* pFmtp);
    IMS_BOOL ExtractFmtpDtx(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ExtractFmtpHfOnly(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ExtractFmtpEvsSwitchMode(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ExtractFmtpBr(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ExtractFmtpBw(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ExtractFmtpCmr(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ExtractFmtpChAwMode(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ExtractFmtpBrSend(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ExtractFmtpBrRecv(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ExtractFmtpBwSend(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ExtractFmtpBwRecv(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    void SetEvsBrVisible(OUT AudioProfile::EvsFmtp* pFmtp);
    void SetEvsBwVisible(OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ExtractTelephoneEventFmtp(
            IN const AString& strFmtp, OUT AudioProfile::Payload* pPayload);
    void ExtractPtime(IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile);
    void ExtractMaxPtime(IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile);
    void ExtractRtcpXr(IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile);
    void ExtractAnbr(IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile);
};

#endif
