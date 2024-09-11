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

#ifndef SDP_GENERATOR_H_
#define SDP_GENERATOR_H_

#include "ISessionDescriptor.h"
#include "media/IMediaDescriptor.h"

#include "MediaBaseProfile.h"

class SdpGenerator
{
public:
    explicit SdpGenerator(IN const MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_NOTUSED);
    virtual ~SdpGenerator();

    virtual IMS_BOOL Generate(OUT ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pBaseProfile) = 0;

protected:
    void GenerateCommonAttributes(OUT ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pProfile);
    void GenerateRtpMap(OUT AString& strRtpmap, OUT AString& strPayloadNum,
            IN MediaBaseProfile::RtpMap& objRtpMap);
    IMS_SINT32 GenerateDirection(OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pProfile);
    void GenerateSessionLevelDirection(
            OUT ISessionDescriptor* pSessionDescriptor, IN IMS_SINT32 nDirection);

private:
    void ClearAttributeAndBandwidth(OUT IMediaDescriptor* pDescriptor);
    void SetSdpSessionIpAddress(
            OUT ISessionDescriptor* pSessionDescriptor, IN MediaBaseProfile* pProfile);
    void SetSdpMediaDescription(OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pProfile);
    void SetSdpMediaBandwidth(OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pProfile);

protected:
    MEDIA_CONTENT_TYPE m_eType;
};

#endif
