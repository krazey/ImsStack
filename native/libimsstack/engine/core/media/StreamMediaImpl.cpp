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
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "base/Ims.h"
#include "media/MediaDescriptor.h"
#include "media/StreamMediaImpl.h"
#include "media/StreamMediaProposalImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
StreamMediaImpl::StreamMediaImpl(IN StreamMedia* pStreamMedia) :
        m_pStreamMedia(pStreamMedia),
        m_pStreamMediaProposal(IMS_NULL)
{
    m_pStreamMedia->SetMediaListener(this);
}

PUBLIC VIRTUAL StreamMediaImpl::~StreamMediaImpl()
{
    if (m_pStreamMediaProposal != IMS_NULL)
    {
        delete m_pStreamMediaProposal;
        m_pStreamMediaProposal = IMS_NULL;
    }
}

PRIVATE VIRTUAL IMS_BOOL StreamMediaImpl::Equals(IN const IMedia* piMedia) const
{
    const StreamMediaImpl* pMediaImpl = DYNAMIC_CAST(const StreamMediaImpl*, piMedia);

    if (pMediaImpl == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return (this == pMediaImpl);
}

PRIVATE VIRTUAL ImsList<IMediaDescriptor*> StreamMediaImpl::GetMediaDescriptors() const
{
    const ImsList<MediaDescriptor*>& objMediaDescriptors = m_pStreamMedia->GetMediaDescriptors();

    if (objMediaDescriptors.IsEmpty())
    {
        IMS_TRACE_E(0, "No media descriptors in the current media", 0, 0, 0);
        return ImsList<IMediaDescriptor*>();
    }

    ImsList<IMediaDescriptor*> objIMediaDescriptors;

    for (IMS_UINT32 i = 0; i < objMediaDescriptors.GetSize(); ++i)
    {
        objIMediaDescriptors.Append(objMediaDescriptors.GetAt(i));
    }

    return objIMediaDescriptors;
}

PRIVATE VIRTUAL IMedia* StreamMediaImpl::GetProposal() const
{
    if ((GetState() != STATE_ACTIVE) || (GetUpdateState() != UPDATE_MODIFIED))
    {
        if ((GetState() == STATE_ACTIVE) && (GetUpdateState() == UPDATE_REMOVED))
        {
            // no-op
        }
        else
        {
            Ims::SetLastError(ImsError::ILLEGAL_STATE);
            return IMS_NULL;
        }
    }

    return m_pStreamMediaProposal;
}

PRIVATE VIRTUAL void StreamMediaImpl::OnMedia_FictitiousMediaCreated(IN Media* pMedia)
{
    if (m_pStreamMedia != pMedia)
    {
        IMS_TRACE_E(0, "MEDIA MISMATCHED", 0, 0, 0);
        return;
    }

    StreamMediaProposal* pMediaProposal =
            DYNAMIC_CAST(StreamMediaProposal*, m_pStreamMedia->GetProposal());

    if (pMediaProposal == IMS_NULL)
    {
        // Do nothing
        IMS_TRACE_E(0, "NO MEDIA PROPOSAL", 0, 0, 0);
        return;
    }

    if (m_pStreamMediaProposal != IMS_NULL)
    {
        delete m_pStreamMediaProposal;
    }

    m_pStreamMediaProposal = new StreamMediaProposalImpl(pMediaProposal);

    if (m_pStreamMediaProposal == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO MEMORY", 0, 0, 0);
        return;
    }
}

PRIVATE VIRTUAL void StreamMediaImpl::OnMedia_FictitiousMediaDestroyed(IN Media* pMedia)
{
    if (m_pStreamMedia != pMedia)
    {
        IMS_TRACE_E(0, "MEDIA MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_pStreamMediaProposal != IMS_NULL)
    {
        delete m_pStreamMediaProposal;
        m_pStreamMediaProposal = IMS_NULL;
    }
}
