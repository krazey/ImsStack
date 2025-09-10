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

#ifndef TEXT_SDP_PARSER_H_
#define TEXT_SDP_PARSER_H_

#include "MediaSdpParser.h"
#include "TextProfile.h"
#include "offeranswer/SdpAvCodec.h"

/**
 * This class is to generate a peer text profile by parsing SDP media attributes from the
 * MediaDescriptor and the SessionDescriptor
 */
class TextSdpParser : public MediaSdpParser
{
public:
    explicit TextSdpParser();
    virtual ~TextSdpParser() override;

    /**
     * @brief It is the core function responsible for extracting text-related attributes from
     * the SDP (Session Description Protocol) message. It takes the SDP data and populates a
     * TextProfile object with the parsed parameters
     *
     * @param pSessionDescriptor This pointer provides access to the overall SDP session description
     * @param pDescriptor This pointer contains the SDP media description specifically for the text
     * stream
     * @param pProfile This is an output parameter. The method will populate this object with the
     * parsed text parameters.
     * @return IMS_BOOL Returns IMS_FALSE when error handling to gracefully manage situations where
     * the SDP message is malformed or missing crucial text parameters.
     */
    virtual IMS_BOOL Parse(IN ISessionDescriptor* pSessionDescriptor,
            IN IMediaDescriptor* pDescriptor, OUT TextProfile* pProfile);

private:
    void ParsePayloads(IN const IMediaDescriptor* pDescriptor, OUT TextProfile* pProfile);
    void ParseRtpMap(IN const SdpAvCodec* pSdpCodec, OUT TextProfile::Payload* pPayload,
            OUT AString& strCodecName);
    IMS_BOOL ParseFmtp(IN const SdpAvCodec* pSdpCodec, OUT TextProfile::Payload* pPayload,
            IN const ImsList<SdpMediaFormat*>& lstMediaFormat);
    IMS_BOOL ParseT140Fmtp(IN const SdpAvCodec* pSdpCodec, OUT TextProfile::Payload* pPayload);
    IMS_BOOL ParseRedFmtp(
            IN const AString& strFmtp, OUT std::shared_ptr<TextProfile::RedFmtp> pFmtp);
    IMS_BOOL ParseRedSubPtExist(
            IN const IMS_SINT32 nRedPayload, IN const ImsList<SdpMediaFormat*>& lstMediaFormat);
};

#endif
