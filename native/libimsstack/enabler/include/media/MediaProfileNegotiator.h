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

#ifndef MEDIA_PROFILE_NEGOTIATOR_H_
#define MEDIA_PROFILE_NEGOTIATOR_H_

#include "ISessionDescriptor.h"
#include "media/IMediaDescriptor.h"

#include "MediaBaseProfile.h"

/**
 * This class is to generate a negotiated profile by negotiating a local profile and a peer profile
 */
class MediaProfileNegotiator
{
public:
    explicit MediaProfileNegotiator(IN const MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_NOTUSED);
    virtual ~MediaProfileNegotiator();

protected:
    IMS_BOOL NegotiateIpPort(IN MediaBaseProfile* pLocalProfile, IN MediaBaseProfile* pPeerProfile,
            OUT MediaBaseProfile* pNegotiatedProfile);

    MEDIA_CONTENT_TYPE m_eType;
    IMS_BOOL m_bIsOfferReceived;
};

#endif
