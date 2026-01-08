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

#ifndef TEXT_SDP_GENERATOR_H_
#define TEXT_SDP_GENERATOR_H_

#include "MediaSdpGenerator.h"
#include "TextProfile.h"

/**
 * This class is to generate a text Sdp by adding text attributes from text profile to the
 * MediaDescriptor and the SessionDescriptor
 */
class TextSdpGenerator : public MediaSdpGenerator
{
public:
    TextSdpGenerator();
    virtual ~TextSdpGenerator() override;

    virtual IMS_BOOL Generate(OUT ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pBaseProfile) override;

private:
    void CheckRedPayloadSubTypeValidity(OUT TextProfile* pProfile);
    void GeneratePayload(OUT IMediaDescriptor* pDescriptor, IN TextProfile* pProfile);
    IMS_BOOL GenerateFmtp(OUT AString& strFmtp, IN TextProfile::Payload* pPayload);
};

#endif
