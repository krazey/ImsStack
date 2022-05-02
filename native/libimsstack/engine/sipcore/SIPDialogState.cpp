/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090302  toastops@                 Created
    </table>

    Description
    This class contains constants representing SIP response codes as defined in RFC 3261
    and extensions.
*/

#include "ServiceMemory.h"
#include "SIPPrivate.h"
#include "SipFeatures.h"
#include "SipDebug.h"
// HEADER_REQ_SESSION-ID
#include "SipHeaderName.h"
#include "SIPMessage.h"
#include "SIPDialogSharedState.h"
#include "SIPDialogState.h"

#define __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__

__IMS_TRACE_TAG_SIP__;



PUBLIC
SIPDialogState::PendingRemoteTarget::PendingRemoteTarget()
    : strKey(AString::ConstNull())
    , pstHeader(IMS_NULL)
{
}

PUBLIC
SIPDialogState::PendingRemoteTarget::PendingRemoteTarget(
        IN CONST AString &strKey_, IN SipHeaderBase *pstHeader_)
    : strKey(strKey_)
    , pstHeader(pstHeader_)
{
}

PUBLIC
SIPDialogState::PendingRemoteTarget::~PendingRemoteTarget()
{
    if (pstHeader != IMS_NULL)
    {
        SIPStack::FreeHeaderEx(pstHeader);
    }
}



PUBLIC
SIPDialogState::SIPDialogState(IN IMS_BOOL bIsCaller_ /* = IMS_TRUE */)
    : RCObject()
    , bIsCaller(bIsCaller_)
    , pstLocalURI(IMS_NULL)
    , pstRemoteURI(IMS_NULL)
    , bSecure(IMS_FALSE)
    , pstLocalTargetURI(IMS_NULL)
    , pstRemoteTargetURI(IMS_NULL)
    , objPendingRemoteTargets(IMSList<PendingRemoteTarget*>())
    , nLocalCSeq(0)
    , nRemoteCSeq(0)
    , nRemoteCSeqForINVITE(0)
    , nState(SIPDState::STATE_INIT)
    , bPreloadedSet(IMS_FALSE)
    , pSharedState(IMS_NULL)
    , pLocalContactHeader(IMS_NULL)
    , strSessionId(AString::ConstNull()) // HEADER_REQ_SESSION-ID
    , nFromChangeOption(FROM_CHANGE_NONE)
{
    pstLocalURI = SIPStack::CreateHeader(SIPStack::GetAnyHeaderType());
    pstRemoteURI = SIPStack::CreateHeader(SIPStack::GetAnyHeaderType());

    // Local & Remote Contact information
    pstLocalTargetURI = SIPStack::CreateHeader(SIPStack::GetAnyHeaderType());
    pstRemoteTargetURI = SIPStack::CreateHeader(SIPStack::GetAnyHeaderType());
}

PUBLIC
SIPDialogState::SIPDialogState(IN CONST SIPDialogState &objRHS)
    : RCObject(objRHS)
{
    // NOTE: If reference count is not used, you MUST implement this copy constructor
}

PUBLIC VIRTUAL
SIPDialogState::~SIPDialogState()
{
    //---------------------------------------------------------------------------------------------

    if (pstLocalURI != IMS_NULL)
        SIPStack::FreeHeaderEx(pstLocalURI);

    if (pstRemoteURI != IMS_NULL)
        SIPStack::FreeHeaderEx(pstRemoteURI);

    if (pstLocalTargetURI != IMS_NULL)
        SIPStack::FreeHeaderEx(pstLocalTargetURI);

    if (pstRemoteTargetURI != IMS_NULL)
        SIPStack::FreeHeaderEx(pstRemoteTargetURI);

    ClearRouteSet();

    if (pSharedState != IMS_NULL)
    {
        delete pSharedState;
    }

    if (pLocalContactHeader != IMS_NULL)
    {
        delete pLocalContactHeader;
        pLocalContactHeader = IMS_NULL;
    }

    // REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
    RemoveAllPendingRemoteTargets();

    IMS_TRACE_D("Destructor :: SIPDialogState (%s : %s)",
            bIsCaller ? "__CALLER__" : "__CALLEE__",
            SIPDebug::GetCharA1(strCallId.GetStr(), 8, '@'), 0);
}

