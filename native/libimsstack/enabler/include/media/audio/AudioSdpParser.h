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

#ifndef AUDIO_SDP_PARSER_H_
#define AUDIO_SDP_PARSER_H_

#include "MediaSdpParser.h"
#include "audio/AudioProfileUtil.h"

class SdpAvCodec;

/**
 * This class is to generate a peer audio profile by parsing SDP media attributes from the
 * MediaDescriptor and the SessionDescriptor
 */
class AudioSdpParser : public MediaSdpParser
{
public:
    explicit AudioSdpParser();
    virtual ~AudioSdpParser();

    IMS_BOOL Parse(IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
            OUT AudioProfile* pProfile);

private:
    void ParsePayloads(IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile);
    void ParsePayload(IN const SdpAvCodec* pSdpCodec, OUT AudioProfile* pProfile);
    void ParseRtpMap(IN const SdpAvCodec* pSdpCodec, OUT AudioProfile::Payload* pPayload,
            OUT IMS_SINT32& nPayloadTypeNumber, OUT AString& strCodecName);
    IMS_BOOL ParseFmtp(IN const SdpAvCodec* pSdpCodec, OUT AudioProfile::Payload* pPayload,
            IN const AString& strCodecName);
    IMS_BOOL ParseAudioBaseFmtp(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp);
    void ParseAmrFmtp(IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AmrFmtp* pFmtp);
    void ParseEvsFmtp(IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ParseModeSet(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp);
    IMS_BOOL ParseModeChangeCapability(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp);
    IMS_BOOL ParseModeChangePeriod(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp);
    IMS_BOOL ParseModeChangeNeighbor(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp);
    IMS_BOOL ParseMaxRed(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp);
    void ParseOctetAlign(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AmrFmtp* pFmtp);
    IMS_BOOL ParseDtx(IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ParseHfOnly(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ParseEvsSwitchMode(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ParseBr(IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ParseBw(IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ParseCmr(IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ParseChAwMode(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ParseBrSend(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ParseBrRecv(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ParseBwSend(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ParseBwRecv(
            IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp);
    void SetEvsBrVisible(OUT AudioProfile::EvsFmtp* pFmtp);
    void SetEvsBwVisible(OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL ParseTelephoneEventFmtp(
            IN const SdpAvCodec* pSdpCodec, OUT AudioProfile::Payload* pPayload);
    void ParseEvents(IN const AString& strFmtp, OUT AudioProfile::TelephoneEventFmtp* pFmtp);
    void ParsePtime(IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile);
    void ParseMaxPtime(IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile);
    void ParseRtcpXr(IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile);
    void ParseAnbr(IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile);
};

#endif
