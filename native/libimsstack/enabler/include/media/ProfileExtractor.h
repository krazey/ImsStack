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

#ifndef PROFILE_EXTRACTOR_H_
#define PROFILE_EXTRACTOR_H_

#include "ISessionDescriptor.h"
#include "media/IMediaDescriptor.h"
#include "offeranswer/SdpAvCodec.h"

#include "MediaBaseProfile.h"

class ProfileExtractor
{
public:
    explicit ProfileExtractor(IN const MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_NOTUSED);
    virtual ~ProfileExtractor();

protected:
    void Extract(IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
            OUT MediaBaseProfile* pProfile);
    IMS_BOOL ExtractCapaNego(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);
    IMS_BOOL ExtractAcfg(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);
    IMS_BOOL ExtractTcap(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);
    IMS_BOOL ExtractAcap(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);
    IMS_BOOL ExtractPcfg(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);

private:
    MEDIA_CONTENT_TYPE m_eType;
};

#endif
