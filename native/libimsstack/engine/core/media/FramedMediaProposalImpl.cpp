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
#include "media/FramedMediaProposalImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;

PRIVATE VIRTUAL ImsList<IMediaDescriptor*> FramedMediaProposalImpl::GetMediaDescriptors() const
{
    const ImsList<MediaDescriptor*>& objMediaDescriptors = m_pMediaProposal->GetMediaDescriptors();

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

PRIVATE VIRTUAL IMedia* FramedMediaProposalImpl::GetProposal() const
{
    IMS_TRACE_E(0, "Operation not allowed in media proposal", 0, 0, 0);

    Ims::SetLastError(ImsError::ILLEGAL_STATE);

    return IMS_NULL;
}

PRIVATE VIRTUAL IMS_SINT32 FramedMediaProposalImpl::GetUpdateState() const
{
    IMS_TRACE_E(0, "Operation not allowed in media proposal", 0, 0, 0);

    Ims::SetLastError(ImsError::ILLEGAL_STATE);

    return UPDATE_INVALID;
}

PRIVATE VIRTUAL IMS_RESULT FramedMediaProposalImpl::SetDirection(IN IMS_SINT32 nDirection)
{
    (void)nDirection;

    IMS_TRACE_E(0, "Operation not allowed in media proposal", 0, 0, 0);

    Ims::SetLastError(ImsError::ILLEGAL_STATE);

    return IMS_FAILURE;
}
