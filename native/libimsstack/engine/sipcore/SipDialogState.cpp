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

#include "ISipHeader.h"
#include "SipDState.h"
#include "SipDebug.h"
#include "SipDialogSharedState.h"
#include "SipDialogState.h"
#include "SipError.h"
#include "SipFeatures.h"
#include "SipHeader.h"
#include "SipMessage.h"
#include "SipMessageInfo.h"
#include "SipPrivate.h"
#include "SipStack.h"

#define __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipDialogState::PendingRemoteTarget::PendingRemoteTarget() :
        strKey(AString::ConstNull()),
        pSipHdr(IMS_NULL)
{
}

PUBLIC
SipDialogState::PendingRemoteTarget::PendingRemoteTarget(IN const PendingRemoteTarget& other) :
        strKey(other.strKey),
        pSipHdr(IMS_NULL)
{
    if (other.pSipHdr != IMS_NULL)
    {
        pSipHdr = SipStack::CloneHeader(other.pSipHdr);
    }
}

PUBLIC
SipDialogState::PendingRemoteTarget::PendingRemoteTarget(
        IN const AString& strKey_, IN SipHeaderBase* pSipHdr_) :
        strKey(strKey_),
        pSipHdr(pSipHdr_)
{
}

PUBLIC
SipDialogState::PendingRemoteTarget::~PendingRemoteTarget()
{
    if (pSipHdr != IMS_NULL)
    {
        SipStack::FreeHeaderEx(pSipHdr);
    }
}

PUBLIC
SipDialogState::SipDialogState(IN IMS_BOOL bIsCaller /*= IMS_TRUE*/) :
        RcObject(),
        m_bIsCaller(bIsCaller),
        m_pLocalUri(IMS_NULL),
        m_pRemoteUri(IMS_NULL),
        m_bSecure(IMS_FALSE),
        m_pLocalTargetUri(IMS_NULL),
        m_pRemoteTargetUri(IMS_NULL),
        m_objPendingRemoteTargets(ImsList<PendingRemoteTarget*>()),
        m_nLocalCSeq(0),
        m_nRemoteCSeq(0),
        m_nRemoteCSeqForInvite(0),
        m_nState(SipDState::STATE_INIT),
        m_bPreloadedSet(IMS_FALSE),
        m_pSharedState(IMS_NULL),
        m_pLocalContactHeader(IMS_NULL),
        m_strSessionId(AString::ConstNull()),
        m_nFromChangeOption(FROM_CHANGE_NONE)
{
    m_pLocalUri = SipStack::CreateHeader(SipStack::GetAnyHeaderType());
    m_pRemoteUri = SipStack::CreateHeader(SipStack::GetAnyHeaderType());

    // Local & Remote Contact information
    m_pLocalTargetUri = SipStack::CreateHeader(SipStack::GetAnyHeaderType());
    m_pRemoteTargetUri = SipStack::CreateHeader(SipStack::GetAnyHeaderType());
}

PUBLIC
SipDialogState::SipDialogState(IN const SipDialogState& other) :
        RcObject(other),
        m_bIsCaller(other.m_bIsCaller),
        m_pLocalUri(IMS_NULL),
        m_pRemoteUri(IMS_NULL),
        m_bSecure(other.m_bSecure),
        m_pLocalTargetUri(IMS_NULL),
        m_pRemoteTargetUri(IMS_NULL),
        m_objPendingRemoteTargets(ImsList<PendingRemoteTarget*>()),
        m_nLocalCSeq(other.m_nLocalCSeq),
        m_nRemoteCSeq(other.m_nRemoteCSeq),
        m_nRemoteCSeqForInvite(other.m_nRemoteCSeqForInvite),
        m_nState(other.m_nState),
        m_bPreloadedSet(other.m_bPreloadedSet),
        m_pSharedState(IMS_NULL),
        m_pLocalContactHeader(IMS_NULL),
        m_strSessionId(other.m_strSessionId),
        m_nFromChangeOption(other.m_nFromChangeOption)
{
    // NOTE: Actually, this class is used as a reference count object so if the class usage is
    // changed, then we need to handle how to copy the SipDialogSharedState object.

    if (other.m_pLocalUri != IMS_NULL)
    {
        m_pLocalUri = SipStack::CloneHeader(other.m_pLocalUri);
    }

    if (other.m_pRemoteUri != IMS_NULL)
    {
        m_pRemoteUri = SipStack::CloneHeader(other.m_pRemoteUri);
    }

    // Local & Remote Contact information
    if (other.m_pLocalTargetUri != IMS_NULL)
    {
        m_pLocalTargetUri = SipStack::CloneHeader(other.m_pLocalTargetUri);
    }

    if (other.m_pRemoteTargetUri != IMS_NULL)
    {
        m_pRemoteTargetUri = SipStack::CloneHeader(other.m_pRemoteTargetUri);
    }

    if (other.m_pLocalContactHeader != IMS_NULL)
    {
        m_pLocalContactHeader = static_cast<SipHeader*>(other.m_pLocalContactHeader->Clone());
    }

    for (IMS_UINT32 i = 0; i < other.m_objPendingRemoteTargets.GetSize(); ++i)
    {
        const PendingRemoteTarget* pRemoteTarget = other.m_objPendingRemoteTargets.GetAt(i);

        if (pRemoteTarget != IMS_NULL)
        {
            m_objPendingRemoteTargets.Append(new PendingRemoteTarget(*pRemoteTarget));
        }
    }
}

PUBLIC VIRTUAL SipDialogState::~SipDialogState()
{
    if (m_pLocalUri != IMS_NULL)
    {
        SipStack::FreeHeaderEx(m_pLocalUri);
    }

    if (m_pRemoteUri != IMS_NULL)
    {
        SipStack::FreeHeaderEx(m_pRemoteUri);
    }

    if (m_pLocalTargetUri != IMS_NULL)
    {
        SipStack::FreeHeaderEx(m_pLocalTargetUri);
    }

    if (m_pRemoteTargetUri != IMS_NULL)
    {
        SipStack::FreeHeaderEx(m_pRemoteTargetUri);
    }

    ClearRouteSet();

    if (m_pSharedState != IMS_NULL)
    {
        delete m_pSharedState;
    }

    if (m_pLocalContactHeader != IMS_NULL)
    {
        delete m_pLocalContactHeader;
        m_pLocalContactHeader = IMS_NULL;
    }

    // REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
    RemoveAllPendingRemoteTargets();

    IMS_TRACE_D("dtor: SipDialogState (%s: %s)", m_bIsCaller ? "__CALLER__" : "__CALLEE__",
            SipDebug::GetCharA1(m_strCallId.GetStr(), 8, '@'), 0);
}

/**
 * @brief Checks if the message received has a valid To-Tag value.
 *
 * If the dialog state has To-Tag and incoming message has no To-Tag,
 * then the validation will be failed.
 * If the invalid message is a request, then it rejects the request with 481 response.
 */
