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

#include "SdpAttribute.h"
#include "SdpMediaDescription.h"
#include "offeranswer/SdpOfferAnswer.h"

#include "SessionParameter.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
SessionParameter::SessionParameter() :
        m_strRemoteVersion(AString::ConstNull()),
        m_bLastSdpProvidedWithNegotiatedSdp(IMS_FALSE),
        m_nMid(0)
{
}

PUBLIC
SessionParameter::SessionParameter(IN const SessionParameter& other) :
        m_strRemoteVersion(other.m_strRemoteVersion),
        m_bLastSdpProvidedWithNegotiatedSdp(other.m_bLastSdpProvidedWithNegotiatedSdp),
        m_objSessionParam(other.m_objSessionParam),
        m_objMediaGroups(other.m_objMediaGroups),
        m_nMid(other.m_nMid)
{
    for (IMS_UINT32 i = 0; i < other.m_objMediaParams.GetSize(); ++i)
    {
        const SdpMediaParameter* pMediaParam = other.m_objMediaParams.GetAt(i);
        m_objMediaParams.Append(new SdpMediaParameter(*pMediaParam));
    }
}

PUBLIC
SessionParameter::~SessionParameter()
{
    Clear();
}

PUBLIC
SessionParameter& SessionParameter::operator=(IN const SessionParameter& other)
{
    if (this != &other)
    {
        Clear();

        m_strRemoteVersion = other.m_strRemoteVersion;
        m_bLastSdpProvidedWithNegotiatedSdp = other.m_bLastSdpProvidedWithNegotiatedSdp;
        m_objSessionParam = other.m_objSessionParam;
        m_objMediaGroups = other.m_objMediaGroups;
        m_nMid = other.m_nMid;

        for (IMS_UINT32 i = 0; i < other.m_objMediaParams.GetSize(); ++i)
        {
            const SdpMediaParameter* pMediaParam = other.m_objMediaParams.GetAt(i);
            m_objMediaParams.Append(new SdpMediaParameter(*pMediaParam));
        }
    }

    return (*this);
}

PUBLIC VIRTUAL SdpMediaParameter* SessionParameter::GetMediaParameter(IN IMS_UINT32 nMid) const
{
    if (nMid >= m_objMediaParams.GetSize())
    {
        return IMS_NULL;
    }

    return const_cast<SdpMediaParameter*>(m_objMediaParams.GetAt(nMid));
}

