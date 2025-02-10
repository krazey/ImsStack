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

#ifndef MEDIA_SDP_PARSER_H_
#define MEDIA_SDP_PARSER_H_

#include "ISessionDescriptor.h"
#include "media/IMediaDescriptor.h"
#include "offeranswer/SdpAvCodec.h"

#include "MediaBaseProfile.h"

/**
 * This class is to generate a peer profile by parsing SDP media attributes from the MediaDescriptor
 * and the SessionDescriptor
 */
class MediaSdpParser
{
public:
    explicit MediaSdpParser(IN const MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_NOTUSED);
    virtual ~MediaSdpParser();

protected:
    /**
     * @brief Parse the SDP media attributes from the MediaDescriptor and the SessionDescriptor
     *
     * @param pSessionDescriptor The SDP descriptor instance to parse the session level SDP
     * @param pDescriptor The SDP descriptor instance to parse the media level SDP
     * @param pProfile The Profile to be set attribute by parsing media/session level SDP
     */
    void Parse(IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
            OUT MediaBaseProfile* pProfile);
    IMS_BOOL ParseCapaNego(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);
    IMS_BOOL ParseAcfg(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);
    IMS_BOOL ParseTcap(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);
    IMS_BOOL ParseAcap(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);
    IMS_BOOL ParsePcfg(
            IN IMediaDescriptor* pDescriptor, OUT MediaBaseProfile::CapaNego* pObjCapaNego);

private:
    MEDIA_CONTENT_TYPE m_eType;
};

#endif
