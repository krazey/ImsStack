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

#include "media/FramedMedia.h"
#include "media/FramedMediaProposal.h"
#include "media/MediaDescriptor.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
FramedMedia::FramedMedia(IN Service* pService, IN ISdpOaState* piOaState) :
        Media(pService, piOaState)
{
    SetInitializationDone(IMS_TRUE);
}

PUBLIC VIRTUAL FramedMedia::~FramedMedia()
{
    IMS_TRACE_D("Destructor :: FramedMedia", 0, 0, 0);
}

PROTECTED VIRTUAL MediaProposal* FramedMedia::CreateMediaProposal(IN ISdpOaState* piOaState)
{
    FramedMediaProposal* pMediaProposal = new FramedMediaProposal(piOaState);

    if (pMediaProposal == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (!pMediaProposal->CreateDescriptor(GetMediaDescriptors()))
    {
        delete pMediaProposal;
        return IMS_NULL;
    }

    return pMediaProposal;
}