PUBLIC
IMS_SINT32 SipDialogState::CheckToTagValidity(IN const SipMessageInfo& objMsgInfo)
{
    ::SipMessage* pSipMsg = objMsgInfo.GetMessage();
    const SipMethod& objMethod = objMsgInfo.GetMethod();

    if (objMethod.Equals(SipMethod::INVALID))
    {
        return SipPrivate::MESSAGE_VALID;
    }

    // For CANCEL & non-2xx responses to CANCEL, relax the To-Tag validation.
    // As these are hop-by-hop responses, they can have different To-Tags.
    if (objMethod.Equals(SipMethod::CANCEL))
    {
        return SipPrivate::MESSAGE_VALID;
    }

    if (!SipStack::IsRequestMessage(pSipMsg))
    {
        if (objMethod.Equals(SipMethod::INVITE) &&
                (SipStack::GetStatusCode(pSipMsg) >= SipStatusCode::SC_300))
        {
            return SipPrivate::MESSAGE_VALID;
        }
    }

    IMS_BOOL bIsInvalid = IMS_FALSE;
    IMS_BOOL bLocalTagMismatched = IMS_FALSE;
    IMS_BOOL bRemoteTagMismatched = IMS_FALSE;

    AString strLocalTag = SipStack::GetParameter(m_pLocalUri, Sip::STR_TAG);
    AString strRemoteTag = SipStack::GetParameter(m_pRemoteUri, Sip::STR_TAG);
    AString strNewLocalTag;
    AString strNewRemoteTag;

    SipHeaderBase* pSipHdr;

    if (SipStack::IsRequestMessage(pSipMsg))
    {
        // Get local tag
        pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::TO);

        strNewLocalTag = SipStack::GetParameter(pSipHdr, Sip::STR_TAG);
        SipStack::FreeHeaderEx(pSipHdr);

        // Get remote tag
        pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::FROM);

        strNewRemoteTag = SipStack::GetParameter(pSipHdr, Sip::STR_TAG);
        SipStack::FreeHeaderEx(pSipHdr);
    }
    else
    {
        // Get local tag
        pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::FROM);

        strNewLocalTag = SipStack::GetParameter(pSipHdr, Sip::STR_TAG);
        SipStack::FreeHeaderEx(pSipHdr);

        // Get remote tag
        pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::TO);

        strNewRemoteTag = SipStack::GetParameter(pSipHdr, Sip::STR_TAG);
        SipStack::FreeHeaderEx(pSipHdr);
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
        if (SipStack::IsRequestMessage(pSipMsg) && !objMethod.Equals(SipMethod::INVITE))
        {
            // If the local tag is not matching,
            // then set the current local tag to this failed response.
            if (bLocalTagMismatched == IMS_TRUE)
            {
                SipPrivate::SetLastError(SipError::LOCAL_TAG_MISMATCH);
            }

            return SipPrivate::MESSAGE_INVALID_481;
        }
        else if (!SipStack::IsRequestMessage(pSipMsg) && objMethod.Equals(SipMethod::INVITE) &&
                (bRemoteTagMismatched == IMS_TRUE))
        {
            // Handle the forked response to INVITE request.
            return SipPrivate::MESSAGE_VALID_FORKED;
        }

        return SipPrivate::MESSAGE_INVALID;
    }

    return SipPrivate::MESSAGE_VALID;
}

PUBLIC
IMS_SINT32 SipDialogState::CompareTo(IN SipDialogState* pDState, IN ::SipMessage* pSipMsg,
        IN IMS_BOOL bCheckForked /*= IMS_FALSE*/)
{
    // Check if the Call-ID's match
    if (!m_strCallId.Equals(pDState->m_strCallId))
    {
        // Call-Id's does not match
        return NOT_MATCHED;
    }

    IMS_SINT32 nComparisonResult = NOT_MATCHED;
    IMS_BOOL bToTagLenient = IMS_FALSE;
    IMS_SINT32 nForkedMessage = FORKED_ANY;
    SipMethod objMethod = SipStack::GetMethod(pSipMsg);

    if (pSipMsg != IMS_NULL)
    {
        IMS_SINT32 nStatusCode = SipStatusCode::SC_INVALID;

        if (!SipStack::IsRequestMessage(pSipMsg))
        {
            nStatusCode = SipStack::GetStatusCode(pSipMsg);
        }

        // For response to CANCEL and non-2xx response to INVITE,
        // the comparison should be lenient against the To-Tags.

        // Fix for CSR 1-1316815
        // The To-Tag leniency should be present for CANCEL request,
        // response for CANCEL, and failure responses for INVITE.
        // If the forking flag is enabled and it's a 1xx/2xx response to SUBSCRIBE,
        // then set the bToTagLenient flag.
        // Also, reset the matching result since it is used only for detecting forked NOTIFYs.
        if (objMethod.Equals(SipMethod::CANCEL) ||
                (!SipStack::IsRequestMessage(pSipMsg) && objMethod.Equals(SipMethod::INVITE) &&
                        SipStatusCode::IsFinalFailure(nStatusCode)) ||
                (bCheckForked && !SipStack::IsRequestMessage(pSipMsg) &&
                        objMethod.Equals(SipMethod::SUBSCRIBE) &&
                        (nStatusCode >= SipStatusCode::SC_100) &&
                        (nStatusCode < SipStatusCode::SC_300)))
        {
            bToTagLenient = IMS_TRUE;
        }
        else if (bCheckForked && !SipStack::IsRequestMessage(pSipMsg) &&
                objMethod.Equals(SipMethod::INVITE) && (nStatusCode >= SipStatusCode::SC_100) &&
                (nStatusCode < SipStatusCode::SC_300))
        {
            bToTagLenient = IMS_TRUE;
            nForkedMessage = FORKED_INVITE;
        }
        else if (bCheckForked && SipStack::IsRequestMessage(pSipMsg) &&
                objMethod.Equals(SipMethod::NOTIFY))
        {
            nForkedMessage = FORKED_SUBSCRIBE;
        }
// Strict comparison - UPDATE request
#if 0
        else if (SipStack::IsRequestMessage(pSipMsg)
                && objMethod.Equals(SipMethod::UPDATE))
        {
            IMS_SINT32 nDState = GetState();

            if ((nDState != SipDState::STATE_EARLY)
                    && (nDState != SipDState::STATE_CONFIRMED))
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
    nComparisonResult =
            CompareHeaders(pDState->m_pLocalUri, m_pLocalUri, bToTagLenient, nForkedMessage);

    if (nComparisonResult == MATCHED)
    {
        // First pair of headers matched.
        // Now, go in for matching the second set of headers viz. the remote-address
        // of each dialog-state and return back whatever this comparison returns.
        nComparisonResult =
                CompareHeaders(pDState->m_pRemoteUri, m_pRemoteUri, bToTagLenient, nForkedMessage);
    }

    // If the SIP message is not NULL, use the type of the SIP message (Request/Response)
    // and the caller member variable in the existing dialog-state, to decide whether to do
    // (From:From, To:To) OR (From:To, To:From) [exact match]
    if ((pSipMsg != IMS_NULL) && (nComparisonResult != NOT_MATCHED))
    {
        // If the comparison returned MATCHED_OVERLAP_DIALING, we check whether this is an INVITE.
        // If it's not an INVITE, we return NOT_MATCHED.
        // Similarly, if extra digits were added in a response, we return NOT_MATCHED.
        if (nComparisonResult == MATCHED_OVERLAP_DIALING)
        {
            if (!SipStack::IsRequestMessage(pSipMsg) ||
                    (SipStack::IsRequestMessage(pSipMsg) && !objMethod.Equals(SipMethod::INVITE)))
            {
                nComparisonResult = NOT_MATCHED;
            }
        }

        if (nComparisonResult != NOT_MATCHED)
        {
            // If the request is CANCEL, then Equalss the Via branch parameter
            // also to have the exact match of dialog-state.
            if (SipStack::IsRequestMessage(pSipMsg) && objMethod.Equals(SipMethod::CANCEL))
            {
                if (!pDState->m_strRemoteViaBranch.IsNULL())
                {
                    if (!m_strRemoteViaBranch.IsNULL())
                    {
                        if (!m_strRemoteViaBranch.Equals(pDState->m_strRemoteViaBranch))
                        {
                            nComparisonResult = NOT_MATCHED;
                        }
                    }
                    else
                    {
                        nComparisonResult = NOT_MATCHED;
                    }
                }
                else
                {
                    if (!m_strRemoteViaBranch.IsNULL())
                    {
                        nComparisonResult = NOT_MATCHED;
                    }
                }
            }
        }
    }

    return nComparisonResult;
}

PUBLIC
IMS_BOOL SipDialogState::Equals(IN SipDialogState* pDState)
{
    if (pDState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!m_strCallId.Equals(pDState->m_strCallId))
    {
        return IMS_FALSE;
    }

    AString strTagVal = SipStack::GetParameter(m_pLocalUri, Sip::STR_TAG);
    AString strOtherTagVal = SipStack::GetParameter(pDState->m_pLocalUri, Sip::STR_TAG);

    if (!strTagVal.Equals(strOtherTagVal))
    {
        return IMS_FALSE;
    }

    strTagVal = SipStack::GetParameter(m_pRemoteUri, Sip::STR_TAG);
    strOtherTagVal = SipStack::GetParameter(pDState->m_pRemoteUri, Sip::STR_TAG);

    if (!strTagVal.Equals(strOtherTagVal))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
const ISipHeader* SipDialogState::GetContactHeader() const
{
    return m_pLocalContactHeader;
}

PUBLIC
AString SipDialogState::GetLocalTag() const
{
    return SipStack::GetParameter(m_pLocalUri, Sip::STR_TAG);
}

PUBLIC
AString SipDialogState::GetRemoteTag() const
{
    return SipStack::GetParameter(m_pRemoteUri, Sip::STR_TAG);
}

PUBLIC
IMS_BOOL SipDialogState::InitDialogDetails(IN ::SipMessage* pSipMsg)
{
    SipHeaderBase* pSipHdr;
    SipHeaderBase* pTmpSipHdr;

    // Call Id
    pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::CALL_ID);
    SipStack::EncodeHeaderBody(pSipHdr, IMS_FALSE, m_strCallId);
    SipStack::FreeHeaderEx(pSipHdr);

    // The dialog state is created by the outgoing dialog-creatable request
    if (m_bIsCaller)
    {
        // Local URI & Tag
        SipStack::FreeHeader(m_pLocalUri);

        pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::FROM);

#ifdef __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__
        pTmpSipHdr = pSipHdr;
        pSipHdr = IMS_NULL;
#else
        pTmpSipHdr = SipStack::CloneHeader(pSipHdr);
#endif

        if (pTmpSipHdr != IMS_NULL)
        {
            m_pLocalUri = SipStack::CopyHeader(m_pLocalUri, pTmpSipHdr);
            SipStack::FreeHeaderEx(pTmpSipHdr);
        }

        SipStack::FreeHeaderEx(pSipHdr);

        // Remote URI & Tag
        SipStack::FreeHeader(m_pRemoteUri);

        pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::TO);

#ifdef __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__
        pTmpSipHdr = pSipHdr;
        pSipHdr = IMS_NULL;
#else
        pTmpSipHdr = SipStack::CloneHeader(pSipHdr);
#endif

        if (pTmpSipHdr != IMS_NULL)
        {
            m_pRemoteUri = SipStack::CopyHeader(m_pRemoteUri, pTmpSipHdr);
            SipStack::FreeHeaderEx(pTmpSipHdr);
        }

        SipStack::FreeHeaderEx(pSipHdr);

        // Via branch parameter
        pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::VIA);
        m_strLocalViaBranch = SipStack::GetParameter(pSipHdr, Sip::STR_BRANCH);
        SipStack::FreeHeaderEx(pSipHdr);
    }
    // The dialog state is created by the incoming dialog-creatable request
    else
    {
        // Local URI & Tag
        SipStack::FreeHeader(m_pLocalUri);

        pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::TO);

