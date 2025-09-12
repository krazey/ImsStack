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
#ifndef STREAM_MEDIA_PROPOSAL_IMPL_H_
#define STREAM_MEDIA_PROPOSAL_IMPL_H_

#include "media/IMedia.h"
#include "media/StreamMediaProposal.h"

class StreamMediaProposalImpl : public IMedia
{
public:
    inline explicit StreamMediaProposalImpl(IN StreamMediaProposal* pMediaProposal) :
            m_pMediaProposal(pMediaProposal)
    {
    }
    ~StreamMediaProposalImpl() override = default;

    StreamMediaProposalImpl(IN const StreamMediaProposalImpl&) = delete;
    StreamMediaProposalImpl& operator=(IN const StreamMediaProposalImpl&) = delete;

private:
    // IMedia interface
    inline IMS_SINT32 GetDirection() const override { return m_pMediaProposal->GetDirection(); }
    ImsList<IMediaDescriptor*> GetMediaDescriptors() const override;
    IMedia* GetProposal() const override;
    inline IMS_SINT32 GetState() const override { return STATE_PROPOSAL; }
    IMS_SINT32 GetUpdateState() const override;
    IMS_RESULT SetDirection(IN IMS_SINT32 nDirection) override;
    inline IMediaDescriptor* GetMediaDescriptor() const override
    {
        return m_pMediaProposal->GetMediaDescriptor();
    }
    inline IMS_SINT32 GetType() const override { return m_pMediaProposal->GetType(); }
    inline void RemoveMediaDescriptor(IN IMS_UINT32 /*nPosition*/) override {}

private:
    StreamMediaProposal* m_pMediaProposal;
};

#endif
