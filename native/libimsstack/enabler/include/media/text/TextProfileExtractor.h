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

#ifndef TEXT_PROFILE_EXTRACTOR_H_
#define TEXT_PROFILE_EXTRACTOR_H_

#include "ProfileExtractor.h"
#include "text/TextProfileUtil.h"

class TextProfileExtractor : public ProfileExtractor
{
public:
    explicit TextProfileExtractor();
    virtual ~TextProfileExtractor();

    IMS_BOOL Extract(IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
            OUT TextProfile* pProfile);

private:
    void ExtractPayloads(IN IMediaDescriptor* pDescriptor, OUT TextProfile* pProfile);
    void ExtractRtpMap(IN const SdpAvCodec* pSdpCodec, OUT TextProfile::Payload* pPayload,
            OUT AString& strCodecName);
    IMS_BOOL ExtractFmtp(IN const AString& strFmtp, OUT TextProfile::Payload* pPayload,
            IN const ImsList<SdpMediaFormat*>& lstMediaFormat);
    IMS_BOOL ExtractRedFmtp(IN const AString& strFmtp, OUT TextProfile::RedFmtp* pFmtp);
    IMS_BOOL ExtractRedSubPtExist(
            IN const IMS_SINT32 nRedPayload, IN const ImsList<SdpMediaFormat*>& lstMediaFormat);
};

#endif