#ifdef __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__
        pTmpSipHdr = pSipHdr;
        pSipHdr = IMS_NULL;
#else
        pTmpSipHdr = SipStack::CloneHeader(pSipHdr);
#endif

        if (pTmpSipHdr != IMS_NULL)
        {
            m_pLocalUri = SipStack::CopyHeader(m_pLocalUri, pTmpSipHdr);
            SipStack::FreeHeaderEx(pTmpSipHdr);
        }

        SipStack::FreeHeaderEx(pSipHdr);

        // Remote URI & Tag
        SipStack::FreeHeader(m_pRemoteUri);

        pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::FROM);

#ifdef __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__
        pTmpSipHdr = pSipHdr;
        pSipHdr = IMS_NULL;
#else
        pTmpSipHdr = SipStack::CloneHeader(pSipHdr);
#endif

        if (pTmpSipHdr != IMS_NULL)
        {
            m_pRemoteUri = SipStack::CopyHeader(m_pRemoteUri, pTmpSipHdr);
            SipStack::FreeHeaderEx(pTmpSipHdr);
        }

        SipStack::FreeHeaderEx(pSipHdr);

        // Via branch parameter
        pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::VIA);
        m_strRemoteViaBranch = SipStack::GetParameter(pSipHdr, Sip::STR_BRANCH);
        SipStack::FreeHeaderEx(pSipHdr);
    }

    // Create a shared dialog state
    if (m_pSharedState == IMS_NULL)
    {
        m_pSharedState = new SipDialogSharedState();

        if (m_pSharedState == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a shared dialog state failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipDialogState::InitDialogDetails(IN IMS_SINT32 nTrigger, IN const SipDialogState* pDState)
{
    if (nTrigger == DIALOG_CANCELLED)
    {
        // Copy the remote sequence number from INVITE dialog.
        m_nRemoteCSeq = pDState->m_nRemoteCSeq;
        m_nRemoteCSeqForInvite = pDState->m_nRemoteCSeqForInvite;
    }
    else if (nTrigger == DIALOG_FORKED_REQUEST)
    {
        // Copy the information from the original dialog info.
        m_bIsCaller = pDState->m_bIsCaller;
        m_nLocalCSeq = pDState->m_nLocalCSeq;
    }
    else if (nTrigger == DIALOG_FORKED_RESPONSE)
    {
        // It will be updated from the previous request later.
        m_nLocalCSeq = 1;

#ifdef __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__
        SipHeaderBase* pTmpSipHdr = SipStack::CloneHeader(m_pRemoteUri);

        if (pTmpSipHdr != IMS_NULL)
        {
            SipStack::FreeHeader(m_pRemoteUri);

            m_pRemoteUri = SipStack::CopyHeader(m_pRemoteUri, pTmpSipHdr);
            SipStack::FreeHeaderEx(pTmpSipHdr);
        }

        // Remove the remote-tag because it is updated by the previous response
        // which is creating a dialog (early)
        SipStack::RemoveParameter(Sip::STR_TAG, m_pRemoteUri);
#else
        // Remove the remote-tag because it is updated by the previous response
        // which is creating a dialog (early)
        SipStack::RemoveParameter(Sip::STR_TAG, m_pRemoteUri);
#endif
    }
    else
    {
        return IMS_FALSE;
    }

    // HEADER_REQ_SESSION-ID
    m_strSessionId = pDState->m_strSessionId;

    if ((pDState->m_nFromChangeOption & FROM_CHANGE_ON_INVITE_REQUEST) != 0)
    {
        SetFromChangeOption(FROM_CHANGE_ON_INVITE_REQUEST);
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipDialogState::InitRequest(IN const SipMethod& objMethod, IN_OUT ::SipMessage*& pSipMsg)
{
    // Method
    SipStack::SetMethod(objMethod, pSipMsg);

    // Request-URI : Set the remote target URI
    SipAddrSpec* pAddrSpec = SipStack::GetAddrSpec(m_pRemoteTargetUri);

    if (pAddrSpec == IMS_NULL)
    {
        IMS_TRACE_E(0, "There is no remote target information (no contact)", 0, 0, 0);
        return IMS_FALSE;
    }

    SipStack::SetRequestUri(pAddrSpec, pSipMsg);
    SipStack::FreeAddrSpec(pAddrSpec);

    // From
    SipHeaderBase* pSipHdr = SipStack::CloneHeader(m_pLocalUri);
    SipStack::SetHeaderType(ISipHeader::FROM, pSipHdr);

    if (!SipStack::SetHeader(pSipHdr, pSipMsg))
    {
        SipStack::FreeHeaderEx(pSipHdr);
        return IMS_FALSE;
    }

    SipStack::FreeHeaderEx(pSipHdr);

    // To
    pSipHdr = SipStack::CloneHeader(m_pRemoteUri);
    SipStack::SetHeaderType(ISipHeader::TO, pSipHdr);

    if (!SipStack::SetHeader(pSipHdr, pSipMsg))
    {
        SipStack::FreeHeaderEx(pSipHdr);
        return IMS_FALSE;
    }

    SipStack::FreeHeaderEx(pSipHdr);

    // Call-ID
    pSipHdr = SipStack::DecodeHeader(ISipHeader::CALL_ID, m_strCallId);

    if (!SipStack::SetHeader(pSipHdr, pSipMsg))
    {
        SipStack::FreeHeaderEx(pSipHdr);
        return IMS_FALSE;
    }

    SipStack::FreeHeaderEx(pSipHdr);

    // Via : Transport Layer will set the header field

    // Route
    for (IMS_UINT32 i = 0; i < m_objRouteSet.GetSize(); ++i)
    {
        pSipHdr = m_objRouteSet.GetAt(i);

        if (pSipHdr != IMS_NULL)
        {
            if (!SipStack::InsertHeader(pSipHdr, i, pSipMsg))
            {
                return IMS_FALSE;
            }
        }
    }

    // Contact
    if (IsContactMandatory(
                sipcore::SipMessage::TYPE_REQUEST, objMethod, SipStatusCode::SC_INVALID, IMS_FALSE))
    {
        pSipHdr = SipStack::CloneHeader(m_pLocalTargetUri);

        if (!SipStack::SetHeader(pSipHdr, pSipMsg))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }

        SipStack::FreeHeaderEx(pSipHdr);
    }

    // HEADER_REQ_SESSION-ID
    if (m_strSessionId.GetLength() > 0)
    {
        pSipHdr = SipStack::DecodeHeader(
                ISipHeader::SESSION_ID, AString::ConstNull(), m_strSessionId);

        (void)SipStack::SetHeader(pSipHdr, pSipMsg);
        SipStack::FreeHeaderEx(pSipHdr);
    }

    return IMS_TRUE;
}

// CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
PUBLIC
IMS_RESULT SipDialogState::SetContactParameter(
        IN const AString& strParameter, IN IMS_SINT32 nOperation /*= 0 (0: ADD, 1: REMOVE)*/)
{
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

        if (!SipStack::SetParameter(m_pLocalTargetUri, strName, strValue))
        {
            return IMS_FAILURE;
        }

        if ((m_pLocalContactHeader != IMS_NULL) &&
                (m_pLocalContactHeader->SetParameter(strName, strValue) != IMS_SUCCESS))
        {
            return IMS_FAILURE;
        }
    }
    else
    {
        SipStack::RemoveParameter(strName, m_pLocalTargetUri);

        if (m_pLocalContactHeader != IMS_NULL)
        {
            m_pLocalContactHeader->RemoveParameter(strName);
        }
    }

    return IMS_SUCCESS;
}

PUBLIC
IMS_SINT32 SipDialogState::UpdateDialogDetails(IN const SipMessageInfo& objMsgInfo,
        IN IMS_SINT32 nUsageState, IN IMS_SINT32 nAction, IN IMS_SINT32 nTrigger)
{
    // If the dialog's state is already in TERMINATED state, skip the below procedures.
    if (m_nState == SipDState::STATE_TERMINATED)
    {
        IMS_TRACE_D("INVALID STATE : Dialog (%s) is already in TERMINATED state",
                SipDebug::GetCharA1(m_strCallId.GetStr(), 8, '@'), 0, 0);
        return SipPrivate::MESSAGE_VALID;
    }

    ::SipMessage* pSipMsg = objMsgInfo.GetMessage();

    if (objMsgInfo.IsOutgoingMessage())
    {
        if (!SipStack::IsRequestMessage(pSipMsg))
        {
            if (GetState() == SipDState::STATE_INIT)
            {
                // Update To-Tag when sending the response to the incoming SIP request
                UpdateComponents(objMsgInfo);

                // HEADER_REQ_SESSION-ID
                if (SipFeatures::IsHeaderSessionIdRequired(objMsgInfo.GetSlotId()))
                {
                    UpdateSessionId(objMsgInfo);
                }
            }
        }
        else
        {
            if (GetState() == SipDState::STATE_INIT)
            {
                // Update the local command sequence number
                m_nLocalCSeq = SipStack::GetCSeqNumber(pSipMsg);

                // HEADER_REQ_SESSION-ID
                if (SipFeatures::IsHeaderSessionIdRequired(objMsgInfo.GetSlotId()))
                {
                    UpdateSessionId(objMsgInfo);
                }
            }
        }

        if (GetState() != SipDState::STATE_CONFIRMED)
        {
            UpdateFromChangeOption(objMsgInfo);
        }
    }
    else
    {
        IMS_BOOL bIsInviteConfirmed = IMS_FALSE;

        // Update the dialog identifiers if it does not already set
        if (!SipStack::IsRequestMessage(pSipMsg))
        {
            // If this is a response, then update only the To-Tag in the dialog state
            // if it has not been set as yet.
            // The From, Call-ID and other fields have already been set at the time
            // of sending out the request.
            // NOTE: The to-tag updates should only be done for a >= 2xx response.
            // Only RPR's with a different tag cause formation of different call legs.
            // As we don't support multiple call legs, for now we will update the to-tag
            // for any incoming response.
            SipHeaderBase* pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::TO);
            AString strMsgTag = SipStack::GetParameter(pSipHdr, Sip::STR_TAG);
            SipStack::FreeHeaderEx(pSipHdr);

            AString strDialogTag = SipStack::GetParameter(m_pRemoteUri, Sip::STR_TAG);

            if ((strDialogTag.GetLength() == 0) && (strMsgTag.GetLength() > 0))
            {
                if (!SipStack::SetParameter(m_pRemoteUri, Sip::STR_TAG, strMsgTag))
                    return SipPrivate::MESSAGE_FAILED;
            }

            // The route set for the dialog MUST be re-computed based on the 2xx response
            IMS_SINT32 nStatusCode = SipStack::GetStatusCode(pSipMsg);

            if (objMsgInfo.GetMethod().Equals(SipMethod::INVITE) &&
                    (SipStatusCode::IsFinalSuccess(nStatusCode)))
            {
                bIsInviteConfirmed = IMS_TRUE;
            }
        }
        else
        {
            // Validate Cseq:
            // It will be handled in the HandleRequest(...) in SipServerTransactionState

            m_nRemoteCSeq = SipStack::GetCSeqNumber(pSipMsg);

            if (objMsgInfo.GetMethod().Equals(SipMethod::INVITE))
            {
                m_nRemoteCSeqForInvite = m_nRemoteCSeq;
            }

            // It is a request that has been received. If the From header has a tag, and
            //  a) If the original call was from us and the to-tag in the dialog state is NULL,
            //    then we need to update it with the received tag.
            //  b) If the original call was from the other end,
            //    and the From tag in the dialog state is NULL,
            //    then we need to update it with the received tag.
            SipHeaderBase* pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::FROM);
            AString strMsgTag = SipStack::GetParameter(pSipHdr, Sip::STR_TAG);
            SipStack::FreeHeaderEx(pSipHdr);

            if (strMsgTag.GetLength() > 0)
            {
                AString strDialogTag = SipStack::GetParameter(m_pRemoteUri, Sip::STR_TAG);

                if (strDialogTag.GetLength() == 0)
                {
                    if (!SipStack::SetParameter(m_pRemoteUri, Sip::STR_TAG, strMsgTag))
                    {
                        return SipPrivate::MESSAGE_FAILED;
                    }
                }
            }

            if (objMsgInfo.GetMethod().Equals(SipMethod::INVITE))
            {
                pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::VIA);
                m_strRemoteViaBranch = SipStack::GetParameter(pSipHdr, AString(Sip::STR_BRANCH));
                SipStack::FreeHeaderEx(pSipHdr);
            }
        }

        if (GetState() != SipDState::STATE_CONFIRMED)
        {
            // HEADER_REQ_SESSION-ID
            if (SipFeatures::IsHeaderSessionIdRequired(objMsgInfo.GetSlotId()))
            {
                UpdateSessionId(objMsgInfo);
            }

            UpdateFromChangeOption(objMsgInfo);

            if (m_objRouteSet.GetSize() == 0)
            {
                if (!CreateRouteSet(objMsgInfo))
                {
                    return SipPrivate::MESSAGE_FAILED;
                }
            }
            else
            {
                // Route headers already exist in the route set.
                // Update only the contact header if required.

                // The route set for the dialog MUST be re-computed based on the 2xx response
                if (bIsInviteConfirmed)
                {
                    m_bPreloadedSet = IMS_FALSE;

                    ClearRouteSet();

                    if (!CreateRouteSet(objMsgInfo))
                    {
                        return SipPrivate::MESSAGE_FAILED;
                    }
                }
                else
                {
                    if (!UpdateRouteSet(objMsgInfo))
                    {
                        return SipPrivate::MESSAGE_FAILED;
                    }
                }
            }
        }
    }

    // Update the Contact information according to the mode
    UpdateContact(objMsgInfo);

    // Update a dialog state
    UpdateState(nUsageState, nAction, nTrigger);

    // Update "remote-URI" if changed
    UpdateRemoteUri(objMsgInfo);

    return SipPrivate::MESSAGE_VALID;
}

