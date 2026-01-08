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
#ifndef MEDIA_PROPOSAL_H_
#define MEDIA_PROPOSAL_H_

#include "ImsCore.h"
#include "media/IMediaState.h"
#include "media/MediaDescriptor.h"

class ISdpOaState;

class MediaProposal : public IMediaState
{
public:
    MediaProposal(IN ISdpOaState* piOaState);
    ~MediaProposal() override;

    MediaProposal(IN const MediaProposal&) = delete;
    MediaProposal& operator=(IN const MediaProposal&) = delete;

public:
    // IMedia interface
    inline virtual const ImsList<MediaDescriptor*>& GetMediaDescriptors() const
    {
        return m_objDescriptors;
    }
    virtual IMS_SINT32 GetType() const = 0;

    IMS_BOOL CreateDescriptor(IN const ImsList<MediaDescriptor*>& objDescriptors);
    IMS_SINT32 GetDirection() const;
    MediaDescriptor* GetMediaDescriptor() const;
    MediaDescriptor* GetMediaDescriptor(IN IMS_SINT32 nMid) const;

protected:
    // IMediaState interface
    const AString& GetConnectionAddress() const override;
    inline IMS_SINT32 GetMediaState() const override { return MEDIA_STATE_PROPOSAL; }
    SdpMediaParameter* GetMediaParameter(IN IMS_SINT32 nMid) const override;
    const AString& GetPeerConnectionAddress() const override;
    SdpMediaParameter* GetPeerMediaParameter(IN IMS_SINT32 nMid) const override;
    SdpMediaParameter* GetProposalMediaParameter(IN IMS_SINT32 nMid) override;

private:
    ISdpOaState* m_piOaState;
    ImsList<MediaDescriptor*> m_objDescriptors;
};

#endif
