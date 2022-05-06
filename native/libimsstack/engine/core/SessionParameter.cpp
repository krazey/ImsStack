/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090622  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "SdpAttribute.h"
#include "offeranswer/SdpOfferAnswer.h"
#include "SessionParameter.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
SessionParameter::SessionParameter() :
        strRemoteVersion(AString::ConstNull()),
        nMid(0)
{
}

PUBLIC
SessionParameter::SessionParameter(IN CONST SessionParameter& objRHS) :
        strRemoteVersion(objRHS.strRemoteVersion),
        objSessionParam(objRHS.objSessionParam),
        objMediaGroups(objRHS.objMediaGroups),
        nMid(objRHS.nMid)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objRHS.objMediaParams.GetSize(); ++i)
    {
        SdpMediaParameter* pMediaParam = objRHS.objMediaParams.GetAt(i);

        objMediaParams.Append(new SdpMediaParameter(*pMediaParam));
    }
}

PUBLIC
SessionParameter::~SessionParameter()
{
    Clear();
}

PUBLIC
SessionParameter& SessionParameter::operator=(IN CONST SessionParameter& objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        Clear();

        strRemoteVersion = objRHS.strRemoteVersion;

        objSessionParam = objRHS.objSessionParam;

        objMediaGroups = objRHS.objMediaGroups;

        nMid = objRHS.nMid;

        for (IMS_UINT32 i = 0; i < objRHS.objMediaParams.GetSize(); ++i)
        {
            SdpMediaParameter* pMediaParam = objRHS.objMediaParams.GetAt(i);

            objMediaParams.Append(new SdpMediaParameter(*pMediaParam));
        }
    }

    return (*this);
}

PUBLIC VIRTUAL const SdpSessionParameter& SessionParameter::GetSessionParameter() const
{
    return objSessionParam;
}

PUBLIC VIRTUAL IMS_SINT32 SessionParameter::GetMediaCount() const
{
    return static_cast<IMS_SINT32>(objMediaParams.GetSize());
}

PUBLIC VIRTUAL SdpMediaParameter* SessionParameter::GetMediaParameter(IN IMS_UINT32 nMid) const
{
#if 1
    if (nMid >= objMediaParams.GetSize())
    {
        return IMS_NULL;
    }

    return const_cast<SdpMediaParameter*>(objMediaParams.GetAt(nMid));
#else
    for (IMS_UINT32 i = 0; i < objMediaParams.GetSize(); ++i)
    {
        SdpMediaParameter* pMediaParam = objMediaParams.GetAt(i);

        if (pMediaParam->GetMid() == nMid)
            return pMediaParam;
    }

    return IMS_NULL;
#endif
}

PUBLIC
IMS_BOOL SessionParameter::Create(IN CONST SdpSessionDescription& objSessionDescription,
        IN CONST IMSList<SdpMediaDescription>& objMediaDescriptions)
{
    //---------------------------------------------------------------------------------------------

    if (!objSessionParam.Create(objSessionDescription))
    {
        return IMS_FALSE;
    }

    if (objMediaDescriptions.IsEmpty())
    {
        objMediaParams.Clear();
        objMediaGroups.Clear();

        IMS_TRACE_I("SessionParameter - NO MEDIA-LEVEL PARAMETERS", 0, 0, 0);
        return IMS_TRUE;
    }

    // Create a media parameter from the SDP media descriptions
    IMS_UINT32 nIndex;

    objMediaParams.Clear();

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

        if (!objMediaParams.Append(pMediaParam))
        {
            delete pMediaParam;

            return IMS_FALSE;
        }
    }

    // a-line for group; it's related to "mid" attribute.
    objMediaGroups.Clear();

    if (objMediaParams.GetSize() > 0)
    {
        IMSList<SdpAttribute> objGroups = objSessionParam.GetAttributes(SdpAttribute::GROUP);

        for (nIndex = 0; nIndex < objGroups.GetSize(); ++nIndex)
        {
            const SdpAttribute& objGroup = objGroups.GetAt(nIndex);
            SdpMediaGroup objMediaGroup;

            if (!objMediaGroup.Create(objGroup.GetAttributeValue(), objMediaParams))
            {
                return IMS_FALSE;
            }

            objMediaGroups.Append(objMediaGroup);

            objSessionParam.RemoveAttribute(objGroup);
        }
    }

    return IMS_TRUE;
}

