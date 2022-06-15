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

#include "media/FramedMediaImpl.h"
#include "media/FramedMediaProposalImpl.h"
#include "media/MediaDescriptor.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
FramedMediaImpl::FramedMediaImpl(IN FramedMedia* pFramedMedia) :
        m_pFramedMedia(pFramedMedia),
        m_pFramedMediaProposal(IMS_NULL)
{
    m_pFramedMedia->SetMediaListener(this);
}

PUBLIC VIRTUAL FramedMediaImpl::~FramedMediaImpl()
{
    if (m_pFramedMediaProposal != IMS_NULL)
    {
        delete m_pFramedMediaProposal;
        m_pFramedMediaProposal = IMS_NULL;
    }
}

PRIVATE VIRTUAL IMS_BOOL FramedMediaImpl::Equals(IN const IMedia* piMedia) const
{
    const FramedMediaImpl* pMediaImpl = DYNAMIC_CAST(const FramedMediaImpl*, piMedia);

    if (pMediaImpl == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return (this == pMediaImpl);
}

PRIVATE VIRTUAL IMSList<IMediaDescriptor*> FramedMediaImpl::GetMediaDescriptors() const
{
    const IMSList<MediaDescriptor*>& objMediaDescriptors = m_pFramedMedia->GetMediaDescriptors();

    if (objMediaDescriptors.IsEmpty())
    {
        IMS_TRACE_E(0, "No media descriptors in the current media", 0, 0, 0);
        return IMSList<IMediaDescriptor*>();
    }

    IMSList<IMediaDescriptor*> objIMediaDescriptors;

    for (IMS_UINT32 i = 0; i < objMediaDescriptors.GetSize(); ++i)
    {
        objIMediaDescriptors.Append(objMediaDescriptors.GetAt(i));
    }

    return objIMediaDescriptors;
}

PRIVATE VIRTUAL IMedia* FramedMediaImpl::GetProposal() const
{
    if ((GetState() != STATE_ACTIVE) || (GetUpdateState() != UPDATE_MODIFIED))
    {
        if ((GetState() == STATE_ACTIVE) && (GetUpdateState() == UPDATE_REMOVED))
        {
            // no-op
        }
        else
        {
            return IMS_NULL;
        }
    }

    return m_pFramedMediaProposal;
}

PRIVATE VIRTUAL void FramedMediaImpl::OnMedia_FictitiousMediaCreated(IN Media* pMedia)
{
    if (m_pFramedMedia != pMedia)
    {
        IMS_TRACE_E(0, "MEDIA MISMATCHED", 0, 0, 0);
        return;
    }

    FramedMediaProposal* pMediaProposal =
            DYNAMIC_CAST(FramedMediaProposal*, m_pFramedMedia->GetProposal());

    if (pMediaProposal == IMS_NULL)
    {
        // Do nothing
        IMS_TRACE_E(0, "NO MEDIA PROPOSAL", 0, 0, 0);
        return;
    }

    if (m_pFramedMediaProposal != IMS_NULL)
    {
        delete m_pFramedMediaProposal;
        m_pFramedMediaProposal = IMS_NULL;
    }

    m_pFramedMediaProposal = new FramedMediaProposalImpl(pMediaProposal);

    if (m_pFramedMediaProposal == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO MEMORY", 0, 0, 0);
        return;
    }
}

PRIVATE VIRTUAL void FramedMediaImpl::OnMedia_FictitiousMediaDestroyed(IN Media* pMedia)
{
    if (m_pFramedMedia != pMedia)
    {
        IMS_TRACE_E(0, "MEDIA MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_pFramedMediaProposal != IMS_NULL)
    {
        delete m_pFramedMediaProposal;
        m_pFramedMediaProposal = IMS_NULL;
    }
}
