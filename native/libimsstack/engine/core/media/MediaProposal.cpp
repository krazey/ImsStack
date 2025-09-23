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

#include "ISdpOaState.h"
#include "media/Media.h"
#include "media/MediaProposal.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
MediaProposal::MediaProposal(IN ISdpOaState* piOaState) :
        m_piOaState(piOaState),
        m_objDescriptors(ImsList<MediaDescriptor*>())
{
}

PUBLIC VIRTUAL MediaProposal::~MediaProposal()
{
    IMS_TRACE_D("Destructor :: MediaProposal", 0, 0, 0);

    if (!m_objDescriptors.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objDescriptors.GetSize(); ++i)
        {
            MediaDescriptor* pDescriptor = m_objDescriptors.GetAt(i);

            if (pDescriptor != IMS_NULL)
            {
                delete pDescriptor;
            }
        }

        m_objDescriptors.Clear();
    }
}

PUBLIC
IMS_BOOL MediaProposal::CreateDescriptor(IN const ImsList<MediaDescriptor*>& objDescriptors)
{
    IMS_SINT32 nResult = m_piOaState->CreateProposalView();

    if ((nResult != ISdpOaState::RESULT_SUCCESS) && (nResult != ISdpOaState::RESULT_ALREADY_EXIST))
    {
        IMS_TRACE_E(0, "Creating a proposed view failed", 0, 0, 0);
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objDescriptors.GetSize(); ++i)
    {
        const MediaDescriptor* pDescriptor = objDescriptors.GetAt(i);
        MediaDescriptor* pNewDescriptor = new MediaDescriptor(this, pDescriptor->GetMid());

        if (pNewDescriptor == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a MediaDescriptor (%d) failed", i, 0, 0);
            return IMS_FALSE;
        }

        if (!m_objDescriptors.Append(pNewDescriptor))
        {
            delete pNewDescriptor;
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_SINT32 MediaProposal::GetDirection() const
{
    if (m_objDescriptors.IsEmpty())
    {
        return Media::DIRECTION_NONE;
    }

    const MediaDescriptor* pDescriptor = GetMediaDescriptor();

    if (pDescriptor == IMS_NULL)
    {
        return Media::DIRECTION_NONE;
    }

    const SdpMediaParameter* pMediaParam = GetPeerMediaParameter(pDescriptor->GetMid());

    if (pMediaParam == IMS_NULL)
    {
        return Media::DIRECTION_NONE;
    }

    IMS_SINT32 nDirection = pMediaParam->GetDirection();

    if (nDirection == Sdp::DIRECTION_INACTIVE)
    {
        return Media::DIRECTION_INACTIVE;
    }
    else if (nDirection == Sdp::DIRECTION_RECVONLY)
    {
        return Media::DIRECTION_RECEIVE;
    }
    else if (nDirection == Sdp::DIRECTION_SENDONLY)
    {
        return Media::DIRECTION_SEND;
    }
    else if (nDirection == Sdp::DIRECTION_SENDRECV)
    {
        return Media::DIRECTION_SEND_RECEIVE;
    }
    else
    {
        return Media::DIRECTION_NONE;
    }
}

PUBLIC
MediaDescriptor* MediaProposal::GetMediaDescriptor() const
{
    if (m_objDescriptors.IsEmpty())
    {
        return IMS_NULL;
    }

    return m_objDescriptors.GetAt(0);
}

PUBLIC
MediaDescriptor* MediaProposal::GetMediaDescriptor(IN IMS_SINT32 nMid) const
{
    for (IMS_UINT32 i = 0; i < m_objDescriptors.GetSize(); ++i)
    {
        MediaDescriptor* pDescriptor = m_objDescriptors.GetAt(i);

        if (nMid == pDescriptor->GetMid())
        {
            return pDescriptor;
        }
    }

    return IMS_NULL;
}

PROTECTED VIRTUAL const AString& MediaProposal::GetConnectionAddress() const
{
    SdpSessionParameter* pSessionParam = IMS_NULL;

    // First, check the current view
    m_piOaState->GetSessionCurrentView(pSessionParam);

    if (pSessionParam != IMS_NULL)
    {
        return pSessionParam->GetConnectionAddress();
    }

    // If the current view does not exist, then check the proposed view
    m_piOaState->GetSessionProposalView(pSessionParam);

    if (pSessionParam != IMS_NULL)
    {
        return pSessionParam->GetConnectionAddress();
    }

    return AString::ConstNull();
}

PROTECTED VIRTUAL SdpMediaParameter* MediaProposal::GetMediaParameter(IN IMS_SINT32 nMid) const
{
    SdpMediaParameter* pMediaParam = IMS_NULL;

    if (m_piOaState->GetMediaProposalView(nMid, pMediaParam) != ISdpOaState::RESULT_SUCCESS)
    {
        return IMS_NULL;
    }

    return pMediaParam;
}

PROTECTED VIRTUAL const AString& MediaProposal::GetPeerConnectionAddress() const
{
    SdpSessionParameter* pSessionParam = IMS_NULL;

    m_piOaState->GetSessionPeerView(pSessionParam);

    if (pSessionParam != IMS_NULL)
    {
        return pSessionParam->GetConnectionAddress();
    }

    return AString::ConstNull();
}

PROTECTED VIRTUAL SdpMediaParameter* MediaProposal::GetPeerMediaParameter(IN IMS_SINT32 nMid) const
{
    SdpMediaParameter* pMediaParam = IMS_NULL;

    if (m_piOaState->GetMediaPeerView(nMid, pMediaParam) != ISdpOaState::RESULT_SUCCESS)
    {
        return IMS_NULL;
    }

    return pMediaParam;
}

PROTECTED VIRTUAL SdpMediaParameter* MediaProposal::GetProposalMediaParameter(IN IMS_SINT32 nMid)
{
    SdpMediaParameter* pMediaParam = IMS_NULL;

    if (m_piOaState->GetMediaProposalView(nMid, pMediaParam) != ISdpOaState::RESULT_SUCCESS)
    {
        IMS_TRACE_E(0, "There is no proposed view", 0, 0, 0);
        return IMS_NULL;
    }

    return pMediaParam;
}