PUBLIC
SdpSessionParameter& SessionParameter::GetSessionParameterNC()
{
    //---------------------------------------------------------------------------------------------

    return objSessionParam;
}

PUBLIC
SdpMediaParameter* SessionParameter::CreateMediaParameter()
{
    SdpMediaParameter* pMediaParam = new SdpMediaParameter(CreateMid());

    //---------------------------------------------------------------------------------------------

    if (pMediaParam == IMS_NULL)
        return IMS_NULL;

    if (!objMediaParams.Append(pMediaParam))
    {
        delete pMediaParam;
        return IMS_NULL;
    }

    return pMediaParam;
}

PUBLIC
const SdpMediaGroup* SessionParameter::GetMediaGroup(IN CONST AString& strMid) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objMediaGroups.GetSize(); ++i)
    {
        const SdpMediaGroup& objGroup = objMediaGroups.GetAt(i);
        const AStringArray& objMids = objGroup.GetMids();

        if (objMids.Contains(strMid))
        {
            return &objGroup;
        }
    }

    return IMS_NULL;
}

PUBLIC
const IMSList<SdpMediaParameter*>& SessionParameter::GetMediaParameters() const
{
    //---------------------------------------------------------------------------------------------

    return objMediaParams;
}

PUBLIC
const AString& SessionParameter::GetRemoteVersion() const
{
    //---------------------------------------------------------------------------------------------

    return strRemoteVersion;
}