PUBLIC
void SipDialogState::UpdateLocalCSeq(IN IMS_UINT32 nCSeq)
{
    IMS_TRACE_I("Dialog(%s): CSeq: %d >> %d", SipDebug::GetCharA1(m_strCallId.GetStr(), 8, '@'),
            m_nLocalCSeq, nCSeq);

    m_nLocalCSeq = nCSeq;
}

PUBLIC
IMS_SINT32 SipDialogState::ValidateRemoteCSeq(
        IN ::SipMessage* pSipMsg, IN IMS_SINT32 nPrevStatusCode /*= 0*/)
{
    SipMethod objMethod = SipStack::GetMethod(pSipMsg);
    IMS_UINT32 nSeqNum = SipStack::GetCSeqNumber(pSipMsg);

    if (nSeqNum == SipPrivate::INVALID_SEQ_NUM)
    {
        return SipPrivate::MESSAGE_FAILED;
    }

    // If a retransmitted ACK of lower CSeq appears after a fresh INVITE of higher CSeq
    // has been received (for example, after the first INVITE was rejected with a 401/407),
    // it can be dropped.

    // For an ACK, if the CSeq is lower than the remote CSeq, it can be dropped.
    // This may happen if the ACK to a 401 appears after a fresh INVITE
    // with higher CSeq has been received by the F/W.
    if (objMethod.Equals(SipMethod::ACK))
    {
        if (m_nRemoteCSeqForInvite != nSeqNum)
        {
            // ACK request of incoming re-INVITE can be received
            // after new re-INVITE request is received.
            if (!SipStatusCode::IsFinalSuccess(nPrevStatusCode))
            {
                IMS_TRACE_E(0, "OUT OF SEQUENCE: ACK(%d|%d)", m_nRemoteCSeqForInvite, nSeqNum, 0);
                return SipPrivate::MESSAGE_DISCARDED;
            }
        }

        return SipPrivate::MESSAGE_VALID;
    }

    // Check if the CSeq is higher.
    // Note that for ACK and CANCEL, the CSeq will be same as that in initial INVITE.
    if (objMethod.Equals(SipMethod::CANCEL))
    {
        if (m_nRemoteCSeqForInvite == nSeqNum)
        {
            return SipPrivate::MESSAGE_VALID;
        }
        else if (m_nRemoteCSeqForInvite < nSeqNum)
        {
            SipPrivate::SetLastError(SipError::CSEQ_VALUE_MISMATCHED);
            return SipPrivate::MESSAGE_INVALID_400;
        }
        else
        {
            SipPrivate::SetLastError(SipError::REQUEST_OUT_OF_ORDER);
            return SipPrivate::MESSAGE_INVALID_500;
        }
    }

    // Checks for PRACK.
    if (objMethod.Equals(SipMethod::PRACK))
    {
        // Application will handle the procedure for PRACK.
    }

    // 3 Loop detection ????

    // a) If CSeq is higher, it should be responded with a 400 Bad Request.
    // b) If CSeq is the same, it is a retransmission, so it can be dropped.
    // c) If CSeq is the lower, it should be responded with a 500 response.
    if (m_nRemoteCSeq > nSeqNum)
    {
        SipPrivate::SetLastError(SipError::REQUEST_OUT_OF_ORDER);
        return SipPrivate::MESSAGE_INVALID_500;
    }
    else if (m_nRemoteCSeq == nSeqNum)
    {
        // It needs to be checked if the request is looped or not.
        // CheckRequestLoop(pSipMsg);
        SipPrivate::SetLastError(SipError::LOOP_DETECTED);
        return SipPrivate::MESSAGE_INVALID_400;
    }

    return SipPrivate::MESSAGE_VALID;
}

