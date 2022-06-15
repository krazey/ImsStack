/*
 * Copyright (C) 2022 The Android Open Source Project
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
#ifndef INTERFACE_MEDIA_STATE_H_
#define INTERFACE_MEDIA_STATE_H_

#include "AString.h"

class SdpMediaParameter;

class IMediaState
{
public:
    virtual const AString& GetConnectionAddress() const = 0;
    virtual IMS_SINT32 GetMediaState() const = 0;
    virtual SdpMediaParameter* GetMediaParameter(IN IMS_SINT32 nMid) const = 0;
    virtual const AString& GetPeerConnectionAddress() const = 0;
    virtual SdpMediaParameter* GetPeerMediaParameter(IN IMS_SINT32 nMid) const = 0;
    virtual SdpMediaParameter* GetProposalMediaParameter(IN IMS_SINT32 nMid) = 0;

public:
    /// Types of main media state
    enum
    {
        MEDIA_STATE_INACTIVE = 1,
        MEDIA_STATE_INACTIVE_PROPOSAL,
        MEDIA_STATE_PENDING,
        MEDIA_STATE_ACTIVE,
        MEDIA_STATE_DELETED,
        MEDIA_STATE_PROPOSAL
    };
};

#endif