PUBLIC
IMS_BOOL SessionParameter::IsSameVersion(IN CONST SessionParameter* pSessionParam) const
{
    //---------------------------------------------------------------------------------------------

    if (!strRemoteVersion.Equals(pSessionParam->strRemoteVersion))
    {
        IMS_TRACE_I("SDP Offer/Answer - Version (%s), Offered Version (%s)",
                strRemoteVersion.GetStr(), pSessionParam->strRemoteVersion.GetStr(), 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("SDP Offer/Answer - NO CHANGES (%s) ...", strRemoteVersion.GetStr(), 0, 0);

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

    //---------------------------------------------------------------------------------------------

    if (nMid >= objMediaParams.GetSize())
        return;

    SdpMediaParameter* pMediaParam = objMediaParams.GetAt(nMid);

    if (pMediaParam != IMS_NULL)
    {
        if (bRejectedOrRemoved)
        {
            pMediaParam->MarkRejectedOrRemoved();
        }
        else
        {
            for (IMS_UINT32 i = nMid + 1; i < objMediaParams.GetSize(); ++i)
            {
                SdpMediaParameter* pMediaParam = objMediaParams.GetAt(i);

                pMediaParam->SetMid(i - 1);
            }

            objMediaParams.RemoveAt(nMid);
        }

        // ???
        RemoveMediaFromGroup(nMid);
    }
}

PUBLIC
IMS_BOOL SessionParameter::FindGroupStartingWithMediaParameter(
        IN IMS_SINT32 nIndex, OUT IMSList<SdpMediaParameter*>& objGroupMediaParams) const
{
    SdpMediaParameter* pMediaParam = objMediaParams.GetAt(nIndex);

    //---------------------------------------------------------------------------------------------

    if (!pMediaParam->IsMidPresent())
    {
        objGroupMediaParams.Append(pMediaParam);
        return IMS_TRUE;
    }

    const AString& strMid = pMediaParam->GetAttributeMid();

    for (IMS_UINT32 i = 0; i < objMediaGroups.GetSize(); ++i)
    {
        const SdpMediaGroup& objGroup = objMediaGroups.GetAt(i);
        const AStringArray& objMids = objGroup.GetMids();

        if (!objMids.IsEmpty() && strMid.Equals(objMids.GetElementAt(0)))
        {
            const IMSList<IMS_SINT32> objMediaIndexes = objGroup.GetMediaStreams();
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
IMS_SINT32 SessionParameter::GenerateAnswer(IN CONST SessionParameter* pOffer,
        OUT SessionParameter*& pProposalView, OUT SessionParameter*& pPeerView)
{
    //---------------------------------------------------------------------------------------------

    // Copy the peer view from the SDP offer
    (*pPeerView) = (*pOffer);

    // Store a remote's session version when receiving an offer
    pPeerView->strRemoteVersion = objSessionParam.GetOrigin().GetSessionVersion();

    // Copy the local view from the SDP offer
    (*pProposalView) = (*this);

    // Store a remote's session version when receiving an offer
    pProposalView->strRemoteVersion = pOffer->objSessionParam.GetOrigin().GetSessionVersion();

    // Update the session direction
    pProposalView->objSessionParam.UpdateDirection(pOffer->objSessionParam);
    pProposalView->objSessionParam.UpdateDirection();

    // Copy Mid
    pProposalView->nMid = pOffer->nMid;

    // Copy all the media parameters
    pProposalView->objMediaGroups = pOffer->objMediaGroups;

    for (IMS_UINT32 i = 0; i < pOffer->objMediaParams.GetSize(); ++i)
    {
        SdpMediaParameter* pMediaParam = pOffer->objMediaParams.GetAt(i);

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

        pProposalView->objMediaParams.Append(pNewMediaParam);
    }

    return SdpOfferAnswer::RESULT_SUCCESS;
}

PUBLIC
IMS_SINT32 SessionParameter::GenerateAnswer(IN CONST SessionParameter* pOffer,
        OUT SessionParameter*& pProposalView, OUT SessionParameter*& pPeerView,
        IN IMS_SINT32 nOptions, IN IMS_BOOL bInitialOffer /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    // Initialize the local & remote SDP views
    if (!pProposalView->Create() || !pPeerView->Create())
    {
        return SdpOfferAnswer::RESULT_FAILURE;
    }

    // Compare & generate the session level parameters
    IMS_SINT32 nOAResult = CompareSessionParameters(IMS_TRUE, pOffer, pProposalView, pPeerView);

    if (nOAResult != SdpOfferAnswer::RESULT_SUCCESS)
    {
        IMS_TRACE_E(0, "Comparing session-level description failed (%d)", nOAResult, 0, 0);
        return nOAResult;
    }

    // Check the media groups
    nOAResult = CompareMediaGroups(pOffer, pProposalView, nOptions);

    if (nOAResult != SdpOfferAnswer::RESULT_SUCCESS)
    {
        IMS_TRACE_E(0, "Comparing the media groups failed (%d)", nOAResult, 0, 0);
        return nOAResult;
    }

    if ((nOptions & SdpOfferAnswer::F_MEDIA_PARAM) != SdpOfferAnswer::F_MEDIA_PARAM)
    {
        IMS_TRACE_I("SDP Offer/Answer - No media parameter comparison ...", 0, 0, 0);

        return SdpOfferAnswer::RESULT_SUCCESS;
    }

    // Compare & generate the media level parameters
    nOAResult = CompareMediaParameters(bInitialOffer, IMS_TRUE, pOffer, pProposalView, pPeerView);

    IMS_TRACE_I("SDP Offer/Answer - Generating Answer (%d)", nOAResult, 0, 0);

    if (nOAResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT)
    {
        RemovePreconditionsIfNotSupport(pProposalView, pPeerView);
    }

    return nOAResult;
}

PUBLIC
IMS_SINT32 SessionParameter::ProcessAnswer(IN CONST SessionParameter* pAnswer,
        OUT SessionParameter*& pProposalView, OUT SessionParameter*& pPeerView,
        IN IMS_SINT32 nOptions)
{
    //---------------------------------------------------------------------------------------------

    // Initialize the local & remote SDP views
    if (!pProposalView->Create() || !pPeerView->Create())
    {
        return SdpOfferAnswer::RESULT_FAILURE;
    }

    // Compare & generate the session level parameters
    IMS_SINT32 nOAResult = CompareSessionParameters(IMS_FALSE, pAnswer, pProposalView, pPeerView);

    if (nOAResult != SdpOfferAnswer::RESULT_SUCCESS)
    {
        IMS_TRACE_E(0, "Comparing session-level description failed (%d)", nOAResult, 0, 0);
        return nOAResult;
    }

    // Check the media groups
    nOAResult = CompareMediaGroups(pAnswer, pProposalView, nOptions);

    if (nOAResult != SdpOfferAnswer::RESULT_SUCCESS)
    {
        IMS_TRACE_E(0, "Comparing the media groups failed (%d)", nOAResult, 0, 0);
        return nOAResult;
    }

    // Compare & generate the media level parameters
    nOAResult = CompareMediaParameters(IMS_FALSE, IMS_FALSE, pAnswer, pProposalView, pPeerView);

    IMS_TRACE_I("SDP Offer/Answer - Processing Answer (%d)", nOAResult, 0, 0);

    if (nOAResult == SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT)
    {
        RemovePreconditionsIfNotSupport(pProposalView, pPeerView);
    }

    return nOAResult;
}

PUBLIC
AString SessionParameter::ToSDP() const
{
    AString strSDP;

    //---------------------------------------------------------------------------------------------

    // Form a session-level description
    strSDP.Append(objSessionParam.ToSdp());

    // Form a "group" attributes in the session-level
    if (!objMediaGroups.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objMediaGroups.GetSize(); ++i)
        {
            const SdpMediaGroup& objMediaGroup = objMediaGroups.GetAt(i);

            strSDP.Append(objMediaGroup.ToSdp());
        }
    }

    // Form a media-level description
    if (!objMediaParams.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objMediaParams.GetSize(); ++i)
        {
            const SdpMediaParameter* pMediaParam = objMediaParams.GetAt(i);

            strSDP.Append(pMediaParam->ToSdp());
        }
    }

    return strSDP;
}

PUBLIC
void SessionParameter::UpdateDirection(IN CONST SessionParameter* pOther)
{
    //---------------------------------------------------------------------------------------------

    if (pOther == IMS_NULL)
    {
        return;
    }

    objSessionParam.UpdateDirection(pOther->objSessionParam);

    IMS_TRACE_D("UpdateDirection :: Count of media parameter (this: %d, other: %d)",
            objMediaParams.GetSize(), pOther->objMediaParams.GetSize(), 0);

    IMS_UINT32 nMediaCount = objMediaParams.GetSize();

    if (nMediaCount > pOther->objMediaParams.GetSize())
    {
        nMediaCount = pOther->objMediaParams.GetSize();
    }

    for (IMS_UINT32 i = 0; i < nMediaCount; ++i)
    {
        SdpMediaParameter* pMediaParam = objMediaParams.GetAt(i);
        SdpMediaParameter* pOtherMediaParam = pOther->objMediaParams.GetAt(i);

        if ((pMediaParam == IMS_NULL) || (pOtherMediaParam == IMS_NULL))
        {
            IMS_TRACE_E(0, "UpdateDirection :: Media parameter is null (%p, %p)", pMediaParam,
                    pOtherMediaParam, 0);
            continue;
        }

        pMediaParam->UpdateDirection(*pOtherMediaParam);
    }
}

PUBLIC
void SessionParameter::UpdateRemoteVersion(IN CONST AString& strRemoteVersion)
{
    //---------------------------------------------------------------------------------------------

    this->strRemoteVersion = strRemoteVersion;
}

PRIVATE
void SessionParameter::Clear()
{
    //---------------------------------------------------------------------------------------------

    objMediaGroups.Clear();

    for (IMS_UINT32 i = 0; i < objMediaParams.GetSize(); ++i)
    {
        SdpMediaParameter* pMediaParam = objMediaParams.GetAt(i);
        delete pMediaParam;
    }

    objMediaParams.Clear();
}

PRIVATE
IMS_BOOL SessionParameter::Create()
{
    SdpSessionParameter objNewSessionParam;

    //---------------------------------------------------------------------------------------------

    Clear();

    objSessionParam = objNewSessionParam;

    return IMS_TRUE;
}

PRIVATE
IMS_SINT32 SessionParameter::CreateMid()
{
    //---------------------------------------------------------------------------------------------

    nMid = static_cast<IMS_SINT32>(objMediaParams.GetSize());

    return nMid;
}

PRIVATE
IMS_SINT32 SessionParameter::CompareMediaGroups(IN CONST SessionParameter* pPeerParam,
        OUT SessionParameter*& pProposalView, IN IMS_SINT32 nOptions)
{
    //---------------------------------------------------------------------------------------------

    // If both media groups are not present, just return TRUE
    if ((objMediaGroups.GetSize() == 0) && (pPeerParam->objMediaGroups.GetSize() == 0))
    {
        return SdpOfferAnswer::RESULT_SUCCESS;
    }

    // If the m-line is not aligned with the mids, then ignore the group.
    if (nOptions & SdpOfferAnswer::F_MEDIA_GROUP)
    {
        // If the peer does not understand the group attribute, then in the SDP answer,
        // the peer may remove the group attribute and all the mids.
        // So, in here we will return SUCCESS.
        if (pPeerParam->objMediaGroups.GetSize() == 0)
        {
            return SdpOfferAnswer::RESULT_SUCCESS;
        }

        if (pPeerParam->objMediaGroups.GetSize() > objMediaGroups.GetSize())
        {
            return SdpOfferAnswer::RESULT_FAILURE;
        }

        // Check if the mid attribute values are intact.
        // The mid values should be same as the one for all the media streams sent in INVITE.
        for (IMS_UINT32 i = 0; i < pPeerParam->objMediaParams.GetSize(); ++i)
        {
            const AString& strMid = objMediaParams.GetAt(i)->GetAttributeMid();
            const AString& strOfferMid = pPeerParam->objMediaParams.GetAt(i)->GetAttributeMid();

            // Compare the media stream id of each media streams
            if (!strOfferMid.EqualsIgnoreCase(strMid))
            {
                return SdpOfferAnswer::RESULT_FAILURE;
            }
        }
    }

    IMSList<SdpMediaGroup> objNegotiatedGroups;

    for (IMS_UINT32 nNewGr = 0; nNewGr < pPeerParam->objMediaGroups.GetSize(); ++nNewGr)
    {
        IMS_BOOL bGroupMatched = IMS_TRUE;
        const SdpMediaGroup& objOfferGroup = pPeerParam->objMediaGroups.GetAt(nNewGr);

        // If the group type is unknown, ignore the group
        if (objOfferGroup.GetType() == SdpMediaGroup::GROUP_OTHER)
            continue;

        if (nOptions & SdpOfferAnswer::F_MEDIA_GROUP)
        {
            // Validate the group: If no group is present in the old session parameter,
            // then the new one also shouldn't contain the group.
            // The number of groups in the answer will be the same or less.
            // If any group is not understood, then it will be ignored.

            // Find the group in the local session parameter which matches this new group
            for (IMS_UINT32 nOldGr = 0; nOldGr < objMediaGroups.GetSize(); ++nOldGr)
            {
                IMS_BOOL bMidMatched = IMS_FALSE;
                const SdpMediaGroup& objGroup = objMediaGroups.GetAt(nOldGr);

                if (objOfferGroup.GetType() != objGroup.GetType())
                    continue;

                const IMSList<IMS_SINT32>& objMid = objGroup.GetMediaStreams();
                const IMSList<IMS_SINT32>& objOfferMid = objOfferGroup.GetMediaStreams();

                for (IMS_UINT32 nOfferMid = 0; nOfferMid < objOfferMid.GetSize(); ++nOfferMid)
                {
                    IMS_SINT32 nOfferIndex = objOfferMid.GetAt(nOfferMid);
                    const SdpMediaParameter* pOfferMedia =
                            pPeerParam->objMediaParams.GetAt(nOfferIndex);
                    const AString& strMid = pOfferMedia->GetAttributeMid();

                    for (IMS_UINT32 nMid = 0; nMid < objMid.GetSize(); ++nMid)
                    {
                        IMS_SINT32 nIndex = objMid.GetAt(nMid);
                        const SdpMediaParameter* pMedia = objMediaParams.GetAt(nIndex);

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

    // if (!objNegotiatedGroups.IsEmpty())
    {
        pProposalView->objMediaGroups.Clear();
        pProposalView->objMediaGroups = objNegotiatedGroups;
    }

    return SdpOfferAnswer::RESULT_SUCCESS;
}

PRIVATE
IMS_SINT32 SessionParameter::CompareMediaParameters(IN IMS_BOOL bInitialOffer, IN IMS_BOOL bIsOffer,
        IN CONST SessionParameter* pPeerParam, OUT SessionParameter*& pProposalView,
        OUT SessionParameter*& pPeerView)
{
    IMS_SINT32 nMatchResult = SdpOfferAnswer::RESULT_NOT_DONE;
    IMS_BOOL bAtLeastOneMatched = IMS_FALSE;
    IMS_BOOL bQosPreconditionPresent = IMS_FALSE;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < pPeerParam->objMediaParams.GetSize(); ++i)
    {
        const SdpMediaParameter* pPeerMedia = pPeerParam->objMediaParams.GetAt(i);
        SdpMediaParameter* pProposalMedia = pProposalView->CreateMediaParameter();

        if (pProposalMedia == IMS_NULL)
        {
            IMS_TRACE_E(0, "Instantiating a SDP media parameter failed", 0, 0, 0);

            nMatchResult = SdpOfferAnswer::RESULT_FAILURE;

            break;
        }

        if (i < objMediaParams.GetSize())
        {
            SdpMediaParameter* pMedia = objMediaParams.GetAt(i);

            // Check if the media stream is reused the "slot" used by an old media stream
            // which had been disabled by setting its port to zero
            if (bIsOffer && (pMedia->GetMedia().GetPort() == 0) &&
                    (pPeerMedia->GetMedia().GetPort() != 0))
            {
                // 4 Does it need to check media_type?
                bAtLeastOneMatched = IMS_TRUE;
                nMatchResult = SdpOfferAnswer::RESULT_SUCCESS;

                // Newly added media stream
                (*pProposalMedia) = (*pPeerMedia);

                // Add the newly added media stream to the peer view
                SdpMediaParameter* pNegotiatedPeerMedia = new SdpMediaParameter(*pPeerMedia);

                if (pNegotiatedPeerMedia != IMS_NULL)
                {
                    pPeerView->objMediaParams.Append(pNegotiatedPeerMedia);
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
                        pPeerView->objMediaParams.Append(pNegotiatedPeerParam);
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

                    pPeerView->objMediaParams.Append(pNegotiatedPeerParam);
                }

                // Update the local properties in the negotiated media stream
                pProposalMedia->UpdateProperties(*pMedia);

                // Update the media direction in the negotiated media stream
                pProposalMedia->UpdateDirection();

                IMS_TRACE_D("Count of media formats :: Proposal (%d), Peer (%d)",
                        pProposalMedia->GetMediaFormats().GetSize(),
                        pNegotiatedPeerParam->GetMediaFormats().GetSize(), 0);
            }
        }
        else
        {
            bAtLeastOneMatched = IMS_TRUE;
            nMatchResult = SdpOfferAnswer::RESULT_SUCCESS;

            // Newly added media stream
            (*pProposalMedia) = (*pPeerMedia);

            // Add the newly added media stream to the peer view
            SdpMediaParameter* pNegotiatedPeerMedia = new SdpMediaParameter(*pPeerMedia);

            if (pNegotiatedPeerMedia != IMS_NULL)
            {
                pPeerView->objMediaParams.Append(pNegotiatedPeerMedia);
            }

            pProposalMedia->RemoveConnections();
            pProposalMedia->UpdateDirection();
        }

        if (pProposalMedia->IsQosPreconditionPresent())
            bQosPreconditionPresent = IMS_TRUE;
    }

    if (bAtLeastOneMatched)
    {
        nMatchResult = SdpOfferAnswer::RESULT_SUCCESS;

        if (bQosPreconditionPresent)
        {
            nMatchResult = SdpOfferAnswer::RESULT_QOS_PRECONDITION_PRESENT;
        }
    }
    else
    {
        nMatchResult = SdpOfferAnswer::RESULT_NOT_FOUND;
    }

#if 0
    if ((nMatchResult != SdpOfferAnswer::RESULT_FAILURE)
            && (nMatchResult != SdpOfferAnswer::RESULT_NOT_FOUND))
    {
    }
#endif

    return nMatchResult;
}

PRIVATE
IMS_SINT32 SessionParameter::CompareSessionParameters(IN IMS_BOOL bIsOffer,
        IN CONST SessionParameter* pPeerParam, OUT SessionParameter*& pProposalView,
        OUT SessionParameter*& pPeerView)
{
    //---------------------------------------------------------------------------------------------

    // Store a remote's session version when receiving an offer or answer
    pProposalView->strRemoteVersion = pPeerParam->objSessionParam.GetOrigin().GetSessionVersion();

    // Session-Level : Compare & Create a local SDP view
    IMS_SINT32 nOAResult =
            objSessionParam.Compare(pPeerParam->objSessionParam, pProposalView->objSessionParam);

    if (nOAResult != SdpOfferAnswer::RESULT_SUCCESS)
    {
        return nOAResult;
    }

    // Determine the direction in special case
    if (!bIsOffer)
    {
        objSessionParam.UpdateDirection(
                pPeerParam->objSessionParam, pProposalView->objSessionParam);
    }

    // Copy all the information from the session parameter
    pProposalView->objSessionParam.UpdateProperties(objSessionParam);

    // Update the session-level direction
    pProposalView->objSessionParam.UpdateDirection();

    // Session-Level : Create a remote SDP view
    pPeerView->objSessionParam = pPeerParam->objSessionParam;

    return SdpOfferAnswer::RESULT_SUCCESS;
}

PRIVATE
void SessionParameter::RemoveMediaFromGroup(IN IMS_SINT32 nMid)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objMediaGroups.GetSize(); ++i)
    {
        SdpMediaGroup& objMediaGroup = objMediaGroups.GetAt(i);

        if (objMediaGroup.RemoveMediaStream(nMid))
            return;
    }
}

PRIVATE
void SessionParameter::RemovePreconditionsIfNotSupport(
        OUT SessionParameter*& pProposalView, OUT SessionParameter*& pPeerView)
{
#if defined(__IMS_SDP_PRECONDITION__)
    const IMSList<SdpMediaParameter*>& objPeerMediaParams = pPeerView->GetMediaParameters();
    const IMSList<SdpMediaParameter*>& objLocalMediaParams = pProposalView->GetMediaParameters();

    for (IMS_UINT32 i = 0; i < objPeerMediaParams.GetSize(); ++i)
    {
        SdpMediaParameter* pPeerMediaParam = objPeerMediaParams.GetAt(i);

        if (pPeerMediaParam == IMS_NULL)
        {
            continue;
        }

        SdpPrecondition* pCurrent = pPeerMediaParam->GetPrecondition(SdpAttribute::CURR);
        SdpPrecondition* pDesired = pPeerMediaParam->GetPrecondition(SdpAttribute::DES);

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