PUBLIC
IMS_BOOL SipDialogState::AddDialogUsage(IN SipDialogEx* pDialogEx)
{
    if (m_pSharedState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_pSharedState->AddDialog(pDialogEx);
}

PUBLIC
void SipDialogState::RemoveDialogUsage(IN SipDialogEx* pDialogEx)
{
    if (m_pSharedState == IMS_NULL)
    {
        return;
    }

    m_pSharedState->RemoveDialog(pDialogEx);
}

PUBLIC
SipDialogEx* SipDialogState::GetDialogUsage(IN const SipMessageInfo& objMsgInfo)
{
    if (m_pSharedState == IMS_NULL)
    {
        return IMS_NULL;
    }

    return m_pSharedState->GetDialog(objMsgInfo);
}

PUBLIC
IMS_BOOL SipDialogState::HasMultipleDialogUsages() const
{
    if (m_pSharedState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_pSharedState->HasMultipleDialogUsages();
}

PUBLIC GLOBAL IMS_BOOL SipDialogState::IsContactMandatory(IN IMS_SINT32 nMsgType,
        IN const SipMethod& objMethod, IN IMS_SINT32 nStatusCode,
        IN IMS_BOOL bContactInAll1xxRequired)
{
    if (nMsgType == sipcore::SipMessage::TYPE_REQUEST)
    {
        // REGISTER, OPTIONS, and PUBLISH: added by imscore requirements.
        if (objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::SUBSCRIBE) ||
                objMethod.Equals(SipMethod::NOTIFY) || objMethod.Equals(SipMethod::REFER) ||
                objMethod.Equals(SipMethod::UPDATE) || objMethod.Equals(SipMethod::REGISTER) ||
                objMethod.Equals(SipMethod::OPTIONS) || objMethod.Equals(SipMethod::PUBLISH))
        {
            return IMS_TRUE;
        }
    }
    else
    {
        if ((nStatusCode >= SipStatusCode::SC_200) && (nStatusCode < SipStatusCode::SC_300))
        {
            // OPTIONS & NOTIFY: added by imscore requirements.
            if (objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::SUBSCRIBE) ||
                    objMethod.Equals(SipMethod::REFER) || objMethod.Equals(SipMethod::UPDATE) ||
                    objMethod.Equals(SipMethod::OPTIONS) || objMethod.Equals(SipMethod::NOTIFY))
            {
                return IMS_TRUE;
            }
        }
        else if ((nStatusCode >= SipStatusCode::SC_300) && (nStatusCode < SipStatusCode::SC_400))
        {
            if (objMethod.Equals(SipMethod::SUBSCRIBE) || objMethod.Equals(SipMethod::NOTIFY))
            {
                return IMS_TRUE;
            }
        }
        // PATCH_FOR_CONTACT_HEADER_IN_ALL_1XX
        else if (SipStatusCode::IsProvisional(nStatusCode))
        {
            if (objMethod.Equals(SipMethod::INVITE))
            {
                // 3GPP Profile: 18x / 19x
                if (bContactInAll1xxRequired || (nStatusCode >= SipStatusCode::SC_180))
                {
                    return IMS_TRUE;
                }
            }
        }
    }

    return IMS_FALSE;
}