/*
 Checks if the message received has a valid To-Tag value.
If the dialog state has To-Tag and incoming message has no To-Tag,
then the validation will be failed.
If the invalid message is a request, then it rejects the request with 481 response.

Remarks

*/
PUBLIC
IMS_SINT32 SIPDialogState::CheckToTagValidity(IN CONST SIPMessageInfo &objMInfo)
{
    SipMessage *pstMessage = objMInfo.GetMessage();

    const SIPMethod &objMethod = objMInfo.GetMethod();

    //---------------------------------------------------------------------------------------------

    if (objMethod.Equals(SIPMethod::INVALID))
        return SIPPrivate::MESSAGE_VALID;

    // For CANCEL & non-2xx responses to CANCEL, relax the To-Tag validation.
    // As these are hop-by-hop responses, they can have different To-Tags.
    if (objMethod.Equals(SIPMethod::CANCEL))
        return SIPPrivate::MESSAGE_VALID;

    if (!SIPStack::IsRequestMessage(pstMessage))
    {
        if (objMethod.Equals(SIPMethod::INVITE)
                && (SIPStack::GetStatusCode(pstMessage) >= SIPStatusCode::SC_300))
        {
            return SIPPrivate::MESSAGE_VALID;
        }
    }

    IMS_BOOL bIsInvalid = IMS_FALSE;
    IMS_BOOL bLocalTagMismatched = IMS_FALSE;
    IMS_BOOL bRemoteTagMismatched = IMS_FALSE;

    AString strLocalTag = SIPStack::GetParameter(pstLocalURI, SIP::STR_TAG);
    AString strRemoteTag = SIPStack::GetParameter(pstRemoteURI, SIP::STR_TAG);
    AString strNewLocalTag;
    AString strNewRemoteTag;

    SipHeaderBase *pstHeader;

    if (SIPStack::IsRequestMessage(pstMessage))
    {
        // Get local tag
        pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::TO);

        strNewLocalTag = SIPStack::GetParameter(pstHeader, SIP::STR_TAG);
        SIPStack::FreeHeaderEx(pstHeader);

        // Get remote tag
        pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::FROM);

        strNewRemoteTag = SIPStack::GetParameter(pstHeader, SIP::STR_TAG);
        SIPStack::FreeHeaderEx(pstHeader);
    }
    else
    {
        // Get local tag
        pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::FROM);

        strNewLocalTag = SIPStack::GetParameter(pstHeader, SIP::STR_TAG);
        SIPStack::FreeHeaderEx(pstHeader);

        // Get remote tag
        pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::TO);

        strNewRemoteTag = SIPStack::GetParameter(pstHeader, SIP::STR_TAG);
        SIPStack::FreeHeaderEx(pstHeader);
    }

    // Check the local tag validation
    if ((strLocalTag.GetLength() != 0) && (strNewLocalTag.GetLength() == 0))
    {
        bIsInvalid = IMS_TRUE;
        bLocalTagMismatched = IMS_TRUE;
    }
    else if ((strLocalTag.GetLength() != 0) && (strNewLocalTag.GetLength() != 0))
    {
        if (!strLocalTag.Equals(strNewLocalTag))
        {
            bIsInvalid = IMS_TRUE;
            bLocalTagMismatched = IMS_TRUE;
        }
    }

    // Check the remote tag validation
    if ((strRemoteTag.GetLength() != 0) && (strNewRemoteTag.GetLength() == 0))
    {
        bIsInvalid = IMS_TRUE;
        bRemoteTagMismatched = IMS_TRUE;
    }
    else if ((strRemoteTag.GetLength() != 0) && (strNewRemoteTag.GetLength() != 0))
    {
        if (!strRemoteTag.Equals(strNewRemoteTag))
        {
            bIsInvalid = IMS_TRUE;
            bRemoteTagMismatched = IMS_TRUE;
        }
    }

    if (bIsInvalid == IMS_TRUE)
    {
        // Error : Tag Mismatch Error
        if (SIPStack::IsRequestMessage(pstMessage)
                && !objMethod.Equals(SIPMethod::INVITE))
        {
            // If the local tag is not matching,
            // then set the current local tag to this failed response.
            if (bLocalTagMismatched == IMS_TRUE)
            {
                SIPPrivate::SetLastError(SIPError::LOCAL_TAG_MISMATCH);
            }

            return SIPPrivate::MESSAGE_INVALID_481;
        }
        else if (!SIPStack::IsRequestMessage(pstMessage)
                && objMethod.Equals(SIPMethod::INVITE)
                && (bRemoteTagMismatched == IMS_TRUE))
        {
            // Handle the forked response to INVITE request.
            return SIPPrivate::MESSAGE_VALID_FORKED;
        }

        return SIPPrivate::MESSAGE_INVALID;
    }

    return SIPPrivate::MESSAGE_VALID;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 SIPDialogState::CompareTo(IN SIPDialogState *pDState, IN SipMessage *pstMessage,
        IN IMS_BOOL bCheckForked /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    // Check if the Call-ID's match
    if (!strCallId.Equals(pDState->strCallId))
    {
        // Call-Id's does not match
        return NOT_MATCHED;
    }

    IMS_SINT32 nComparisonResult = NOT_MATCHED;
    IMS_BOOL bToTagLenient = IMS_FALSE;
    IMS_SINT32 nForkedMessage = FORKED_ANY;
    SIPMethod objMethod = SIPStack::GetMethod(pstMessage);

    if (pstMessage != IMS_NULL)
    {
        IMS_SINT32 nStatusCode = SIPStatusCode::SC_INVALID;

        if (!SIPStack::IsRequestMessage(pstMessage))
        {
            nStatusCode = SIPStack::GetStatusCode(pstMessage);
        }

        // For response to CANCEL and non-2xx response to INVITE,
        // the comparison should be lenient against the To-Tags.

        // Fix for CSR 1-1316815
        // The To-Tag leniency should be present for CANCEL request,
        // response for CANCEL, and failure responses for INVITE.
        if (objMethod.Equals(SIPMethod::CANCEL))
            bToTagLenient = IMS_TRUE;

        else if (!SIPStack::IsRequestMessage(pstMessage)
                && (objMethod.Equals(SIPMethod::INVITE))
                && (nStatusCode >= SIPStatusCode::SC_300))
        {
            bToTagLenient = IMS_TRUE;
        }
        else if ((bCheckForked)
                && !SIPStack::IsRequestMessage(pstMessage)
                && (objMethod.Equals(SIPMethod::INVITE))
                && (nStatusCode >= SIPStatusCode::SC_100)
                && (nStatusCode < SIPStatusCode::SC_300))
        {
            bToTagLenient = IMS_TRUE;
            nForkedMessage = FORKED_INVITE;
        }
        // If the forking flag in enabled (i.e. Sdf_en_forkedSubscribe)
        // and it's a 1xx/2xx response to SUBSCRIBE, then set the bToTagLenient flag.
        // Also, reset the matching result since it is used only for detecting forked NOTIFYs.
        else if ((bCheckForked)
                && !SIPStack::IsRequestMessage(pstMessage)
                && (objMethod.Equals(SIPMethod::SUBSCRIBE))
                && (nStatusCode >= SIPStatusCode::SC_100)
                && (nStatusCode < SIPStatusCode::SC_300))
        {
            bToTagLenient = IMS_TRUE;
        }
        else if ((bCheckForked)
                && SIPStack::IsRequestMessage(pstMessage)
                && (objMethod.Equals(SIPMethod::NOTIFY)))
        {
            nForkedMessage = FORKED_SUBSCRIBE;
        }
        // FIXME: Strict comparison - UPDATE request
        #if 0
        else if (SIPStack::IsRequestMessage(pstMessage)
                && objMethod.Equals(SIPMethod::UPDATE))
        {
            IMS_SINT32 nDState = GetState();

            if ((nDState != SIPDState::STATE_EARLY)
                    && (nDState != SIPDState::STATE_CONFIRMED))
            {
                // If the dialog state is not proper, we don't need to compare the remains.
                return NOT_MATCHED;
            }
        }
        #endif
    }

    // Compares a local & remote headers on basis of
    // { (From : From) (To : To) } OR { (From : To) (To : From) }

    // Compares the first case (The local-address of each dialog-state).
    nComparisonResult = CompareHeaders(pDState->pstLocalURI,
                            pstLocalURI, bToTagLenient, nForkedMessage);

    if (nComparisonResult == MATCHED)
    {
        // First pair of headers matched.
        // Now, go in for matching the second set of headers viz. the remote-address
        // of each dialog-state and return back whatever this comparison returns.
        nComparisonResult = CompareHeaders(pDState->pstRemoteURI,
                                pstRemoteURI, bToTagLenient, nForkedMessage);
    }

    // If the SIP message is not NULL, use the type of the SIP message (Request/Response)
    // and the caller member variable in the existing dialog-state, to decide whether to do
    // (From:From, To:To) OR (From:To, To:From) [exact match]
    if ((pstMessage != IMS_NULL) && (nComparisonResult != NOT_MATCHED))
    {
        // If the comparison returned MATCHED_OVERLAP_DIALING, we check whether this is an INVITE.
        // If it's not an INVITE, we return NOT_MATCHED.
        // Similarly, if extra digits were added in a response, we return NOT_MATCHED.
        if (nComparisonResult == MATCHED_OVERLAP_DIALING)
        {
            if (!SIPStack::IsRequestMessage(pstMessage)
                    || (SIPStack::IsRequestMessage(pstMessage)
                        && !objMethod.Equals(SIPMethod::INVITE)))
            {
                nComparisonResult = NOT_MATCHED;
            }
        }

        if (nComparisonResult != NOT_MATCHED)
        {
            // If the request is CANCEL, then Equalss the Via branch parameter
            // also to have the exact match of dialog-state.
            if (SIPStack::IsRequestMessage(pstMessage)
                    && objMethod.Equals(SIPMethod::CANCEL))
            {
                if (!pDState->strRemoteViaBranch.IsNULL())
                {
                    if (!strRemoteViaBranch.IsNULL())
                    {
                        if (!strRemoteViaBranch.Equals(pDState->strRemoteViaBranch))
                            nComparisonResult = NOT_MATCHED;
                    }
                    else
                        nComparisonResult = NOT_MATCHED;
                }
                else
                {
                    if (!strRemoteViaBranch.IsNULL())
                        nComparisonResult = NOT_MATCHED;
                }
            }
        }
    }

    return nComparisonResult;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPDialogState::Equals(IN SIPDialogState *pDState)
{
    //---------------------------------------------------------------------------------------------

    if (pDState == IMS_NULL)
        return IMS_FALSE;

    if (!strCallId.Equals(pDState->strCallId))
        return IMS_FALSE;

    AString strTagVal = SIPStack::GetParameter(pstLocalURI, SIP::STR_TAG);
    AString strOtherTagVal = SIPStack::GetParameter(pDState->pstLocalURI, SIP::STR_TAG);

    if (!strTagVal.Equals(strOtherTagVal))
        return IMS_FALSE;

    strTagVal = SIPStack::GetParameter(pstRemoteURI, SIP::STR_TAG);
    strOtherTagVal = SIPStack::GetParameter(pDState->pstRemoteURI, SIP::STR_TAG);

    if (!strTagVal.Equals(strOtherTagVal))
        return IMS_FALSE;

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
const ISIPHeader* SIPDialogState::GetContactHeader() const
{
    //---------------------------------------------------------------------------------------------

    return pLocalContactHeader;
}

/*

Remarks

*/
PUBLIC
AString SIPDialogState::GetLocalTag() const
{
    //---------------------------------------------------------------------------------------------

    return SIPStack::GetParameter(pstLocalURI, SIP::STR_TAG);
}

/*

Remarks

*/
PUBLIC
AString SIPDialogState::GetRemoteTag() const
{
    //---------------------------------------------------------------------------------------------

    return SIPStack::GetParameter(pstRemoteURI, SIP::STR_TAG);
}

/*

Remarks

*/
PUBLIC
SipHeaderBase* SIPDialogState::GetLocalTargetURI() const
{
    //---------------------------------------------------------------------------------------------

    return pstLocalTargetURI;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPDialogState::InitDialogDetails(IN SipMessage *pstMessage)
{
    SipHeaderBase *pstHeader;
    SipHeaderBase *pstTmpHeader;

    //---------------------------------------------------------------------------------------------

    // Call Id
    pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::CALL_ID);
    SIPStack::EncodeHeaderBody(pstHeader, IMS_FALSE, strCallId);
    SIPStack::FreeHeaderEx(pstHeader);

    // The dialog state is created by the outgoing dialog-creatable request
    if (bIsCaller)
    {
        // Local URI & Tag
        SIPStack::FreeHeader(pstLocalURI);

        pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::FROM);

#ifdef __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__
        pstTmpHeader = pstHeader;
        pstHeader = IMS_NULL;
#else
        pstTmpHeader = SIPStack::CloneHeader(pstHeader);
#endif

        if (pstTmpHeader != IMS_NULL)
        {
            pstLocalURI = SIPStack::CopyHeader(pstLocalURI, pstTmpHeader);
            SIPStack::FreeHeaderEx(pstTmpHeader);
        }

        SIPStack::FreeHeaderEx(pstHeader);

        // Remote URI & Tag
        SIPStack::FreeHeader(pstRemoteURI);

        pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::TO);

#ifdef __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__
        pstTmpHeader = pstHeader;
        pstHeader = IMS_NULL;
#else
        pstTmpHeader = SIPStack::CloneHeader(pstHeader);
#endif

        if (pstTmpHeader != IMS_NULL)
        {
            pstRemoteURI = SIPStack::CopyHeader(pstRemoteURI, pstTmpHeader);
            SIPStack::FreeHeaderEx(pstTmpHeader);
        }

        SIPStack::FreeHeaderEx(pstHeader);

        // Via branch parameter
        pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::VIA);
        strLocalViaBranch = SIPStack::GetParameter(pstHeader, SIP::STR_BRANCH);
        SIPStack::FreeHeaderEx(pstHeader);
    }
    // The dialog state is created by the incoming dialog-creatable request
    else
    {
        // Local URI & Tag
        SIPStack::FreeHeader(pstLocalURI);

        pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::TO);

