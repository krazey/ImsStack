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

#ifndef TEXT_SDP_PARSER_H_
#define TEXT_SDP_PARSER_H_

#include "MediaSdpParser.h"
#include "text/TextProfileUtil.h"

/**
 * This class is to generate a peer text profile by parsing SDP media attributes from the
 * MediaDescriptor and the SessionDescriptor
 */
class TextSdpParser : public MediaSdpParser
{
public:
    explicit TextSdpParser();
    virtual ~TextSdpParser();

    IMS_BOOL Parse(IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
            OUT TextProfile* pProfile);

private:
    void ParsePayloads(IN IMediaDescriptor* pDescriptor, OUT TextProfile* pProfile);
    void ParseRtpMap(IN const SdpAvCodec* pSdpCodec, OUT TextProfile::Payload* pPayload,
            OUT AString& strCodecName);
    IMS_BOOL ParseFmtp(IN const SdpAvCodec* pSdpCodec, OUT TextProfile::Payload* pPayload,
            IN const ImsList<SdpMediaFormat*>& lstMediaFormat);
    IMS_BOOL ParseRedFmtp(IN const AString& strFmtp, OUT TextProfile::RedFmtp* pFmtp);
    IMS_BOOL ParseRedSubPtExist(
            IN const IMS_SINT32 nRedPayload, IN const ImsList<SdpMediaFormat*>& lstMediaFormat);
};

#endif