PRIVATE
void SipDialogState::ClearRouteSet()
{
    if (m_objRouteSet.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objRouteSet.GetSize(); ++i)
    {
        SipHeaderBase* pSipHdr = m_objRouteSet.GetAt(i);

        SipStack::FreeHeaderEx(pSipHdr);
    }

    m_objRouteSet.Clear();
}

PRIVATE
IMS_BOOL SipDialogState::CreateRouteSet(IN const SipMessageInfo& objMsgInfo)
{
    ::SipMessage* pSipMsg = objMsgInfo.GetMessage();

    if (SipDialogState::IsTargetRefreshMessage(pSipMsg))
    {
        IMS_SINT32 nHCount = SipStack::GetHeaderCount(pSipMsg, ISipHeader::RECORD_ROUTE);

        if (nHCount != 0)
        {
            SipHeaderBase* pRrHeader;
            SipHeaderBase* pSipHdr;

            IMS_SINT32 nIndex;

            // Reverse the record-routes into the route list only
            // if a response message is received (UAC end).
            // Else, copy the record-routes into routes as they are (UAS end).
            if (!SipStack::IsRequestMessage(pSipMsg))
            {
                IMS_SINT32 nStatusCode = SipStack::GetStatusCode(pSipMsg);

                if (!SipStack::IsMessageRpr(pSipMsg) && SipStatusCode::IsProvisional(nStatusCode))
                {
                    m_bPreloadedSet = IMS_TRUE;
                }

                for (nIndex = nHCount - 1; nIndex >= 0; nIndex--)
                {
                    pRrHeader = SipStack::GetHeader(pSipMsg, ISipHeader::RECORD_ROUTE, nIndex);

                    SipStack::SetHeaderType(ISipHeader::ROUTE, pRrHeader);

                    pSipHdr = SipStack::CloneHeader(pRrHeader);

                    SipStack::FreeHeaderEx(pRrHeader);

                    if (!m_objRouteSet.InsertAt(pSipHdr, (nHCount - 1) - nIndex))
                    {
                        SipStack::FreeHeaderEx(pSipHdr);
                        return IMS_FALSE;
                    }
                }
            }
            else
            {
                for (nIndex = 0; nIndex < nHCount; nIndex++)
                {
                    pRrHeader = SipStack::GetHeader(pSipMsg, ISipHeader::RECORD_ROUTE, nIndex);

                    SipStack::SetHeaderType(ISipHeader::ROUTE, pRrHeader);

                    pSipHdr = SipStack::CloneHeader(pRrHeader);

                    SipStack::FreeHeaderEx(pRrHeader);

                    if (!m_objRouteSet.Append(pSipHdr))
                    {
                        SipStack::FreeHeaderEx(pSipHdr);
                        return IMS_FALSE;
                    }
                }
            }
        }

        // 3 Contact or From or Request-URI need to be extracted ???
        UpdateContact(objMsgInfo);
    }

    return IMS_TRUE;
}

PRIVATE
void SipDialogState::UpdateComponents(IN const SipMessageInfo& objMsgInfo)
{
    ::SipMessage* pSipMsg = objMsgInfo.GetMessage();
    AString strTmpVal;
    SipHeaderBase* pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::CALL_ID);
    SipStack::EncodeHeaderBody(pSipHdr, IMS_FALSE, strTmpVal);

    if ((m_strCallId.GetLength() == 0) || (!m_strCallId.Equals(strTmpVal)))
    {
        m_strCallId = strTmpVal;
    }

    SipStack::FreeHeaderEx(pSipHdr);

    // Local URI & Tag
    SipStack::FreeHeader(m_pLocalUri);

    SipHeaderBase* pTmpSipHdr;
    IMS_SINT32 nHType;

    if (SipStack::IsRequestMessage(pSipMsg))
    {
        if (objMsgInfo.IsOutgoingMessage())
        {
            nHType = ISipHeader::FROM;
        }
        else
        {
            nHType = ISipHeader::TO;
        }
    }
    else
    {
        if (objMsgInfo.IsOutgoingMessage())
        {
            nHType = ISipHeader::TO;
        }
        else
        {
            nHType = ISipHeader::FROM;
        }
    }

    pSipHdr = SipStack::GetHeader(pSipMsg, nHType);

#ifdef __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__
    pTmpSipHdr = pSipHdr;
    pSipHdr = IMS_NULL;
#else
    pTmpSipHdr = SipStack::CloneHeader(pSipHdr);
#endif

    if (pTmpSipHdr != IMS_NULL)
    {
        m_pLocalUri = SipStack::CopyHeader(m_pLocalUri, pTmpSipHdr);
        SipStack::FreeHeaderEx(pTmpSipHdr);
    }

    SipStack::FreeHeaderEx(pSipHdr);

    // Remote URI & Tag
    SipStack::FreeHeader(m_pRemoteUri);

    if (SipStack::IsRequestMessage(pSipMsg))
    {
        if (objMsgInfo.IsOutgoingMessage())
        {
            nHType = ISipHeader::TO;
        }
        else
        {
            nHType = ISipHeader::FROM;
        }
    }
    else
    {
        if (objMsgInfo.IsOutgoingMessage())
        {
            nHType = ISipHeader::FROM;
        }
        else
        {
            nHType = ISipHeader::TO;
        }
    }

    pSipHdr = SipStack::GetHeader(pSipMsg, nHType);

#ifdef __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__
    pTmpSipHdr = pSipHdr;
    pSipHdr = IMS_NULL;
#else
    pTmpSipHdr = SipStack::CloneHeader(pSipHdr);
#endif

    if (pTmpSipHdr != IMS_NULL)
    {
        m_pRemoteUri = SipStack::CopyHeader(m_pRemoteUri, pTmpSipHdr);
        SipStack::FreeHeaderEx(pTmpSipHdr);
    }

    SipStack::FreeHeaderEx(pSipHdr);
}

PRIVATE
void SipDialogState::UpdateContact(IN const SipMessageInfo& objMsgInfo)
{
    ::SipMessage* pSipMsg = objMsgInfo.GetMessage();

    if (!IsTargetRefreshMessage(pSipMsg))
    {
        // REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
        if (objMsgInfo.IsOutgoingMessage() && !SipStack::IsRequestMessage(pSipMsg))
        {
            IMS_SINT32 nStatusCode = SipStack::GetStatusCode(pSipMsg);

            if (nStatusCode != SipStatusCode::SC_100)
            {
                RemovePendingRemoteTarget(objMsgInfo);
            }
        }
        return;
    }

    IMS_SINT32 nHCount = SipStack::GetHeaderCount(pSipMsg, ISipHeader::CONTACT_NORMAL);

    if (nHCount > 0)
    {
        if (objMsgInfo.IsOutgoingMessage())
        {
            SipStack::FreeHeader(m_pLocalTargetUri);

            SipHeaderBase* pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::CONTACT_NORMAL);

            if (pSipHdr != IMS_NULL)
            {
                m_pLocalTargetUri = SipStack::CopyHeader(m_pLocalTargetUri, pSipHdr);
                SipStack::FreeHeaderEx(pSipHdr);
            }

            if (m_pLocalContactHeader != IMS_NULL)
            {
                delete m_pLocalContactHeader;
                m_pLocalContactHeader = IMS_NULL;
            }

            if (SipStack::IsValidHeader(m_pLocalTargetUri))
            {
                m_pLocalContactHeader = new SipHeader(m_pLocalTargetUri);
            }

            // REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
            if (!SipStack::IsRequestMessage(pSipMsg))
            {
                UpdateAndRemovePendingRemoteTarget(objMsgInfo);
            }
        }
        else
        {
            // REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
            if ((m_nState != SipDState::STATE_INIT) && SipStack::IsRequestMessage(pSipMsg))
            {
                // Store the remote target and it will be updated after sending 2xx response.
                AddPendingRemoteTarget(objMsgInfo);
            }
            else
            {
                SipStack::FreeHeader(m_pRemoteTargetUri);

                SipHeaderBase* pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::CONTACT_NORMAL);

                if (pSipHdr != IMS_NULL)
                {
                    m_pRemoteTargetUri = SipStack::CopyHeader(m_pRemoteTargetUri, pSipHdr);
                    SipStack::FreeHeaderEx(pSipHdr);
                }
            }
        }
    }
}