#ifdef __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__
        pstTmpHeader = pstHeader;
        pstHeader = IMS_NULL;
#else
        pstTmpHeader = SIPStack::CloneHeader(pstHeader);
#endif

        if (pstTmpHeader != IMS_NULL)
        {
            pstLocalURI = SIPStack::CopyHeader(pstLocalURI, pstTmpHeader);
            SIPStack::FreeHeaderEx(pstTmpHeader);
        }

        SIPStack::FreeHeaderEx(pstHeader);

        // Remote URI & Tag
        SIPStack::FreeHeader(pstRemoteURI);

        pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::FROM);

#ifdef __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__
        pstTmpHeader = pstHeader;
        pstHeader = IMS_NULL;
#else
        pstTmpHeader = SIPStack::CloneHeader(pstHeader);
#endif

        if (pstTmpHeader != IMS_NULL)
        {
            pstRemoteURI = SIPStack::CopyHeader(pstRemoteURI, pstTmpHeader);
            SIPStack::FreeHeaderEx(pstTmpHeader);
        }

        SIPStack::FreeHeaderEx(pstHeader);

        // Via branch parameter
        pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::VIA);
        strRemoteViaBranch = SIPStack::GetParameter(pstHeader, SIP::STR_BRANCH);
        SIPStack::FreeHeaderEx(pstHeader);
    }

    // Create a shared dialog state
    if (pSharedState == IMS_NULL)
    {
        pSharedState = new SIPDialogSharedState();

        if (pSharedState == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a shared dialog state failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPDialogState::InitDialogDetails(IN IMS_SINT32 nTrigger, IN SIPDialogState *pDState)
{
    //---------------------------------------------------------------------------------------------

    if (nTrigger == DIALOG_CANCELLED)
    {
        // Copy the remote sequence number from INVITE dialog...
        nRemoteCSeq = pDState->nRemoteCSeq;
        nRemoteCSeqForINVITE = pDState->nRemoteCSeqForINVITE;
    }
    else if (nTrigger == DIALOG_FORKED_REQUEST)
    {
        // Copy the information from the original dialog info.
        bIsCaller = pDState->bIsCaller;
        nLocalCSeq = pDState->nLocalCSeq;
    }
    else if (nTrigger == DIALOG_FORKED_RESPONSE)
    {
        // It will be updated from the previous request later...
        nLocalCSeq = 1;

#ifdef __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__
        SipHeaderBase *pstTmpHeader = SIPStack::CloneHeader(pstRemoteURI);

        if (pstTmpHeader != IMS_NULL)
        {
            SIPStack::FreeHeader(pstRemoteURI);

            pstRemoteURI = SIPStack::CopyHeader(pstRemoteURI, pstTmpHeader);
            SIPStack::FreeHeaderEx(pstTmpHeader);
        }

        // Remove the remote-tag because it is updated by the previous response
        // which is creating a dialog (early)
        SIPStack::RemoveParameter(SIP::STR_TAG, pstRemoteURI);
#else
        // Remove the remote-tag because it is updated by the previous response
        // which is creating a dialog (early)
        SIPStack::RemoveParameter(SIP::STR_TAG, pstRemoteURI);
#endif
    }
    else
    {
        return IMS_FALSE;
    }

    // HEADER_REQ_SESSION-ID
    strSessionId = pDState->strSessionId;

    if ((pDState->nFromChangeOption & FROM_CHANGE_ON_INVITE_REQUEST) != 0)
    {
        SetFromChangeOption(FROM_CHANGE_ON_INVITE_REQUEST);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPDialogState::InitRequest(IN CONST SIPMethod &objMethod,
        IN_OUT SipMessage *&pstMessage)
{
    SipHeaderBase *pstHeader;

    //---------------------------------------------------------------------------------------------

    // Method
    SIPStack::SetMethod(objMethod.ToString(), pstMessage);

    // Request-URI : Set the remote target URI
    SipAddrSpec *pstAddrSpec = SIPStack::GetAddrSpec(pstRemoteTargetURI);

    if (pstAddrSpec == IMS_NULL)
    {
        IMS_TRACE_E(0, "There is no remote target information (no contact)", 0, 0, 0);
        return IMS_FALSE;
    }

    SIPStack::SetRequestUri(pstAddrSpec, pstMessage);
    SIPStack::FreeAddrSpec(pstAddrSpec);

    // From
    pstHeader = SIPStack::CloneHeader(pstLocalURI);
    SIPStack::SetHeaderType(ISIPHeader::FROM, pstHeader);

    if (!SIPStack::SetHeader(pstHeader, pstMessage))
    {
        SIPStack::FreeHeaderEx(pstHeader);
        return IMS_FALSE;
    }

    SIPStack::FreeHeaderEx(pstHeader);

    // To
    pstHeader = SIPStack::CloneHeader(pstRemoteURI);
    SIPStack::SetHeaderType(ISIPHeader::TO, pstHeader);

    if (!SIPStack::SetHeader(pstHeader, pstMessage))
    {
        SIPStack::FreeHeaderEx(pstHeader);
        return IMS_FALSE;
    }

    SIPStack::FreeHeaderEx(pstHeader);

    // Call-ID
    pstHeader = SIPStack::DecodeHeader(ISIPHeader::CALL_ID, strCallId);

    if (!SIPStack::SetHeader(pstHeader, pstMessage))
    {
        SIPStack::FreeHeaderEx(pstHeader);
        return IMS_FALSE;
    }

    SIPStack::FreeHeaderEx(pstHeader);

    // Via : Transport Layer will set the header field

    // Route
    for (IMS_UINT32 i = 0; i < objRouteSet.GetSize(); ++i)
    {
        SipHeaderBase *pstHeader = objRouteSet.GetAt(i);

        if (pstHeader != IMS_NULL)
        {
            if (!SIPStack::InsertHeader(pstHeader, i, pstMessage))
            {
                return IMS_FALSE;
            }
        }
    }

    // Contact
    if (IsContactMandatory(SIPMessage::TYPE_REQUEST,
            objMethod, SIPStatusCode::SC_INVALID, IMS_FALSE))
    {
        pstHeader = SIPStack::CloneHeader(pstLocalTargetURI);

        if (!SIPStack::SetHeader(pstHeader, pstMessage))
        {
            SIPStack::FreeHeaderEx(pstHeader);
            return IMS_FALSE;
        }

        SIPStack::FreeHeaderEx(pstHeader);
    }

    // HEADER_REQ_SESSION-ID
    if (strSessionId.GetLength() > 0)
    {
        const AString SESSION_ID(SIPHeaderName::SESSION_ID);

        pstHeader = SIPStack::DecodeHeader(
                ISIPHeader::UNKNOWN, SESSION_ID, strSessionId);

        (void) SIPStack::SetUnknownHeader(pstHeader, SESSION_ID, pstMessage);
        SIPStack::FreeHeaderEx(pstHeader);
    }

    return IMS_TRUE;
}

/*

Remarks
 CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST

*/
PUBLIC
IMS_RESULT SIPDialogState::SetContactParameter(IN CONST AString &strParameter,
        IN IMS_SINT32 nOperation /* = 0 (0: ADD, 1: REMOVE) */)
{
    //---------------------------------------------------------------------------------------------

    if ((nOperation != 0) && (nOperation != 1))
    {
        return IMS_FAILURE;
    }

    AString strName;
    AString strValue = AString::ConstNull();
    IMS_SINT32 nCount = strParameter.SplitF(TextParser::CHAR_EQUAL, strName, strValue);

    if (nCount == 0)
    {
        return IMS_FAILURE;
    }

    if (nOperation == 0 /* ADD */)
    {
        if (strValue.IsEmpty())
        {
            strValue = AString::ConstNull();
        }

        if (!SIPStack::SetParameter(pstLocalTargetURI, strName, strValue))
        {
            return IMS_FAILURE;
        }

        if ((pLocalContactHeader != IMS_NULL)
                && (pLocalContactHeader->SetParameter(strName, strValue) != IMS_SUCCESS))
        {
            return IMS_FAILURE;
        }
    }
    else
    {
        SIPStack::RemoveParameter(strName, pstLocalTargetURI);

        if (pLocalContactHeader != IMS_NULL)
        {
            pLocalContactHeader->RemoveParameter(strName);
        }
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 SIPDialogState::UpdateDialogDetails(IN CONST SIPMessageInfo &objMInfo,
        IN IMS_SINT32 nUsageState, IN IMS_SINT32 nAction, IN IMS_SINT32 nTrigger)
{

    //---------------------------------------------------------------------------------------------

    // If the dialog's state is already in TERMINATED state, skip the below procedures ...
    if (nState == SIPDState::STATE_TERMINATED)
    {
        IMS_TRACE_D("INVALID STATE : Dialog (%s) is already in TERMINATED state",
                SIPDebug::GetCharA1(strCallId.GetStr(), 8, '@'), 0, 0);
        return SIPPrivate::MESSAGE_VALID;
    }

    SipMessage *pstMessage = objMInfo.GetMessage();

    if (objMInfo.IsOutgoingMessage())
    {
        if (!SIPStack::IsRequestMessage(pstMessage))
        {
            if (GetState() == SIPDState::STATE_INIT)
            {
                // Update To-Tag when sending the response to the incoming SIP request
                if (!UpdateComponents(objMInfo))
                {
                    return SIPPrivate::MESSAGE_FAILED;
                }

                // HEADER_REQ_SESSION-ID
                if (SIPFeatures::IsHeaderSessionIdRequired(objMInfo.GetSlotId()))
                {
                    UpdateSessionId(objMInfo);
                }
            }
        }
        else
        {
            if (GetState() == SIPDState::STATE_INIT)
            {
                // Update the local command sequence number
                nLocalCSeq = SIPStack::GetCSeqNumber(pstMessage);

                // HEADER_REQ_SESSION-ID
                if (SIPFeatures::IsHeaderSessionIdRequired(objMInfo.GetSlotId()))
                {
                    UpdateSessionId(objMInfo);
                }
            }
        }

        if (GetState() != SIPDState::STATE_CONFIRMED)
        {
            UpdateFromChangeOption(objMInfo);
        }
    }
    else
    {
        IMS_BOOL bIsINVITEConfirmed = IMS_FALSE;

        // Update the dialog identifiers if it does not already set
        if (!SIPStack::IsRequestMessage(pstMessage))
        {
            // If this is a response, then update only the To-Tag in the dialog state
            // if it has not been set as yet.
            // The From, Call-ID and other fields have already been set at the time
            // of sending out the request.
            // NOTE: The to-tag updates should only be done for a >= 2xx response.
            // Only RPR's with a different tag cause formation of different call legs.
            // As we don't support multiple call legs, for now we will update the to-tag
            // for any incoming response.
            SipHeaderBase *pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::TO);
            AString strMsgTag = SIPStack::GetParameter(pstHeader, SIP::STR_TAG);
            SIPStack::FreeHeaderEx(pstHeader);

            AString strDialogTag = SIPStack::GetParameter(pstRemoteURI, SIP::STR_TAG);

            if ((strDialogTag.GetLength() == 0) && (strMsgTag.GetLength() > 0))
            {
                if (!SIPStack::SetParameter(pstRemoteURI, SIP::STR_TAG, strMsgTag))
                    return SIPPrivate::MESSAGE_FAILED;
            }

            // The route set for the dialog MUST be re-computed based on the 2xx response
            IMS_SINT32 nStatusCode = SIPStack::GetStatusCode(pstMessage);

            if (objMInfo.GetMethod().Equals(SIPMethod::INVITE)
                    && (SIPStatusCode::IsFinalSuccess(nStatusCode)))
            {
                bIsINVITEConfirmed = IMS_TRUE;
            }
        }
        else
        {
            // Validate Cseq:
            // It will be handled in the HandleRequest(...) in SIPServerTransactionState
            /*
            IMS_SINT32 nValidity = ValidateRemoteCSeq(pstMessage);

            if (nValidity != SIPPrivate::MESSAGE_VALID)
            {
                IMS_TRACE_E(0, "Validating CSeq failed", 0, 0, 0);
                return nValidity;
            }
            */

            nRemoteCSeq = SIPStack::GetCSeqNumber(pstMessage);

            if (objMInfo.GetMethod().Equals(SIPMethod::INVITE))
            {
                nRemoteCSeqForINVITE = nRemoteCSeq;
            }

            // It is a request that has been received. If the From header has a tag, and
            //  a) If the original call was from us and the to-tag in the dialog state is NULL,
            //    then we need to update it with the received tag.
            //  b) If the original call was from the other end,
            //    and the From tag in the dialog state is NULL,
            //    then we need to update it with the received tag.
            SipHeaderBase *pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::FROM);
            AString strMsgTag = SIPStack::GetParameter(pstHeader, SIP::STR_TAG);
            SIPStack::FreeHeaderEx(pstHeader);

            if (strMsgTag.GetLength() > 0)
            {
                AString strDialogTag = SIPStack::GetParameter(pstRemoteURI, SIP::STR_TAG);

                if (strDialogTag.GetLength() == 0)
                {
                    if (!SIPStack::SetParameter(pstRemoteURI, SIP::STR_TAG, strMsgTag))
                        return SIPPrivate::MESSAGE_FAILED;
                }
            }

            if (objMInfo.GetMethod().Equals(SIPMethod::INVITE))
            {
                pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::VIA);
                strRemoteViaBranch = SIPStack::GetParameter(pstHeader, AString(SIP::STR_BRANCH));
                SIPStack::FreeHeaderEx(pstHeader);
            }
        }

        if (GetState() != SIPDState::STATE_CONFIRMED)
        {
            // HEADER_REQ_SESSION-ID
            if (SIPFeatures::IsHeaderSessionIdRequired(objMInfo.GetSlotId()))
            {
                UpdateSessionId(objMInfo);
            }

            UpdateFromChangeOption(objMInfo);

            if (objRouteSet.GetSize() == 0)
            {
                if (!CreateRouteSet(objMInfo))
                    return SIPPrivate::MESSAGE_FAILED;
            }
            else
            {
                // Route headers already exist in the route set.
                // Update only the contact header if required.

                // The route set for the dialog MUST be re-computed based on the 2xx response
                if (bIsINVITEConfirmed)
                {
                    bPreloadedSet = IMS_FALSE;

                    ClearRouteSet();

                    if (!CreateRouteSet(objMInfo))
                        return SIPPrivate::MESSAGE_FAILED;
                }
                else
                {
                    if (!UpdateRouteSet(objMInfo))
                        return SIPPrivate::MESSAGE_FAILED;
                }
            }
        }
    }

    // Update the Contact information according to the mode
    UpdateContact(objMInfo);

    // Update a dialog state
    UpdateState(nUsageState, nAction, nTrigger);

    // Update "remote-URI" if changed
    UpdateRemoteURI(objMInfo);

    return SIPPrivate::MESSAGE_VALID;
}

/*

Remarks

*/
PUBLIC
void SIPDialogState::UpdateLocalCSeq(IN IMS_UINT32 nCSeq)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Dialog (%s) - CSeq: %d >> %d",
            SIPDebug::GetCharA1(strCallId.GetStr(), 8, '@'), nLocalCSeq, nCSeq);

    nLocalCSeq = nCSeq;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 SIPDialogState::ValidateRemoteCSeq(IN SipMessage *pstMessage,
        IN IMS_SINT32 nPrevStatusCode/* = 0*/)
{
    IMS_UINT32 nSeqNum;
    SIPMethod objMethod = SIPStack::GetMethod(pstMessage);

    //---------------------------------------------------------------------------------------------

    nSeqNum = SIPStack::GetCSeqNumber(pstMessage);

    if (nSeqNum == SIPPrivate::INVALID_SEQ_NUM)
    {
        return SIPPrivate::MESSAGE_FAILED;
    }

    // If a retransmitted ACK of lower CSeq appears after a fresh INVITE of higher CSeq
    // has been received (for example, after the first INVITE was rejected with a 401/407),
    // it can be dropped.

    // For an ACK, if the CSeq is lower than the remote CSeq, it can be dropped.
    // This may happen if the ACK to a 401 appears after a fresh INVITE
    // with higher CSeq has been received by the F/W.
    if (objMethod.Equals(SIPMethod::ACK))
    {
        if (nRemoteCSeqForINVITE != nSeqNum)
        {
            // ACK request of incoming re-INVITE can be received
            // after new re-INVITE request is received.
            if (!SIPStatusCode::IsFinalSuccess(nPrevStatusCode))
            {
                IMS_TRACE_E(0, "OUT OF SEQUENCE :: ACK (%d, %d)",
                        nRemoteCSeqForINVITE, nSeqNum, 0);
                return SIPPrivate::MESSAGE_DISCARDED;
            }
        }

        return SIPPrivate::MESSAGE_VALID;
    }

    // Check if the CSeq is higher.
    // Note that for ACK and CANCEL, the CSeq will be same as that in initial INVITE.
    if (objMethod.Equals(SIPMethod::CANCEL))
    {
        if (nRemoteCSeqForINVITE == nSeqNum)
        {
            return SIPPrivate::MESSAGE_VALID;
        }
        else if (nRemoteCSeqForINVITE < nSeqNum)
        {
            SIPPrivate::SetLastError(SIPError::CSEQ_VALUE_MISMATCHED);
            return SIPPrivate::MESSAGE_INVALID_400;
        }
        else
        {
            SIPPrivate::SetLastError(SIPError::REQUEST_OUT_OF_ORDER);
            return SIPPrivate::MESSAGE_INVALID_500;
        }
    }

    // Checks for PRACK.
    if (objMethod.Equals(SIPMethod::PRACK))
    {
        // Application will handle the procedure for PRACK.
    }

    //3 Loop detection ????

    // a) If CSeq is higher, it should be responded with a 400 Bad Request.
    // b) If CSeq is the same, it is a retransmission, so it can be dropped.
    // c) If CSeq is the lower, it should be responded with a 500 response.
    if (nRemoteCSeq > nSeqNum)
    {
        SIPPrivate::SetLastError(SIPError::REQUEST_OUT_OF_ORDER);
        return SIPPrivate::MESSAGE_INVALID_500;
    }
    else if (nRemoteCSeq == nSeqNum)
    {
        // It needs to be checked if the request is looped or not.
        // CheckRequestLoop(pstMessage);
        SIPPrivate::SetLastError(SIPError::LOOP_DETECTED);
        return SIPPrivate::MESSAGE_INVALID_400;
    }

    return SIPPrivate::MESSAGE_VALID;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPDialogState::AddDialogUsage(IN SIPDialogEx *pDialogEx)
{
    //---------------------------------------------------------------------------------------------

    if (pSharedState == IMS_NULL)
        return IMS_FALSE;

    return pSharedState->AddDialog(pDialogEx);
}

/*

Remarks

*/
PUBLIC
void SIPDialogState::RemoveDialogUsage(IN SIPDialogEx *pDialogEx)
{
    //---------------------------------------------------------------------------------------------

    if (pSharedState == IMS_NULL)
        return;

    pSharedState->RemoveDialog(pDialogEx);
}

/*

Remarks

*/
PUBLIC
SIPDialogEx* SIPDialogState::GetDialogUsage(IN CONST SIPMessageInfo &objMInfo)
{
    //---------------------------------------------------------------------------------------------

    if (pSharedState == IMS_NULL)
        return IMS_NULL;

    return pSharedState->GetDialog(objMInfo);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPDialogState::HasMultipleDialogUsages() const
{
    //---------------------------------------------------------------------------------------------

    if (pSharedState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pSharedState->HasMultipleDialogUsages();
}

/*

Remarks

*/
PUBLIC GLOBAL
IMS_BOOL SIPDialogState::IsContactMandatory(IN IMS_SINT32 nMsgType, IN CONST SIPMethod &objMethod,
        IN IMS_SINT32 nStatusCode, IN IMS_BOOL bContactInAll1xxRequired)
{
    //---------------------------------------------------------------------------------------------

    if (nMsgType == SIPMessage::TYPE_REQUEST)
    {
        if (objMethod.Equals(SIPMethod::INVITE)
                || objMethod.Equals(SIPMethod::SUBSCRIBE)
                || objMethod.Equals(SIPMethod::NOTIFY)
                || objMethod.Equals(SIPMethod::REFER)
                || objMethod.Equals(SIPMethod::UPDATE))
        {
            return IMS_TRUE;
        }
        // REGISTER, OPTIONS, and PUBLISH is added by the JSR 281 requirements.
        else if (objMethod.Equals(SIPMethod::REGISTER)
                || objMethod.Equals(SIPMethod::OPTIONS)
                || objMethod.Equals(SIPMethod::PUBLISH))
        {
            return IMS_TRUE;
        }
    }
    else
    {
        if ((nStatusCode >= SIPStatusCode::SC_200)
                && (nStatusCode < SIPStatusCode::SC_300))
        {
            if (objMethod.Equals(SIPMethod::INVITE)
                    || objMethod.Equals(SIPMethod::SUBSCRIBE)
                    || objMethod.Equals(SIPMethod::REFER)
                    || objMethod.Equals(SIPMethod::UPDATE))
            {
                return IMS_TRUE;
            }
            // OPTIONS & NOTIFY is added by the JSR 281 requirements.
            else if (objMethod.Equals(SIPMethod::OPTIONS)
                    || objMethod.Equals(SIPMethod::NOTIFY))
            {
                return IMS_TRUE;
            }
        }
        else if ((nStatusCode >= SIPStatusCode::SC_300)
                && (nStatusCode < SIPStatusCode::SC_400))
        {
            if (objMethod.Equals(SIPMethod::SUBSCRIBE)
                    || objMethod.Equals(SIPMethod::NOTIFY))
            {
                return IMS_TRUE;
            }
        }
        // PATCH_FOR_CONTACT_HEADER_IN_ALL_1XX
        else if (SIPStatusCode::IsProvisional(nStatusCode))
        {
            if (objMethod.Equals(SIPMethod::INVITE))
            {
                if (bContactInAll1xxRequired)
                {
                    return IMS_TRUE;
                }
                else if (nStatusCode >= SIPStatusCode::SC_180)
                {
                    // 3GPP Profile: 18x / 19x
                    return IMS_TRUE;
                }
            }
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE
void SIPDialogState::ClearRouteSet()
{
    //---------------------------------------------------------------------------------------------

    if (objRouteSet.IsEmpty())
        return;

    for (IMS_UINT32 i = 0; i < objRouteSet.GetSize(); ++i)
    {
        SipHeaderBase *pstHeader = objRouteSet.GetAt(i);

        SIPStack::FreeHeaderEx(pstHeader);
    }

    objRouteSet.Clear();
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPDialogState::CreateRouteSet(IN CONST SIPMessageInfo &objMInfo)
{
    SipMessage *pstMessage = objMInfo.GetMessage();

    //---------------------------------------------------------------------------------------------

    if (SIPDialogState::IsTargetRefreshMessage(pstMessage))
    {
        IMS_SINT32 nHCount = SIPStack::GetHeaderCount(pstMessage, ISIPHeader::RECORD_ROUTE);

        if (nHCount != 0)
        {
            SipHeaderBase *pstRRHdr;
            SipHeaderBase *pstHeader;

            IMS_SINT32 nIndex;

            // Reverse the record-routes into the route list only
            // if a response message is received (UAC end).
            // Else, copy the record-routes into routes as they are (UAS end).
            if (!SIPStack::IsRequestMessage(pstMessage))
            {
                IMS_SINT32 nStatusCode = SIPStack::GetStatusCode(pstMessage);

                if (!SIPStack::IsMessageRPR(pstMessage)
                        && SIPStatusCode::IsProvisional(nStatusCode))
                {
                    bPreloadedSet = IMS_TRUE;
                }

                for (nIndex = nHCount - 1; nIndex >= 0; nIndex--)
                {
                    pstRRHdr = SIPStack::GetHeader(pstMessage, ISIPHeader::RECORD_ROUTE, nIndex);

                    SIPStack::SetHeaderType(ISIPHeader::ROUTE, pstRRHdr);

                    pstHeader = SIPStack::CloneHeader(pstRRHdr);

                    SIPStack::FreeHeaderEx(pstRRHdr);

                    if (!objRouteSet.InsertAt(pstHeader, (nHCount - 1) - nIndex))
                    {
                        SIPStack::FreeHeaderEx(pstHeader);
                        return IMS_FALSE;
                    }
                }
            }
            else
            {
                for (nIndex = 0; nIndex < nHCount; nIndex++)
                {
                    pstRRHdr = SIPStack::GetHeader(pstMessage, ISIPHeader::RECORD_ROUTE, nIndex);

                    SIPStack::SetHeaderType(ISIPHeader::ROUTE, pstRRHdr);

                    pstHeader = SIPStack::CloneHeader(pstRRHdr);

                    SIPStack::FreeHeaderEx(pstRRHdr);

                    if (!objRouteSet.Append(pstHeader))
                    {
                        SIPStack::FreeHeaderEx(pstHeader);
                        return IMS_FALSE;
                    }
                }
            }
        }

        //3 Contact or From or Request-URI need to be extracted ???
        if (!UpdateContact(objMInfo))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPDialogState::UpdateComponents(IN CONST SIPMessageInfo &objMInfo)
{
    SipHeaderBase *pstHeader;
    SipHeaderBase *pstTmpHeader;
    SipMessage *pstMessage = objMInfo.GetMessage();

    IMS_SINT32 nHType;
    AString strTmpVal;

    //---------------------------------------------------------------------------------------------

    // Call Id
    pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::CALL_ID);
    SIPStack::EncodeHeaderBody(pstHeader, IMS_FALSE, strTmpVal);

    if ((strCallId.GetLength() == 0) || (!strCallId.Equals(strTmpVal)))
    {
        strCallId = strTmpVal;
    }

    SIPStack::FreeHeaderEx(pstHeader);

    // Local URI & Tag
    SIPStack::FreeHeader(pstLocalURI);

    if (SIPStack::IsRequestMessage(pstMessage))
    {
        if (objMInfo.IsOutgoingMessage())
            nHType = ISIPHeader::FROM;
        else
            nHType = ISIPHeader::TO;
    }
    else
    {
        if (objMInfo.IsOutgoingMessage())
            nHType = ISIPHeader::TO;
        else
            nHType = ISIPHeader::FROM;
    }

    pstHeader = SIPStack::GetHeader(pstMessage, nHType);

#ifdef __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__
    pstTmpHeader = pstHeader;
    pstHeader = IMS_NULL;
#else
    pstTmpHeader = SIPStack::CloneHeader(pstHeader);
#endif

    if (pstTmpHeader != IMS_NULL)
    {
        pstLocalURI = SIPStack::CopyHeader(pstLocalURI, pstTmpHeader);
        SIPStack::FreeHeaderEx(pstTmpHeader);
    }

    SIPStack::FreeHeaderEx(pstHeader);

    // Remote URI & Tag
    SIPStack::FreeHeader(pstRemoteURI);

    if (SIPStack::IsRequestMessage(pstMessage))
    {
        if (objMInfo.IsOutgoingMessage())
            nHType = ISIPHeader::TO;
        else
            nHType = ISIPHeader::FROM;
    }
    else
    {
        if (objMInfo.IsOutgoingMessage())
            nHType = ISIPHeader::FROM;
        else
            nHType = ISIPHeader::TO;
    }

    pstHeader = SIPStack::GetHeader(pstMessage, nHType);

#ifdef __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__
    pstTmpHeader = pstHeader;
    pstHeader = IMS_NULL;
#else
    pstTmpHeader = SIPStack::CloneHeader(pstHeader);
#endif

    if (pstTmpHeader != IMS_NULL)
    {
        pstRemoteURI = SIPStack::CopyHeader(pstRemoteURI, pstTmpHeader);
        SIPStack::FreeHeaderEx(pstTmpHeader);
    }

    SIPStack::FreeHeaderEx(pstHeader);

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPDialogState::UpdateContact(IN CONST SIPMessageInfo &objMInfo)
{
    SipMessage *pstMessage = objMInfo.GetMessage();

    //---------------------------------------------------------------------------------------------

    if (!IsTargetRefreshMessage(pstMessage))
    {
        // REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
        if (objMInfo.IsOutgoingMessage()
                && !SIPStack::IsRequestMessage(pstMessage))
        {
            IMS_SINT32 nStatusCode = SIPStack::GetStatusCode(pstMessage);

            if (nStatusCode != SIPStatusCode::SC_100)
            {
                RemovePendingRemoteTarget(objMInfo);
            }
        }
        return IMS_TRUE;
    }

    IMS_SINT32 nHCount = SIPStack::GetHeaderCount(pstMessage, ISIPHeader::CONTACT_NORMAL);

    if (nHCount > 0)
    {
        if (objMInfo.IsOutgoingMessage())
        {
            SIPStack::FreeHeader(pstLocalTargetURI);

            SipHeaderBase *pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::CONTACT_NORMAL);

            if (pstHeader != IMS_NULL)
            {
                pstLocalTargetURI = SIPStack::CopyHeader(pstLocalTargetURI, pstHeader);
                SIPStack::FreeHeaderEx(pstHeader);
            }

            if (pLocalContactHeader != IMS_NULL)
            {
                delete pLocalContactHeader;
                pLocalContactHeader = IMS_NULL;
            }

            if (SIPStack::IsValidHeader(pstLocalTargetURI))
            {
                pLocalContactHeader = new SIPHeader(pstLocalTargetURI);
            }

            // REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
            if (!SIPStack::IsRequestMessage(pstMessage))
            {
                UpdateAndRemovePendingRemoteTarget(objMInfo);
            }
        }
        else
        {
            // REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
            if ((nState != SIPDState::STATE_INIT) && SIPStack::IsRequestMessage(pstMessage))
            {
                // Store the remote target and it will be updated after sending 2xx response.
                AddPendingRemoteTarget(objMInfo);
            }
            else
            {
                SIPStack::FreeHeader(pstRemoteTargetURI);

                SipHeaderBase *pstHeader = SIPStack::GetHeader(
                        pstMessage, ISIPHeader::CONTACT_NORMAL);

                if (pstHeader != IMS_NULL)
                {
                    pstRemoteTargetURI = SIPStack::CopyHeader(pstRemoteTargetURI, pstHeader);
                    SIPStack::FreeHeaderEx(pstHeader);
                }
            }
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPDialogState::UpdateRemoteURI(IN CONST SIPMessageInfo &objMInfo)
{
    SipMessage *pstMessage = objMInfo.GetMessage();
    IMS_BOOL bFromChangeable = IMS_FALSE;

    //---------------------------------------------------------------------------------------------

    if ((GetState() != SIPDState::STATE_INIT)
            && IsFromChangeCapable()
            && (objMInfo.GetMethod().Equals(SIPMethod::INVITE)
                || objMInfo.GetMethod().Equals(SIPMethod::UPDATE))
            && !SIPStack::IsRequestMessage(pstMessage))
    {
        IMS_SINT32 nStatusCode = SIPStack::GetStatusCode(pstMessage);

        if (SIPStatusCode::IsFinalSuccess(nStatusCode))
        {
            bFromChangeable = IMS_TRUE;
        }
    }

    if (!bFromChangeable)
    {
        return IMS_FALSE;
    }

    SipHeaderBase *pstTmpHeader;
    IMS_SINT32 nHType;

    if (SIPStack::IsRequestMessage(pstMessage))
    {
        if (objMInfo.IsOutgoingMessage())
            nHType = ISIPHeader::TO;
        else
            nHType = ISIPHeader::FROM;
    }
    else
    {
        if (objMInfo.IsOutgoingMessage())
            nHType = ISIPHeader::FROM;
        else
            nHType = ISIPHeader::TO;
    }

    // Remote URI & Tag
    SIPStack::FreeHeader(pstRemoteURI);

    SipHeaderBase *pstHeader = SIPStack::GetHeader(pstMessage, nHType);

#ifdef __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__
    pstTmpHeader = pstHeader;
    pstHeader = IMS_NULL;
#else
    pstTmpHeader = SIPStack::CloneHeader(pstHeader);
#endif

    if (pstTmpHeader != IMS_NULL)
    {
        pstRemoteURI = SIPStack::CopyHeader(pstRemoteURI, pstTmpHeader);
        SIPStack::FreeHeaderEx(pstTmpHeader);
    }

    SIPStack::FreeHeaderEx(pstHeader);

    IMS_TRACE_D("DialogState :: from-change is done", 0, 0, 0);

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPDialogState::UpdateRouteSet(IN CONST SIPMessageInfo &objMInfo)
{
    SipMessage *pstMessage = objMInfo.GetMessage();

    //---------------------------------------------------------------------------------------------

    if (SIPDialogState::IsTargetRefreshMessage(pstMessage))
    {
        if (bPreloadedSet)
        {
            // This indicates that the existing route set is only the preloaded one.
            // So, delete the preloaded one and form the fresh route set.
            ClearRouteSet();

            if (!CreateRouteSet(objMInfo))
                return IMS_FALSE;

            bPreloadedSet = IMS_FALSE;
        }
        else
        {
            // This is an established route set.
            // Only last entry which indicates the contact address can be changed.
            if (!UpdateContact(objMInfo))
                return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*

Remarks
 HEADER_REQ_SESSION-ID
*/
PRIVATE
void SIPDialogState::UpdateSessionId(IN CONST SIPMessageInfo &objMInfo)
{
    //---------------------------------------------------------------------------------------------

    if (strSessionId.GetLength() > 0)
    {
        // Session-ID is already set by the previous request or response...
        return;
    }

    SipHeaderBase *pstHeader = SIPStack::GetUnknownHeader(
                objMInfo.GetMessage(), SIPHeaderName::SESSION_ID);

    if (SIPStack::IsValidHeader(pstHeader))
    {
        strSessionId = SIPStack::GetUnknownHeaderBody(pstHeader);
    }

    SIPStack::FreeHeaderEx(pstHeader);
}

/*

Remarks

*/
PRIVATE
void SIPDialogState::UpdateState(IN IMS_SINT32 nUsageState,
        IN IMS_SINT32 nAction, IN IMS_SINT32 nTrigger)
{
    IMS_SINT32 nNextState = SIPDState::STATE_MAX;

    //---------------------------------------------------------------------------------------------

    //3 TODO
    (void) nTrigger;

    switch (nAction)
    {
    case SIPDState::ACTION_TRANSIT_STATE:
        // If the shared state has a multiple dialog usages, do not transit the state.
        nNextState = nUsageState;

        if (pSharedState->IsShared() && (nUsageState == SIPDState::STATE_TERMINATED))
        {
            // Do not transit the state...
            nNextState = SIPDState::STATE_MAX;
        }
        break;

    case SIPDState::ACTION_DESTROY_USAGE:
        // If the shared state has a multiple dialog usages, do not transit the state.
        if (!pSharedState->IsShared())
        {
            nNextState = SIPDState::STATE_TERMINATED;
        }
        break;

    case SIPDState::ACTION_DESTROY_DIALOG:
        nNextState = SIPDState::STATE_TERMINATED;
        break;

    case SIPDState::ACTION_IGNORE:
    default:
        return;
    }

    if ((nNextState >= SIPDState::STATE_INIT)
            && (nNextState < SIPDState::STATE_MAX))
    {
        static const IMS_CHAR* STATE[SIPDState::STATE_MAX] =
        {
            "INITIALIZED",
            "TERMINATED",
            "EARLY",
            "CONFIRMED"
        };

        IMS_TRACE_I("_____ DIALOG STATE : (%s) >>> (%s) _____",
                STATE[nState], STATE[nNextState], 0);

        nState = nNextState;
    }
}

/*

Remarks
 REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
*/
PRIVATE
void SIPDialogState::AddPendingRemoteTarget(IN CONST SIPMessageInfo &objMInfo)
{
    SipMessage *pstMessage = objMInfo.GetMessage();

    //---------------------------------------------------------------------------------------------

    AString strKey = SIPStack::GetViaBranchParameter(pstMessage);

    if (strKey.GetLength() == 0)
    {
        // no-op
        return;
    }

    // Checks if the pending remote target is already present or not
    for (IMS_UINT32 i = 0; i < objPendingRemoteTargets.GetSize(); ++i)
    {
        PendingRemoteTarget *pRemoteTarget = objPendingRemoteTargets.GetAt(i);

        if (pRemoteTarget == IMS_NULL)
        {
            continue;
        }

        if (strKey.Equals(pRemoteTarget->strKey))
        {
            IMS_TRACE_D("PendingRemoteTarget(add) :: key(dup)=%s, count=%d",
                    SIPDebug::GetCharA1(strKey.GetStr(), 8, '@'),
                    objPendingRemoteTargets.GetSize(), 0);
            return;
        }
    }

    SipHeaderBase *pstContact = IMS_NULL;
    SipHeaderBase *pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::CONTACT_NORMAL);

    if (SIPStack::IsValidHeader(pstHeader))
    {
        pstContact = SIPStack::CloneHeader(pstHeader);
    }

    SIPStack::FreeHeaderEx(pstHeader);

    if (pstContact == IMS_NULL)
    {
        // no-op
        return;
    }

    objPendingRemoteTargets.Append(new PendingRemoteTarget(strKey, pstContact));

    IMS_TRACE_D("PendingRemoteTarget(add) :: key=%s, count=%d",
            SIPDebug::GetCharA1(strKey.GetStr(), 8, '@'),
            objPendingRemoteTargets.GetSize(), 0);
}

/*

Remarks
 REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
*/
PRIVATE
void SIPDialogState::RemoveAllPendingRemoteTargets()
{
    //---------------------------------------------------------------------------------------------

    if (!objPendingRemoteTargets.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objPendingRemoteTargets.GetSize(); ++i)
        {
            PendingRemoteTarget *pRemoteTarget = objPendingRemoteTargets.GetAt(i);

            if (pRemoteTarget != IMS_NULL)
            {
                delete pRemoteTarget;
            }
        }

        objPendingRemoteTargets.Clear();
    }
}

/*

Remarks
 REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
*/
PRIVATE
void SIPDialogState::RemovePendingRemoteTarget(IN CONST SIPMessageInfo &objMInfo)
{
    //---------------------------------------------------------------------------------------------

    if (objPendingRemoteTargets.IsEmpty())
    {
        // no-op
        return;
    }

    AString strKey = SIPStack::GetViaBranchParameter(objMInfo.GetMessage());

    if (strKey.GetLength() == 0)
    {
        // no-op
        return;
    }

    for (IMS_UINT32 i = 0; i < objPendingRemoteTargets.GetSize(); ++i)
    {
        PendingRemoteTarget *pRemoteTarget = objPendingRemoteTargets.GetAt(i);

        if (pRemoteTarget == IMS_NULL)
        {
            continue;
        }

        if (strKey.Equals(pRemoteTarget->strKey))
        {
            objPendingRemoteTargets.RemoveAt(i);
            delete pRemoteTarget;

            IMS_TRACE_D("PendingRemoteTarget(remove) :: key=%s, count=%d",
                    SIPDebug::GetCharA1(strKey.GetStr(), 8, '@'),
                    objPendingRemoteTargets.GetSize(), 0);
            break;
        }
    }
}

/*

Remarks
 REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
*/
PRIVATE
void SIPDialogState::UpdateAndRemovePendingRemoteTarget(IN CONST SIPMessageInfo &objMInfo)
{
    //---------------------------------------------------------------------------------------------

    if (objPendingRemoteTargets.IsEmpty())
    {
        // no-op
        return;
    }

    AString strKey = SIPStack::GetViaBranchParameter(objMInfo.GetMessage());

    if (strKey.GetLength() == 0)
    {
        // no-op
        return;
    }

    PendingRemoteTarget *pRemoteTarget = IMS_NULL;
    IMS_UINT32 nMatchedIndex = 0;

    for (IMS_UINT32 i = 0; i < objPendingRemoteTargets.GetSize(); ++i)
    {
        pRemoteTarget = objPendingRemoteTargets.GetAt(i);

        if (pRemoteTarget == IMS_NULL)
        {
            continue;
        }

        if (strKey.Equals(pRemoteTarget->strKey))
        {
            nMatchedIndex = i;
            break;
        }

        pRemoteTarget = IMS_NULL;
    }

    if ((pRemoteTarget != IMS_NULL) && (pRemoteTarget->pstHeader != IMS_NULL))
    {
        SIPStack::FreeHeaderEx(pstRemoteTargetURI);
        pstRemoteTargetURI = SIPStack::CopyHeader(pRemoteTarget->pstHeader);
    }

    if (pRemoteTarget != IMS_NULL)
    {
        if (nMatchedIndex == (objPendingRemoteTargets.GetSize() - 1))
        {
            RemoveAllPendingRemoteTargets();
        }
        else
        {
            IMS_SINT32 nCount = static_cast<IMS_SINT32>(nMatchedIndex + 1);

            while (nCount > 0)
            {
                pRemoteTarget = objPendingRemoteTargets.GetAt(0);
                objPendingRemoteTargets.RemoveAt(0);

                if (pRemoteTarget != IMS_NULL)
                {
                    delete pRemoteTarget;
                }

                --nCount;
            }
        }

        IMS_TRACE_D("PendingRemoteTarget(update&remove) :: key=%s, index=%d, count=%d",
                SIPDebug::GetCharA1(strKey.GetStr(), 8, '@'),
                nMatchedIndex, objPendingRemoteTargets.GetSize());
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPDialogState::IsFromChangeCapable() const
{
    //---------------------------------------------------------------------------------------------

    return ((nFromChangeOption & FROM_CHANGE_CAPABLE) == FROM_CHANGE_CAPABLE);
}

/*

Remarks

*/
PRIVATE
void SIPDialogState::ClearFromChangeOption(IN IMS_SINT32 nOption)
{
    //---------------------------------------------------------------------------------------------

    nFromChangeOption &= (~nOption);
}

/*

Remarks

*/
PRIVATE
void SIPDialogState::SetFromChangeOption(IN IMS_SINT32 nOption)
{
    IMS_BOOL bChanged = ((nFromChangeOption & nOption) != nOption);

    //---------------------------------------------------------------------------------------------

    nFromChangeOption |= nOption;

    if (bChanged)
    {
        IMS_TRACE_D("DialogState :: from-change=%02x", nFromChangeOption, 0, 0);
    }
}

/*

Remarks

*/
PRIVATE
void SIPDialogState::UpdateFromChangeOption(IN CONST SIPMessageInfo &objMInfo)
{
    const SIPMethod &objMethod = objMInfo.GetMethod();

    //---------------------------------------------------------------------------------------------

    if (!objMethod.Equals(SIPMethod::INVITE))
    {
        // no-op
        return;
    }

    SipMessage *pstMessage = objMInfo.GetMessage();

    if ((GetState() == SIPDState::STATE_INIT)
            && SIPStack::IsRequestMessage(pstMessage))
    {
        if (SIPStack::IsOptionSupported(pstMessage, SIP::STR_FROM_CHANGE))
        {
            SetFromChangeOption(FROM_CHANGE_ON_INVITE_REQUEST);
        }
    }
    else if ((GetState() != SIPDState::STATE_CONFIRMED)
            && !SIPStack::IsRequestMessage(pstMessage))
    {
        IMS_SINT32 nStatusCode = SIPStack::GetStatusCode(pstMessage);

        if (SIPStatusCode::IsProvisional(nStatusCode)
                || SIPStatusCode::IsFinalSuccess(nStatusCode))
        {
            if (SIPStack::IsOptionSupported(pstMessage, SIP::STR_FROM_CHANGE))
            {
                SetFromChangeOption(FROM_CHANGE_ON_INVITE_RESPONSE);
            }
        }
    }
}

/*

Remarks

*/
PRIVATE GLOBAL
IMS_SINT32 SIPDialogState::CompareHeaders(IN SipHeaderBase *pstNewH, IN SipHeaderBase *pstExistingH,
        IN IMS_BOOL bToTagLenient, IN IMS_SINT32 nForkedMessage)
{
    IMS_BOOL bToTagComparison = IMS_FALSE;
    IMS_SINT32 nComparisonResult = NOT_MATCHED;

    //---------------------------------------------------------------------------------------------

    if ((pstNewH == IMS_NULL) || (pstExistingH == IMS_NULL))
        return NOT_MATCHED;

    // Compare the tags which are sufficient for RFC 3261 compliant UAs
    AString strNewTag = SIPStack::GetParameter(pstNewH, SIP::STR_TAG);
    AString strExistingTag = SIPStack::GetParameter(pstExistingH, SIP::STR_TAG);

    if ((SIPStack::GetHeaderType(pstNewH) == ISIPHeader::TO)
            && (SIPStack::GetHeaderType(pstExistingH) == ISIPHeader::TO))
    {
        bToTagComparison = IMS_TRUE;
    }

    // New-Tag is the tag of the new key.
    // Existing-Tag is the tag of the existing key.
    // If Existing-Tag is NULL, but New-Tag is present, that is fine.
    // But, if New-Tag is NULL and Existing-Tag is not, then the remote end has gobbled the tag.
    // We return NOT_MATCHED in this case.
    if (!strNewTag.IsNULL())
    {
        if (!strExistingTag.IsNULL())
        {
            if (!strNewTag.Equals(strExistingTag))
            {
                if ((bToTagComparison) && (bToTagLenient))
                {
                    if (nForkedMessage == FORKED_INVITE)
                        nComparisonResult = MATCHED_DIFFERENT;
                    else
                        nComparisonResult = MATCHED;
                }
                else
                {
                    nComparisonResult = NOT_MATCHED;
                }
            }
            else
            {
                nComparisonResult = MATCHED;
            }
        }
        else
        {
            // Fix for CSR 1-1316815
            // Leniency should be present only for the To-To pair.
            // For all other pairs (i.e. From-From, From-To, To-From),
            // it should map to NOT_MATCHED.
            if (bToTagComparison)
                nComparisonResult = MATCHED;
            else
                nComparisonResult = NOT_MATCHED;
        }
    }
    else
    {
        if (!strExistingTag.IsNULL())
        {
            // Fix for CSR 1-1316815
            // Leniency should be present only for the To-To pair and that too
            // if the bToTagLenient flag is set (i.e. to provide leniency for special cases).
            if (bToTagComparison && bToTagLenient)
                nComparisonResult = MATCHED;
            else
                nComparisonResult = NOT_MATCHED;
        }
        else
        {
            nComparisonResult = MATCHED;
        }
    }

    // The following code is added to check the two scenarios:
    // 1) Early NOTIFY i.e. a NOTIFY arriving before any other response/SUBSCRIBE.
    // In case of Early NOTIFY, the existing dialog-state would not have a tag
    // whereas the new dialog-state would have one.
    //
    // 2) A NOTIFY arriving because of forking of the SUBSCRIBE. This check has to be
    // prompted by the application and we use the "dIsCheckForkedSubs" boolean for this purpose.
    //
    // NOTE: In case of NOTIFYs from different notifiers(as a result of forked SUBSCRIBE),
    // it is most likely that the type of the remote-address in the dialog-state is the same as
    // the type of the remote-address got from the NOTIFY(i.e. its From header).
    // So, the check for Early NOTIFY will not pass & so we've to explicitly check the boolean,
    // "dIsCheckForkedSubs.
    if ((nComparisonResult == NOT_MATCHED)
            && ((nForkedMessage == FORKED_SUBSCRIBE)
                || ((SIPStack::GetHeaderType(pstNewH) == ISIPHeader::FROM)
                    && (SIPStack::GetHeaderType(pstExistingH) == ISIPHeader::TO))))
    {
        if (!strNewTag.IsNULL())
        {
            if (!strExistingTag.IsNULL() && (nForkedMessage == FORKED_SUBSCRIBE))
            {
                // If To-Tag of the existing dialog-state doesn't match that of the existing
                // dialog-state, then it could be either: NOTIFY received for a forked SUBSCRIBE
                // (OR) NOTIFY outside any subscription. In either case, we will return
                // the partially matched dialog-state and let the application find out the rest.
                nComparisonResult = MATCHED_FORKED_SUBSCRIBE;

                // Check the case,
                //        Existing Header: From (local-tag), New Header: To (local-tag)
                if ((SIPStack::GetHeaderType(pstNewH) == ISIPHeader::TO)
                        && (SIPStack::GetHeaderType(pstExistingH) == ISIPHeader::FROM))
                {
                    if (!strNewTag.Equals(strExistingTag))
                    {
                        nComparisonResult = NOT_MATCHED;
                    }
                }
            }
            else if (strExistingTag.IsNULL())
            {
                // If To-tag is not present in the existing dialog-state,
                // then it could be a case of either:
                // Early NOTIFY (OR) NOTIFY received for a forked SUBSCRIBE.
                // We will treat this as an early NOTIFY.
                // Again, return the partially matching dialog-state,
                // application should figure out the rest.
                nComparisonResult = MATCHED_EARLY_NOTIFY;
            }
        }
    }

    return nComparisonResult;
}

/*

Remarks

*/
PRIVATE GLOBAL
IMS_BOOL SIPDialogState::IsTargetRefreshMessage(IN SipMessage *pstMessage)
{
    SIPMethod objMethod = SIPStack::GetMethod(pstMessage);

    //---------------------------------------------------------------------------------------------

    if (objMethod.Equals(SIPMethod::INVALID))
        return IMS_FALSE;

    // Check if the method is a remote target refreshable one or not
    if (!objMethod.Equals(SIPMethod::INVITE)
            && !objMethod.Equals(SIPMethod::UPDATE)
            && !objMethod.Equals(SIPMethod::SUBSCRIBE)
            && !objMethod.Equals(SIPMethod::NOTIFY)
            && !objMethod.Equals(SIPMethod::REFER))
    {
        return IMS_FALSE;
    }

    if (!SIPStack::IsRequestMessage(pstMessage))
    {
        IMS_SINT32 nStatusCode = SIPStack::GetStatusCode(pstMessage);

        if (!SIPStatusCode::IsProvisional(nStatusCode)
                && !SIPStatusCode::IsFinalSuccess(nStatusCode))
        {
            return IMS_FALSE;
        }

        if (SIPStatusCode::IsProvisional(nStatusCode))
        {
            // Check if the message is a reliable provisional response or not
            if (SIPStack::IsMessageRPR(pstMessage) != IMS_TRUE)
            {
                // Check if the message has a To-Tag or not
                SipHeaderBase *pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::TO);

                // Get To-Tag parameter from To header
                AString strToTag = SIPStack::GetParameter(pstHeader, SIP::STR_TAG);
                SIPStack::FreeHeaderEx(pstHeader);

                if (strToTag.GetLength() == 0)
                {
                    return IMS_FALSE;
                }
            }
        }
    }

    return IMS_TRUE;
}