PUBLIC
IMS_BOOL SessionParameter::Create(IN const SdpSessionDescription& objSessionDescription,
        IN const ImsList<SdpMediaDescription>& objMediaDescriptions)
{
    if (!m_objSessionParam.Create(objSessionDescription))
    {
        return IMS_FALSE;
    }

    if (objMediaDescriptions.IsEmpty())
    {
        m_objMediaParams.Clear();
        m_objMediaGroups.Clear();

        IMS_TRACE_I("SessionParameter - NO MEDIA-LEVEL PARAMETERS", 0, 0, 0);
        return IMS_TRUE;
    }

    // Create a media parameter from the SDP media descriptions
    IMS_UINT32 nIndex;

    m_objMediaParams.Clear();

    for (nIndex = 0; nIndex < objMediaDescriptions.GetSize(); ++nIndex)
    {
        const SdpMediaDescription& objMediaDescription = objMediaDescriptions.GetAt(nIndex);

        SdpMediaParameter* pMediaParam = new SdpMediaParameter(CreateMid());

        if (pMediaParam == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (!pMediaParam->Create(objMediaDescription))
        {
            delete pMediaParam;
            return IMS_FALSE;
        }

        if (!m_objMediaParams.Append(pMediaParam))
        {
            delete pMediaParam;
            return IMS_FALSE;
        }
    }

    // a-line for group; it's related to "mid" attribute.
    m_objMediaGroups.Clear();

    if (m_objMediaParams.GetSize() > 0)
    {
        ImsList<SdpAttribute> objGroups = m_objSessionParam.GetAttributes(SdpAttribute::GROUP);

        for (nIndex = 0; nIndex < objGroups.GetSize(); ++nIndex)
        {
            const SdpAttribute& objGroup = objGroups.GetAt(nIndex);
            SdpMediaGroup objMediaGroup;

            if (!objMediaGroup.Create(objGroup.GetAttributeValue(), m_objMediaParams))
            {
                return IMS_FALSE;
            }

            m_objMediaGroups.Append(objMediaGroup);
            m_objSessionParam.RemoveAttribute(objGroup);
        }
    }

    return IMS_TRUE;
}

PUBLIC
SdpMediaParameter* SessionParameter::CreateMediaParameter()
{
    SdpMediaParameter* pMediaParam = new SdpMediaParameter(CreateMid());

    if (pMediaParam == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (!m_objMediaParams.Append(pMediaParam))
    {
        delete pMediaParam;
        return IMS_NULL;
    }

    return pMediaParam;
}

PUBLIC
const SdpMediaGroup* SessionParameter::GetMediaGroup(IN const AString& strMid) const
{
    for (IMS_UINT32 i = 0; i < m_objMediaGroups.GetSize(); ++i)
    {
        const SdpMediaGroup& objGroup = m_objMediaGroups.GetAt(i);
        const AStringArray& objMids = objGroup.GetMids();

        if (objMids.Contains(strMid))
        {
            return &objGroup;
        }
    }

    return IMS_NULL;
}

PUBLIC
IMS_BOOL SessionParameter::IsSameVersion(IN const SessionParameter* pSessionParam) const
{
    if (!m_strRemoteVersion.Equals(pSessionParam->m_strRemoteVersion))
    {
        IMS_TRACE_I("SDP Offer/Answer - Version (%s), Offered Version (%s)",
                m_strRemoteVersion.GetStr(), pSessionParam->m_strRemoteVersion.GetStr(), 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("SDP Offer/Answer - NO CHANGES (%s) ...", m_strRemoteVersion.GetStr(), 0, 0);

    return IMS_TRUE;
}

PUBLIC
void SessionParameter::RemoveMediaParameter(IN IMS_UINT32 nMid, IN IMS_BOOL bRejectedOrRemoved)
{
    // This method does not remove a real SDP media parameter,
    // but it just sets the port number of m-line to zero.
    // If the session parameter is from an incoming offer,
    // it will be rejected by the IMS engine or application.
    // If the session parameter is from an outgoing offer,
    // it will be removed by the application.
    if (nMid >= m_objMediaParams.GetSize())
    {
        return;
    }

    SdpMediaParameter* pMediaParam = m_objMediaParams.GetAt(nMid);

    if (pMediaParam != IMS_NULL)
    {
        if (bRejectedOrRemoved)
        {
            pMediaParam->MarkRejectedOrRemoved();
        }
        else
        {
            for (IMS_UINT32 i = nMid + 1; i < m_objMediaParams.GetSize(); ++i)
            {
                pMediaParam = m_objMediaParams.GetAt(i);
                pMediaParam->SetMid(i - 1);
            }

            m_objMediaParams.RemoveAt(nMid);
        }

        RemoveMediaFromGroup(nMid);
    }
}

PUBLIC
IMS_BOOL SessionParameter::FindGroupStartingWithMediaParameter(
        IN IMS_SINT32 nIndex, OUT ImsList<SdpMediaParameter*>& objGroupMediaParams) const
{
    SdpMediaParameter* pMediaParam = m_objMediaParams.GetAt(nIndex);

    if (!pMediaParam->IsMidPresent())
    {
        objGroupMediaParams.Append(pMediaParam);
        return IMS_TRUE;
    }

    const AString& strMid = pMediaParam->GetAttributeMid();

    for (IMS_UINT32 i = 0; i < m_objMediaGroups.GetSize(); ++i)
    {
        const SdpMediaGroup& objGroup = m_objMediaGroups.GetAt(i);
        const AStringArray& objMids = objGroup.GetMids();

        if (!objMids.IsEmpty() && strMid.Equals(objMids.GetElementAt(0)))
        {
            const ImsList<IMS_SINT32> objMediaIndexes = objGroup.GetMediaStreams();
            // The mid we are looking for is first in the group, this is the group we are
            // looking for. Find the other media parameters indicated in the group: attribute.
            for (IMS_UINT32 j = 0; j < objMediaIndexes.GetSize(); ++j)
            {
                objGroupMediaParams.Append(GetMediaParameter(objMediaIndexes.GetAt(j)));
            }

            return IMS_TRUE;
        }

        // The media parameter is not first in the group, is it on any other position?
        if (objMids.Contains(strMid))
        {
            return IMS_FALSE;
        }
    }

    // The media parameter has an ID tag but is not part of any (valid) group.
    // Create a group with only the parameter in it.
    objGroupMediaParams.Append(pMediaParam);

    return IMS_TRUE;
}

PUBLIC
IMS_SINT32 SessionParameter::GenerateAnswer(IN const SessionParameter* pOffer,
        OUT SessionParameter*& pProposalView, OUT SessionParameter*& pPeerView)
{
    // Copy the peer view from the SDP offer
    (*pPeerView) = (*pOffer);

    // Store a remote's session version when receiving an offer
    pPeerView->m_strRemoteVersion = m_objSessionParam.GetOrigin().GetSessionVersion();

    // Copy the local view from the SDP offer
    (*pProposalView) = (*this);

    // Store a remote's session version when receiving an offer
    pProposalView->m_strRemoteVersion = pOffer->m_objSessionParam.GetOrigin().GetSessionVersion();

    // Update the session direction
    pProposalView->m_objSessionParam.UpdateDirection(pOffer->m_objSessionParam);
    pProposalView->m_objSessionParam.UpdateDirection();

    // Copy Mid
    pProposalView->m_nMid = pOffer->m_nMid;

    // Copy all the media parameters
    pProposalView->m_objMediaGroups = pOffer->m_objMediaGroups;

    for (IMS_UINT32 i = 0; i < pOffer->m_objMediaParams.GetSize(); ++i)
    {
        SdpMediaParameter* pMediaParam = pOffer->m_objMediaParams.GetAt(i);

        if (pMediaParam == IMS_NULL)
        {
            IMS_TRACE_E(0, "Getting SDP media parameter from the offerred SDP failed", 0, 0, 0);
            continue;
        }

        SdpMediaParameter* pNewMediaParam = new SdpMediaParameter(*pMediaParam);

        if (pNewMediaParam == IMS_NULL)
        {
            IMS_TRACE_E(0, "Instantiating SDP media parameter failed", 0, 0, 0);
            continue;
        }

        pNewMediaParam->RemoveConnections();
        pNewMediaParam->UpdateDirection();

        pProposalView->m_objMediaParams.Append(pNewMediaParam);
    }

    return SdpOfferAnswer::RESULT_SUCCESS;
}

PUBLIC
IMS_SINT32 SessionParameter::GenerateAnswer(IN const SessionParameter* pOffer,
        OUT SessionParameter*& pProposalView, OUT SessionParameter*& pPeerView,
        IN IMS_SINT32 nOptions, IN IMS_BOOL bInitialOffer /*= IMS_FALSE*/)
{
    // Initialize the local & remote SDP views
    pProposalView->Create();
    pPeerView->Create();

    // Compare & generate the session level parameters
    IMS_SINT32 nOaResult = CompareSessionParameters(IMS_TRUE, pOffer, pProposalView, pPeerView);

    if (nOaResult != SdpOfferAnswer::RESULT_SUCCESS)
    {
        IMS_TRACE_E(0, "Comparing session-level description failed (%d)", nOaResult, 0, 0);
        return nOaResult;
    }

    // Check the media groups
    nOaResult = CompareMediaGroups(pOffer, pProposalView, nOptions);

    if (nOaResult != SdpOfferAnswer::RESULT_SUCCESS)
    {
        IMS_TRACE_E(0, "Comparing the media groups failed (%d)", nOaResult, 0, 0);
        return nOaResult;
    }

    if ((nOptions & SdpOfferAnswer::F_MEDIA_PARAM) != SdpOfferAnswer::F_MEDIA_PARAM)
    {
        IMS_TRACE_I("SDP Offer/Answer - No media parameter comparison ...", 0, 0, 0);

        return SdpOfferAnswer::RESULT_SUCCESS;
    }

    // Compare & generate the media level parameters
    nOaResult = CompareMediaParameters(bInitialOffer, IMS_TRUE, pOffer, pProposalView, pPeerView);

    IMS_TRACE_I("SDP Offer/Answer - Generating Answer (%d)", nOaResult, 0, 0);

    if (nOaResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT)
    {
        RemovePreconditionsIfNotSupport(pProposalView, pPeerView);
    }

    return nOaResult;
}

PUBLIC
IMS_SINT32 SessionParameter::ProcessAnswer(IN const SessionParameter* pAnswer,
        OUT SessionParameter*& pProposalView, OUT SessionParameter*& pPeerView,
        IN IMS_SINT32 nOptions)
{
    // Initialize the local & remote SDP views
    pProposalView->Create();
    pPeerView->Create();

    // Compare & generate the session level parameters
    IMS_SINT32 nOaResult = CompareSessionParameters(IMS_FALSE, pAnswer, pProposalView, pPeerView);

    if (nOaResult != SdpOfferAnswer::RESULT_SUCCESS)
    {
        IMS_TRACE_E(0, "Comparing session-level description failed (%d)", nOaResult, 0, 0);
        return nOaResult;
    }

    // Check the media groups
    nOaResult = CompareMediaGroups(pAnswer, pProposalView, nOptions);

    if (nOaResult != SdpOfferAnswer::RESULT_SUCCESS)
    {
        IMS_TRACE_E(0, "Comparing the media groups failed (%d)", nOaResult, 0, 0);
        return nOaResult;
    }

    // Compare & generate the media level parameters
    nOaResult = CompareMediaParameters(IMS_FALSE, IMS_FALSE, pAnswer, pProposalView, pPeerView);

    IMS_TRACE_I("SDP Offer/Answer - Processing Answer (%d)", nOaResult, 0, 0);

    if (nOaResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT)
    {
        RemovePreconditionsIfNotSupport(pProposalView, pPeerView);
    }

    return nOaResult;
}

PUBLIC
AString SessionParameter::ToSdp() const
{
    AString strSdp;

    // Form a session-level description
    strSdp.Append(m_objSessionParam.ToSdp());

    // Form a "group" attributes in the session-level
    if (!m_objMediaGroups.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objMediaGroups.GetSize(); ++i)
        {
            const SdpMediaGroup& objMediaGroup = m_objMediaGroups.GetAt(i);

            strSdp.Append(objMediaGroup.ToSdp());
        }
    }

    // Form a media-level description
    if (!m_objMediaParams.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objMediaParams.GetSize(); ++i)
        {
            const SdpMediaParameter* pMediaParam = m_objMediaParams.GetAt(i);

            strSdp.Append(pMediaParam->ToSdp());
        }
    }

    return strSdp;
}

PUBLIC
void SessionParameter::UpdateDirection(IN const SessionParameter* pOther)
{
    if (pOther == IMS_NULL)
    {
        return;
    }

    m_objSessionParam.UpdateDirection(pOther->m_objSessionParam);

    IMS_TRACE_D("UpdateDirection :: Count of media parameter (this: %d, other: %d)",
            m_objMediaParams.GetSize(), pOther->m_objMediaParams.GetSize(), 0);

    IMS_UINT32 nMediaCount = m_objMediaParams.GetSize();

    if (nMediaCount > pOther->m_objMediaParams.GetSize())
    {
        nMediaCount = pOther->m_objMediaParams.GetSize();
    }

    for (IMS_UINT32 i = 0; i < nMediaCount; ++i)
    {
        SdpMediaParameter* pMediaParam = m_objMediaParams.GetAt(i);
        SdpMediaParameter* pOtherMediaParam = pOther->m_objMediaParams.GetAt(i);

        if ((pMediaParam == IMS_NULL) || (pOtherMediaParam == IMS_NULL))
        {
            IMS_TRACE_E(0, "UpdateDirection :: Media parameter is null (%p, %p)", pMediaParam,
                    pOtherMediaParam, 0);
            continue;
        }

        pMediaParam->UpdateDirection(*pOtherMediaParam);
    }
}

PRIVATE
void SessionParameter::Clear()
{
    m_objMediaGroups.Clear();

    for (IMS_UINT32 i = 0; i < m_objMediaParams.GetSize(); ++i)
    {
        SdpMediaParameter* pMediaParam = m_objMediaParams.GetAt(i);
        delete pMediaParam;
    }

    m_objMediaParams.Clear();
}

PRIVATE
void SessionParameter::Create()
{
    SdpSessionParameter objNewSessionParam;

    Clear();

    m_objSessionParam = objNewSessionParam;
}

PRIVATE
IMS_SINT32 SessionParameter::CreateMid()
{
    m_nMid = static_cast<IMS_SINT32>(m_objMediaParams.GetSize());
    return m_nMid;
}

PRIVATE
IMS_SINT32 SessionParameter::CompareMediaGroups(IN const SessionParameter* pPeerParam,
        OUT SessionParameter*& pProposalView, IN IMS_SINT32 nOptions)
{
    // If both media groups are not present, just return TRUE
    if ((m_objMediaGroups.GetSize() == 0) && (pPeerParam->m_objMediaGroups.GetSize() == 0))
    {
        return SdpOfferAnswer::RESULT_SUCCESS;
    }

    // If the m-line is not aligned with the mids, then ignore the group.
    if (nOptions & SdpOfferAnswer::F_MEDIA_GROUP)
    {
        // If the peer does not understand the group attribute, then in the SDP answer,
        // the peer may remove the group attribute and all the mids.
        // So, in here we will return SUCCESS.
        if (pPeerParam->m_objMediaGroups.GetSize() == 0)
        {
            return SdpOfferAnswer::RESULT_SUCCESS;
        }

        if (pPeerParam->m_objMediaGroups.GetSize() > m_objMediaGroups.GetSize())
        {
            return SdpOfferAnswer::RESULT_FAILURE;
        }

        // Check if the mid attribute values are intact.
        // The mid values should be same as the one for all the media streams sent in INVITE.
        for (IMS_UINT32 i = 0; i < pPeerParam->m_objMediaParams.GetSize(); ++i)
        {
            const AString& strMid = m_objMediaParams.GetAt(i)->GetAttributeMid();
            const AString& strOfferMid = pPeerParam->m_objMediaParams.GetAt(i)->GetAttributeMid();

            // Compare the media stream id of each media streams
            if (!strOfferMid.EqualsIgnoreCase(strMid))
            {
                return SdpOfferAnswer::RESULT_FAILURE;
            }
        }
    }

    ImsList<SdpMediaGroup> objNegotiatedGroups;

    for (IMS_UINT32 nNewGr = 0; nNewGr < pPeerParam->m_objMediaGroups.GetSize(); ++nNewGr)
    {
        IMS_BOOL bGroupMatched = IMS_TRUE;
        const SdpMediaGroup& objOfferGroup = pPeerParam->m_objMediaGroups.GetAt(nNewGr);

        // If the group type is unknown, ignore the group
        if (objOfferGroup.GetType() == SdpMediaGroup::GROUP_OTHER)
        {
            continue;
        }

        if (nOptions & SdpOfferAnswer::F_MEDIA_GROUP)
        {
            // Validate the group: If no group is present in the old session parameter,
            // then the new one also shouldn't contain the group.
            // The number of groups in the answer will be the same or less.
            // If any group is not understood, then it will be ignored.

            // Find the group in the local session parameter which matches this new group
            for (IMS_UINT32 nOldGr = 0; nOldGr < m_objMediaGroups.GetSize(); ++nOldGr)
            {
                IMS_BOOL bMidMatched = IMS_FALSE;
                const SdpMediaGroup& objGroup = m_objMediaGroups.GetAt(nOldGr);

                if (objOfferGroup.GetType() != objGroup.GetType())
                {
                    continue;
                }

                const ImsList<IMS_SINT32>& objMid = objGroup.GetMediaStreams();
                const ImsList<IMS_SINT32>& objOfferMid = objOfferGroup.GetMediaStreams();

                for (IMS_UINT32 nOfferMid = 0; nOfferMid < objOfferMid.GetSize(); ++nOfferMid)
                {
                    IMS_SINT32 nOfferIndex = objOfferMid.GetAt(nOfferMid);
                    const SdpMediaParameter* pOfferMedia =
                            pPeerParam->m_objMediaParams.GetAt(nOfferIndex);
                    const AString& strMid = pOfferMedia->GetAttributeMid();

                    for (IMS_UINT32 nMid = 0; nMid < objMid.GetSize(); ++nMid)
                    {
                        IMS_SINT32 nIndex = objMid.GetAt(nMid);
                        const SdpMediaParameter* pMedia = m_objMediaParams.GetAt(nIndex);

                        if (strMid.EqualsIgnoreCase(pMedia->GetAttributeMid()))
                        {
                            bMidMatched = IMS_TRUE;
                            break;
                        }
                    }

                    if (bMidMatched == IMS_FALSE)
                    {
                        bGroupMatched = IMS_FALSE;
                        break;
                    }
                }

                if (bMidMatched == IMS_TRUE)
                {
                    bGroupMatched = IMS_TRUE;
                    break;
                }
            }

            if (bGroupMatched == IMS_FALSE)
            {
                return SdpOfferAnswer::RESULT_FAILURE;
            }
        }

        if (bGroupMatched == IMS_TRUE)
        {
            objNegotiatedGroups.Append(objOfferGroup);
        }
    }

    pProposalView->m_objMediaGroups.Clear();
    pProposalView->m_objMediaGroups = objNegotiatedGroups;

    return SdpOfferAnswer::RESULT_SUCCESS;
}

PRIVATE
IMS_SINT32 SessionParameter::CompareMediaParameters(IN IMS_BOOL bInitialOffer, IN IMS_BOOL bIsOffer,
        IN const SessionParameter* pPeerParam, OUT SessionParameter*& pProposalView,
        OUT SessionParameter*& pPeerView)
{
    IMS_SINT32 nMatchResult;
    IMS_BOOL bAtLeastOneMatched = IMS_FALSE;
    IMS_BOOL bQosPreconditionPresent = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < pPeerParam->m_objMediaParams.GetSize(); ++i)
    {
        const SdpMediaParameter* pPeerMedia = pPeerParam->m_objMediaParams.GetAt(i);
        SdpMediaParameter* pProposalMedia = pProposalView->CreateMediaParameter();

        if (pProposalMedia == IMS_NULL)
        {
            IMS_TRACE_E(0, "Instantiating a SDP media parameter failed", 0, 0, 0);
            break;
        }

        if (i < m_objMediaParams.GetSize())
        {
            SdpMediaParameter* pMedia = m_objMediaParams.GetAt(i);

            // Check if the media stream is reused the "slot" used by an old media stream
            // which had been disabled by setting its port to zero
            if (bIsOffer && (pMedia->GetMedia().GetPort() == 0) &&
                    (pPeerMedia->GetMedia().GetPort() != 0))
            {
                // 4 Does it need to check media_type?
                bAtLeastOneMatched = IMS_TRUE;

                // Newly added media stream
                (*pProposalMedia) = (*pPeerMedia);

                // Add the newly added media stream to the peer view
                SdpMediaParameter* pNegotiatedPeerMedia = new SdpMediaParameter(*pPeerMedia);

                if (pNegotiatedPeerMedia != IMS_NULL)
                {
                    pPeerView->m_objMediaParams.Append(pNegotiatedPeerMedia);
                }

                pProposalMedia->RemoveConnections();
                pProposalMedia->UpdateDirection();
            }
            else
            {
                SdpMediaParameter* pNegotiatedPeerParam = IMS_NULL;

                nMatchResult = pMedia->Compare(
                        bInitialOffer, bIsOffer, pPeerMedia, pNegotiatedPeerParam, pProposalMedia);

                if (nMatchResult != SdpOfferAnswer::RESULT_SUCCESS)
                {
                    // Check if the media stream is re-used or not;
                    // if the port of previous SDP was set to zero
                    (*pProposalMedia) = (*pPeerMedia);

                    pNegotiatedPeerParam = new SdpMediaParameter(*pPeerMedia);

                    if (pNegotiatedPeerParam != IMS_NULL)
                    {
                        pPeerView->m_objMediaParams.Append(pNegotiatedPeerParam);
                    }

                    pProposalMedia->RemoveConnections();
                    pProposalMedia->UpdateDirection();

                    // Not supported media stream; Set the port to zero.
                    pProposalMedia->MarkRejectedOrRemoved();

                    // Remove the media group
                    RemoveMediaFromGroup(pProposalMedia->GetMid());

                    continue;
                }

                // m-line match found
                bAtLeastOneMatched = IMS_TRUE;

                // Determine the direction in special case
                if (!bIsOffer)
                {
                    pMedia->UpdateDirection(*pPeerMedia, *pProposalMedia);
                }

                // In the accepted media streams, we shall be giving the peer's view of the stream
                // (i.e. peer's port / address and etc.) so before updating with
                // the local information in the media stream, it needs to be newly created.
                // objNegotiatedMedias.Append(objOfferMedia);
                if (pNegotiatedPeerParam != IMS_NULL)
                {
                    // Update the negotiated media formats
                    pNegotiatedPeerParam->UpdateProperties(*pPeerMedia);

                    pPeerView->m_objMediaParams.Append(pNegotiatedPeerParam);
                }

                // Update the local properties in the negotiated media stream
                pProposalMedia->UpdateProperties(*pMedia);

                // Update the media direction in the negotiated media stream
                pProposalMedia->UpdateDirection();

                IMS_TRACE_D("Count of media formats :: Proposal (%d), Peer (%d)",
                        pProposalMedia->GetMediaFormats().GetSize(),
                        (pNegotiatedPeerParam != IMS_NULL)
                                ? pNegotiatedPeerParam->GetMediaFormats().GetSize()
                                : 0,
                        0);
            }
        }
        else
        {
            bAtLeastOneMatched = IMS_TRUE;

            // Newly added media stream
            (*pProposalMedia) = (*pPeerMedia);

            // Add the newly added media stream to the peer view
            SdpMediaParameter* pNegotiatedPeerMedia = new SdpMediaParameter(*pPeerMedia);

            if (pNegotiatedPeerMedia != IMS_NULL)
            {
                pPeerView->m_objMediaParams.Append(pNegotiatedPeerMedia);
            }

            pProposalMedia->RemoveConnections();
            pProposalMedia->UpdateDirection();
        }

        if (pProposalMedia->IsQosPreconditionPresent())
        {
            bQosPreconditionPresent = IMS_TRUE;
        }
    }

    if (bAtLeastOneMatched)
    {
        if (bQosPreconditionPresent)
        {
            nMatchResult = SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT;
        }
        else
        {
            nMatchResult = SdpOfferAnswer::RESULT_SUCCESS;
        }
    }
    else
    {
        nMatchResult = SdpOfferAnswer::RESULT_NOT_FOUND;
    }

    return nMatchResult;
}

PRIVATE
IMS_SINT32 SessionParameter::CompareSessionParameters(IN IMS_BOOL bIsOffer,
        IN const SessionParameter* pPeerParam, OUT SessionParameter*& pProposalView,
        OUT SessionParameter*& pPeerView)
{
    // Store a remote's session version when receiving an offer or answer
    pProposalView->m_strRemoteVersion =
            pPeerParam->m_objSessionParam.GetOrigin().GetSessionVersion();

    // Session-Level : Compare & Create a local SDP view
    IMS_SINT32 nOaResult = m_objSessionParam.Compare(
            pPeerParam->m_objSessionParam, pProposalView->m_objSessionParam);

    if (nOaResult != SdpOfferAnswer::RESULT_SUCCESS)
    {
        return nOaResult;
    }

    // Determine the direction in special case
    if (!bIsOffer)
    {
        m_objSessionParam.UpdateDirection(
                pPeerParam->m_objSessionParam, pProposalView->m_objSessionParam);
    }

    // Copy all the information from the session parameter
    pProposalView->m_objSessionParam.UpdateProperties(m_objSessionParam);

    // Update the session-level direction
    pProposalView->m_objSessionParam.UpdateDirection();

    // Session-Level : Create a remote SDP view
    pPeerView->m_objSessionParam = pPeerParam->m_objSessionParam;

    return SdpOfferAnswer::RESULT_SUCCESS;
}

PRIVATE
void SessionParameter::RemoveMediaFromGroup(IN IMS_SINT32 nMid)
{
    for (IMS_UINT32 i = 0; i < m_objMediaGroups.GetSize(); ++i)
    {
        SdpMediaGroup& objMediaGroup = m_objMediaGroups.GetAt(i);

        if (objMediaGroup.RemoveMediaStream(nMid))
        {
            return;
        }
    }
}

PRIVATE
void SessionParameter::RemovePreconditionsIfNotSupport(
        IN const SessionParameter* pProposalView, IN const SessionParameter* pPeerView)
{
#if defined(__IMS_SDP_PRECONDITION__)
    const ImsList<SdpMediaParameter*>& objPeerMediaParams = pPeerView->GetMediaParameters();
    const ImsList<SdpMediaParameter*>& objLocalMediaParams = pProposalView->GetMediaParameters();

    for (IMS_UINT32 i = 0; i < objPeerMediaParams.GetSize(); ++i)
    {
        const SdpMediaParameter* pPeerMediaParam = objPeerMediaParams.GetAt(i);

        if (pPeerMediaParam == IMS_NULL)
        {
            continue;
        }

        const SdpPrecondition* pCurrent = pPeerMediaParam->GetPrecondition(SdpAttribute::CURR);
        const SdpPrecondition* pDesired = pPeerMediaParam->GetPrecondition(SdpAttribute::DES);

        if ((pCurrent != IMS_NULL) || (pDesired != IMS_NULL))
        {
            continue;
        }

        if (i >= objLocalMediaParams.GetSize())
        {
            continue;
        }

        SdpMediaParameter* pLocalMediaParam = objLocalMediaParams.GetAt(i);

        pLocalMediaParam->RemovePrecondition(SdpAttribute::CURR);
        pLocalMediaParam->RemovePrecondition(SdpAttribute::DES);
        pLocalMediaParam->RemovePrecondition(SdpAttribute::CONF);

        IMS_TRACE_I("SDP offer/answer :: The precondition is not supported by the peer;"
                    " remove the precondition attributes...",
                0, 0, 0);
    }
#endif
}