PRIVATE
IMS_BOOL SipDialogState::UpdateRemoteUri(IN const SipMessageInfo& objMsgInfo)
{
    ::SipMessage* pSipMsg = objMsgInfo.GetMessage();
    IMS_BOOL bFromChangeable = IMS_FALSE;

    if ((GetState() != SipDState::STATE_INIT) && IsFromChangeCapable() &&
            (objMsgInfo.GetMethod().Equals(SipMethod::INVITE) ||
                    objMsgInfo.GetMethod().Equals(SipMethod::UPDATE)) &&
            !SipStack::IsRequestMessage(pSipMsg))
    {
        IMS_SINT32 nStatusCode = SipStack::GetStatusCode(pSipMsg);

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            bFromChangeable = IMS_TRUE;
        }
    }

    if (!bFromChangeable)
    {
        return IMS_FALSE;
    }

    SipHeaderBase* pTmpSipHdr;
    IMS_SINT32 nHType;

    if (SipStack::IsRequestMessage(pSipMsg))
    {
        if (objMsgInfo.IsOutgoingMessage())
        {
            nHType = ISipHeader::TO;
        }
        else
        {
            nHType = ISipHeader::FROM;
        }
    }
    else
    {
        if (objMsgInfo.IsOutgoingMessage())
        {
            nHType = ISipHeader::FROM;
        }
        else
        {
            nHType = ISipHeader::TO;
        }
    }

    // Remote URI & Tag
    SipStack::FreeHeader(m_pRemoteUri);

    SipHeaderBase* pSipHdr = SipStack::GetHeader(pSipMsg, nHType);

#ifdef __IMS_SIP_DIALOG_COMPONENT_BY_REFERENCE__
    pTmpSipHdr = pSipHdr;
    pSipHdr = IMS_NULL;
#else
    pTmpSipHdr = SipStack::CloneHeader(pSipHdr);
#endif

    if (pTmpSipHdr != IMS_NULL)
    {
        m_pRemoteUri = SipStack::CopyHeader(m_pRemoteUri, pTmpSipHdr);
        SipStack::FreeHeaderEx(pTmpSipHdr);
    }

    SipStack::FreeHeaderEx(pSipHdr);

    IMS_TRACE_D("DialogState: from-change is done", 0, 0, 0);

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SipDialogState::UpdateRouteSet(IN const SipMessageInfo& objMsgInfo)
{
    ::SipMessage* pSipMsg = objMsgInfo.GetMessage();

    if (SipDialogState::IsTargetRefreshMessage(pSipMsg))
    {
        if (m_bPreloadedSet)
        {
            // This indicates that the existing route set is only the preloaded one.
            // So, delete the preloaded one and form the fresh route set.
            ClearRouteSet();

            if (!CreateRouteSet(objMsgInfo))
            {
                return IMS_FALSE;
            }

            m_bPreloadedSet = IMS_FALSE;
        }
        else
        {
            // This is an established route set.
            // Only last entry which indicates the contact address can be changed.
            UpdateContact(objMsgInfo);
        }
    }

    return IMS_TRUE;
}

// HEADER_REQ_SESSION-ID
PRIVATE
void SipDialogState::UpdateSessionId(IN const SipMessageInfo& objMsgInfo)
{
    if (m_strSessionId.GetLength() > 0)
    {
        // Session-ID is already set by the previous request or response.
        return;
    }

    SipHeaderBase* pSipHdr = SipStack::GetHeader(objMsgInfo.GetMessage(), ISipHeader::SESSION_ID);

    if (SipStack::IsValidHeader(pSipHdr))
    {
        m_strSessionId = pSipHdr->GetValue();
        IMS_TRACE_I("DialogState: Session-ID (%s)", m_strSessionId.GetStr(), 0, 0);
    }

    SipStack::FreeHeaderEx(pSipHdr);
}

PRIVATE
void SipDialogState::UpdateState(
        IN IMS_SINT32 nUsageState, IN IMS_SINT32 nAction, IN IMS_SINT32 nTrigger)
{
    IMS_SINT32 nNextState = SipDState::STATE_MAX;

    (void)nTrigger;

    switch (nAction)
    {
        case SipDState::ACTION_TRANSIT_STATE:
            // If the shared state has a multiple dialog usages, do not transit the state.
            nNextState = nUsageState;

            if (m_pSharedState->IsShared() && (nUsageState == SipDState::STATE_TERMINATED))
            {
                // Do not transit the state.
                nNextState = SipDState::STATE_MAX;
            }
            break;

        case SipDState::ACTION_DESTROY_USAGE:
            // If the shared state has a multiple dialog usages, do not transit the state.
            if (!m_pSharedState->IsShared())
            {
                nNextState = SipDState::STATE_TERMINATED;
            }
            break;

        case SipDState::ACTION_DESTROY_DIALOG:
            nNextState = SipDState::STATE_TERMINATED;
            break;

        case SipDState::ACTION_IGNORE:
        default:
            return;
    }

    if ((nNextState >= SipDState::STATE_INIT) && (nNextState < SipDState::STATE_MAX))
    {
        // clang-format off
        static const IMS_CHAR* STATE[SipDState::STATE_MAX] =
                {
                    "INITIALIZED",
                    "TERMINATED",
                    "EARLY",
                    "CONFIRMED"
                };
        // clang-format on

        IMS_TRACE_I("___ DIALOG STATE: (%s) >>> (%s) ___", STATE[m_nState], STATE[nNextState], 0);

        m_nState = nNextState;
    }
}

// REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
PRIVATE
void SipDialogState::AddPendingRemoteTarget(IN const SipMessageInfo& objMsgInfo)
{
    ::SipMessage* pSipMsg = objMsgInfo.GetMessage();

    AString strKey = SipStack::GetViaBranchParameter(pSipMsg);

    if (strKey.GetLength() == 0)
    {
        // no-op
        return;
    }

    // Checks if the pending remote target is already present or not
    for (IMS_UINT32 i = 0; i < m_objPendingRemoteTargets.GetSize(); ++i)
    {
        const PendingRemoteTarget* pRemoteTarget = m_objPendingRemoteTargets.GetAt(i);

        if (pRemoteTarget == IMS_NULL)
        {
            continue;
        }

        if (strKey.Equals(pRemoteTarget->strKey))
        {
            IMS_TRACE_D("PendingRemoteTarget(add): key(dup)=%s, count=%d",
                    SipDebug::GetCharA1(strKey.GetStr(), 8, '@'),
                    m_objPendingRemoteTargets.GetSize(), 0);
            return;
        }
    }

    SipHeaderBase* pContact = IMS_NULL;
    SipHeaderBase* pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::CONTACT_NORMAL);

    if (SipStack::IsValidHeader(pSipHdr))
    {
        pContact = SipStack::CloneHeader(pSipHdr);
    }

    SipStack::FreeHeaderEx(pSipHdr);

    if (pContact == IMS_NULL)
    {
        // no-op
        return;
    }

    m_objPendingRemoteTargets.Append(new PendingRemoteTarget(strKey, pContact));

    IMS_TRACE_D("PendingRemoteTarget(add): key=%s, count=%d",
            SipDebug::GetCharA1(strKey.GetStr(), 8, '@'), m_objPendingRemoteTargets.GetSize(), 0);
}

// REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
PRIVATE
void SipDialogState::RemoveAllPendingRemoteTargets()
{
    if (!m_objPendingRemoteTargets.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objPendingRemoteTargets.GetSize(); ++i)
        {
            PendingRemoteTarget* pRemoteTarget = m_objPendingRemoteTargets.GetAt(i);

            if (pRemoteTarget != IMS_NULL)
            {
                delete pRemoteTarget;
            }
        }

        m_objPendingRemoteTargets.Clear();
    }
}

// REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
PRIVATE
void SipDialogState::RemovePendingRemoteTarget(IN const SipMessageInfo& objMsgInfo)
{
    if (m_objPendingRemoteTargets.IsEmpty())
    {
        // no-op
        return;
    }

    AString strKey = SipStack::GetViaBranchParameter(objMsgInfo.GetMessage());

    if (strKey.GetLength() == 0)
    {
        // no-op
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objPendingRemoteTargets.GetSize(); ++i)
    {
        PendingRemoteTarget* pRemoteTarget = m_objPendingRemoteTargets.GetAt(i);

        if (pRemoteTarget == IMS_NULL)
        {
            continue;
        }

        if (strKey.Equals(pRemoteTarget->strKey))
        {
            m_objPendingRemoteTargets.RemoveAt(i);
            delete pRemoteTarget;

            IMS_TRACE_D("PendingRemoteTarget(remove): key=%s, count=%d",
                    SipDebug::GetCharA1(strKey.GetStr(), 8, '@'),
                    m_objPendingRemoteTargets.GetSize(), 0);
            break;
        }
    }
}

// REMOTE_TARGET_UPDATE_FROM_MID_DIALOG_REQUEST
PRIVATE
void SipDialogState::UpdateAndRemovePendingRemoteTarget(IN const SipMessageInfo& objMsgInfo)
{
    if (m_objPendingRemoteTargets.IsEmpty())
    {
        // no-op
        return;
    }

    AString strKey = SipStack::GetViaBranchParameter(objMsgInfo.GetMessage());

    if (strKey.GetLength() == 0)
    {
        // no-op
        return;
    }

    PendingRemoteTarget* pRemoteTarget = IMS_NULL;
    IMS_UINT32 nMatchedIndex = 0;

    for (IMS_UINT32 i = 0; i < m_objPendingRemoteTargets.GetSize(); ++i)
    {
        pRemoteTarget = m_objPendingRemoteTargets.GetAt(i);

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

    if ((pRemoteTarget != IMS_NULL) && (pRemoteTarget->pSipHdr != IMS_NULL))
    {
        SipStack::FreeHeaderEx(m_pRemoteTargetUri);
        m_pRemoteTargetUri = SipStack::CopyHeader(pRemoteTarget->pSipHdr);
    }

    if (pRemoteTarget != IMS_NULL)
    {
        if (nMatchedIndex == (m_objPendingRemoteTargets.GetSize() - 1))
        {
            RemoveAllPendingRemoteTargets();
        }
        else
        {
            IMS_SINT32 nCount = static_cast<IMS_SINT32>(nMatchedIndex + 1);

            while (nCount > 0)
            {
                pRemoteTarget = m_objPendingRemoteTargets.GetAt(0);
                m_objPendingRemoteTargets.RemoveAt(0);

                if (pRemoteTarget != IMS_NULL)
                {
                    delete pRemoteTarget;
                }

                --nCount;
            }
        }

        IMS_TRACE_D("PendingRemoteTarget(update&remove): key=%s, index=%d, count=%d",
                SipDebug::GetCharA1(strKey.GetStr(), 8, '@'), nMatchedIndex,
                m_objPendingRemoteTargets.GetSize());
    }
}

PRIVATE
void SipDialogState::SetFromChangeOption(IN IMS_SINT32 nOption)
{
    IMS_BOOL bChanged = ((m_nFromChangeOption & nOption) != nOption);

    m_nFromChangeOption |= nOption;

    if (bChanged)
    {
        IMS_TRACE_D("DialogState: from-change=%02x", m_nFromChangeOption, 0, 0);
    }
}

PRIVATE
void SipDialogState::UpdateFromChangeOption(IN const SipMessageInfo& objMsgInfo)
{
    const SipMethod& objMethod = objMsgInfo.GetMethod();

    if (!objMethod.Equals(SipMethod::INVITE))
    {
        // no-op
        return;
    }

    ::SipMessage* pSipMsg = objMsgInfo.GetMessage();

    if ((GetState() == SipDState::STATE_INIT) && SipStack::IsRequestMessage(pSipMsg))
    {
        if (SipStack::IsOptionSupported(pSipMsg, Sip::STR_FROM_CHANGE))
        {
            SetFromChangeOption(FROM_CHANGE_ON_INVITE_REQUEST);
        }
    }
    else if ((GetState() != SipDState::STATE_CONFIRMED) && !SipStack::IsRequestMessage(pSipMsg))
    {
        IMS_SINT32 nStatusCode = SipStack::GetStatusCode(pSipMsg);

        if (SipStatusCode::IsProvisional(nStatusCode) || SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            if (SipStack::IsOptionSupported(pSipMsg, Sip::STR_FROM_CHANGE))
            {
                SetFromChangeOption(FROM_CHANGE_ON_INVITE_RESPONSE);
            }
        }
    }
}

PRIVATE GLOBAL IMS_SINT32 SipDialogState::CompareHeaders(IN SipHeaderBase* pNewH,
        IN SipHeaderBase* pExistingH, IN IMS_BOOL bToTagLenient, IN IMS_SINT32 nForkedMessage)
{
    IMS_BOOL bToTagComparison = IMS_FALSE;
    IMS_SINT32 nComparisonResult = NOT_MATCHED;

    if ((pNewH == IMS_NULL) || (pExistingH == IMS_NULL))
    {
        return NOT_MATCHED;
    }

    // Compare the tags which are sufficient for RFC 3261 compliant UAs
    AString strNewTag = SipStack::GetParameter(pNewH, Sip::STR_TAG);
    AString strExistingTag = SipStack::GetParameter(pExistingH, Sip::STR_TAG);

    if ((SipStack::GetHeaderType(pNewH) == ISipHeader::TO) &&
            (SipStack::GetHeaderType(pExistingH) == ISipHeader::TO))
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
                    {
                        nComparisonResult = MATCHED_DIFFERENT;
                    }
                    else
                    {
                        nComparisonResult = MATCHED;
                    }
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
            {
                nComparisonResult = MATCHED;
            }
            else
            {
                nComparisonResult = NOT_MATCHED;
            }
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
            {
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
    if ((nComparisonResult == NOT_MATCHED) &&
            ((nForkedMessage == FORKED_SUBSCRIBE) ||
                    ((SipStack::GetHeaderType(pNewH) == ISipHeader::FROM) &&
                            (SipStack::GetHeaderType(pExistingH) == ISipHeader::TO))))
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
                if ((SipStack::GetHeaderType(pNewH) == ISipHeader::TO) &&
                        (SipStack::GetHeaderType(pExistingH) == ISipHeader::FROM))
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

PRIVATE GLOBAL IMS_BOOL SipDialogState::IsTargetRefreshMessage(IN ::SipMessage* pSipMsg)
{
    SipMethod objMethod = SipStack::GetMethod(pSipMsg);

    if (objMethod.Equals(SipMethod::INVALID))
    {
        return IMS_FALSE;
    }

    // Check if the method is a remote target refreshable one or not
    if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::UPDATE) &&
            !objMethod.Equals(SipMethod::SUBSCRIBE) && !objMethod.Equals(SipMethod::NOTIFY) &&
            !objMethod.Equals(SipMethod::REFER))
    {
        return IMS_FALSE;
    }

    if (!SipStack::IsRequestMessage(pSipMsg))
    {
        IMS_SINT32 nStatusCode = SipStack::GetStatusCode(pSipMsg);

        if (!SipStatusCode::IsProvisional(nStatusCode) &&
                !SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            return IMS_FALSE;
        }

        if (SipStatusCode::IsProvisional(nStatusCode))
        {
            // Check if the message is a reliable provisional response or not
            if (SipStack::IsMessageRpr(pSipMsg) != IMS_TRUE)
            {
                // Check if the message has a To-Tag or not
                SipHeaderBase* pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::TO);

                // Get To-Tag parameter from To header
                AString strToTag = SipStack::GetParameter(pSipHdr, Sip::STR_TAG);
                SipStack::FreeHeaderEx(pSipHdr);

                if (strToTag.GetLength() == 0)
                {
                    return IMS_FALSE;
                }
            }
        }
    }

    return IMS_TRUE;
}
