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

#include "ServiceTrace.h"
#include "SipAddress.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcCallContext.h"
#include "call/ParticipantInfo.h"
#include "conferencecall/ExpandController.h"
#include "conferencecall/ConferenceConfigurationWrapper.h"
#include "conferencecall/IConferenceReference.h"
#include "IMtcContext.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ExpandController::ExpandController(IN CallKey nConfCallKey, IMtcContext& objContext,
        IN CallConnectionIdManager& objConnectionIdManager) :
        ConferenceController(nConfCallKey, objContext, objConnectionIdManager)
{
    IMS_TRACE_I("+ExpandController", 0, 0, 0);
}

PUBLIC VIRTUAL ExpandController::~ExpandController()
{
    IMS_TRACE_I("~ExpandController", 0, 0, 0);
}

PUBLIC VIRTUAL void ExpandController::OnCallUpdated(IN IMS_UINT32 nType, IN IMS_UINTP nCallKey)
{
    // TODO: session updated...
    // if (ntype != SESSION_UPDATED)
    if (IMS_FALSE)
    {
        return ConferenceController::OnCallUpdated(nType, nCallKey);
    }

    if (nCallKey != m_nConfCallKey)
    {
        return;
    }

    if ((ConferenceConfigurationWrapper::GetReferTypeForInvite() == REFER_INVITE_SINGLE) ||
            GetState() != STATE_EXPANDING)
    {
        return;
    }

    // Session is updated to conference during EXPANDING state - LGU+
    // expanded_by will be notified via LGUPUCSession::StateCONVERSATION_Updated()

    // Add the user of exist 1-to-1 session to participant list
    IMS_TRACE_D("Updated : Add user of the exist 1-to-1 session", 0, 0, 0);

    ConfUser* p1to1User = new ConfUser();
    SipAddress objSIPAddress(
            GetConferenceCall()->GetCallContext().GetParticipantInfo().GetRemoteUri());
    p1to1User->aStrTarget = objSIPAddress.GetUserInfoPart()->GetUser();

    m_objParticipantList.AddUser(p1to1User);
    m_objParticipantList.Login();

    CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_REFER_INVITE);
    SetState(STATE_IDLE);
}

PUBLIC VIRTUAL void ExpandController::OnReferenceStarted(IN IConferenceReference* piConfRef)
{
    IMS_TRACE_D("OnReferenceStarted", 0, 0, 0);
    // LGU+
    if ((ConferenceConfigurationWrapper::IsReferSubscriptionRequired() == IMS_FALSE) &&
            (GetState() == STATE_EXPANDING) && (piConfRef->GetType() == REFERENCE_TYPE_INVITE))
    {
        m_objNotifier.NotifyExpanded();
    }
    else
    {
        ConferenceController::OnReferenceStarted(piConfRef);
    }
}

PUBLIC VIRTUAL void ExpandController::OnReferenceStartFailed(IN IConferenceReference* piConfRef)
{
    IMS_TRACE_D("OnReferenceStartFailed", 0, 0, 0);

    if (piConfRef->GetType() != REFERENCE_TYPE_INVITE)
    {
        return ConferenceController::OnReferenceStartFailed(piConfRef);
    }

    StopFinalSipfragWaitTimer();

    ConfUser* pTempUser = m_objParticipantList.GetConfUser(piConfRef);
    if (pTempUser != IMS_NULL)
    {
        pTempUser->eStatus = CONFINFO_STATUS_FAIL;
    }

    Recover();
    CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_REFER_INVITE, pTempUser);
    RemoveReference(piConfRef);

    if (GetState() == STATE_EXPANDING)
    {
        IMS_TRACE_D("Close Controller - because of failure expanding to conference", 0, 0, 0);
        SendClosed();
    }

    if ((GetState() == STATE_JOINING) && (m_objIConfReferences.GetSize() <= 0))
    {
        m_objOperationQueue.CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, IMS_TRUE);
    }
}

PUBLIC VIRTUAL void ExpandController::OnReferenceUpdated(IN IConferenceReference* piConfRef,
        IN SipStatusCode nSipFragCode, IN ReferSubscriptionState eState)
{
    IMS_TRACE_D("OnReferenceUpdated : R-NOTIFY is received.", 0, 0, 0);

    if (piConfRef->GetType() != REFERENCE_TYPE_INVITE)
    {
        return ConferenceController::OnReferenceUpdated(piConfRef, nSipFragCode, eState);
    }

    ConfUser* pTempUser = m_objParticipantList.GetConfUser(piConfRef);
    UpdateUserStatusByReferResult(pTempUser, piConfRef, nSipFragCode);

    if (SipStatusCode::IsFinalSuccess(nSipFragCode.ToInt()))
    {
        StopFinalSipfragWaitTimer();

        if (GetState() == STATE_EXPANDING)
        {
            StopMedia1to1Session();
        }

        if (GetState() == STATE_JOINING)
        {
            NotifyUsersInfo();
        }
        else
        {
            CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_REFER_INVITE, pTempUser);
        }
    }
    else if (SipStatusCode::IsFinalFailure(nSipFragCode.ToInt()))
    {
        StopFinalSipfragWaitTimer();

        if (GetState() == STATE_EXPANDING)
        {
            Recover();
            SendClosed();
        }
        else
        {
            NotifyUsersInfo();
        }
    }
    else if (SipStatusCode::Is1XX(nSipFragCode.ToInt()) && (GetState() == STATE_EXPANDING))
    {
        CheckNStartFinalSipfragWaitTimer(CONDITION_SIPFRAG_100_RECEIVED);
    }

    if (eState == ReferSubscriptionState::TERMINATED)
    {
        RemoveReference(piConfRef);
    }

    if ((GetState() == STATE_JOINING) && (m_objIConfReferences.GetSize() <= 0))
    {
        m_objOperationQueue.CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, IMS_TRUE);
    }
}

PROTECTED
void ExpandController::ProcessExpand(IN IMSList<ConfUser*>& objUsers)
{
    IMS_TRACE_I("ProcessExpand", 0, 0, 0);

    if (GetState() != STATE_CREATED)
    {
        m_objNotifier.NotifyExpandFailed(CallReasonInfo(CODE_UNSPECIFIED, -1));
        return;
    }

    SetState(STATE_EXPANDING);
    IMS_UINT32 nStartIndex = AddUserToParticipantList(objUsers);
    ClearListForConfUsers(objUsers);

    IMS_SINT32 nReferType = ConferenceConfigurationWrapper::GetReferTypeForInvite();

    if (nReferType == REFER_INVITE_SINGLE)  // SKT
    {
        m_objOperationQueue.CreateNPut(CONTROL_OPERATION_CREATE_CONFERENCE_SESSION, objUsers);
        m_objOperationQueue.CreateNPut(CONTROL_OPERATION_SUBSCRIBE);
        m_objOperationQueue.CreateNPut(CONTROL_OPERATION_REFER_INVITE,
                m_objParticipantList.GetConfUsers().GetAt(nStartIndex));

        // Terminate the exist 1-to-1 session : it is triggered by GII operation.
        m_objOperationQueue.CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI);
    }
    else if (nReferType == REFER_INVITE_MULTIPLE)  // LGU+
    {
        m_objOperationQueue.CreateNPut(
                CONTROL_OPERATION_REFER_INVITE, m_objParticipantList.GetConfUsers());
        m_objOperationQueue.CreateNPut(CONTROL_OPERATION_SUBSCRIBE);
    }

    m_objOperationQueue.SetAddingOperationSetCompleted();
}

PUBLIC VIRTUAL void ExpandController::StartConferenceCall(
        IN ConferenceOperationQueue::ConferenceOperation* pOperation)
{
    // TODO: how to check nullcall? never be null so no need to check?
    /*
    if (piCall is null Call)
    {
        delete pParams;
        Recover();
        SendClosed();
    }
    */

    GetConferenceCall()->StartConference(
            CallType::VOIP, AString::ConstNull(), pOperation->GetUsers());
}

PROTECTED VIRTUAL IMS_BOOL ExpandController::IsStartFinalSipfragWaitTimer() const
{
    if (GetState() != STATE_EXPANDING)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("IsStartFinalSipfragWaitTimer : [%d]", m_nConditionFinalSipfragTimer, 0, 0);

    if (IsConditionMet(CONDITION_SIPFRAG_100_RECEIVED) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    if (IsConditionMet(CONDITION_1TO1_TERMINATED) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void ExpandController::Recover()
{
    IMS_TRACE_I("Recover", 0, 0, 0);
    switch (m_objOperationQueue.GetTypeOfCurrentOperation())
    {
        case CONTROL_OPERATION_CREATE_CONFERENCE_SESSION:
            RecoverOnCreating();
            break;
        case CONTROL_OPERATION_SUBSCRIBE:
            break;
        case CONTROL_OPERATION_REFER_INVITE:
            RecoverOnReferring();
            break;

        default:
            IMS_TRACE_I("Recover : not handled.", 0, 0, 0);
            break;
    }
}

PROTECTED VIRTUAL void ExpandController::UpdateUserStatusByReferResult(IN ConfUser* pUser,
        IN IConferenceReference* piConfRef,
        IN SipStatusCode nStatusCode /* = SipStatusCode::SC_200*/)
{
    if (ConferenceConfigurationWrapper::IsReferSubscriptionRequired() &&
            (GetState() == STATE_JOINING) && (piConfRef->GetType() == REFERENCE_TYPE_INVITE) &&
            SipStatusCode::IsFinalFailure(nStatusCode.ToInt()))
    {
        switch (nStatusCode.ToInt())
        {
            case SipStatusCode::SC_400:
            case SipStatusCode::SC_503:
                pUser->eStatus = CONFINFO_STATUS_SERVERERROR;
                break;
            case SipStatusCode::SC_403:
                pUser->eStatus = CONFINFO_STATUS_FORBIDDEN;
                break;
            case SipStatusCode::SC_404:
            case SipStatusCode::SC_415:
                pUser->eStatus = CONFINFO_STATUS_NOTSUPPORTED;
                break;
            case SipStatusCode::SC_408:
                pUser->eStatus = CONFINFO_STATUS_NOANSWER;
                break;
            case SipStatusCode::SC_480:
                pUser->eStatus = CONFINFO_STATUS_LOWBATTERY;
                break;
            case SipStatusCode::SC_486:
                pUser->eStatus = CONFINFO_STATUS_BUSY;
                break;
            case SipStatusCode::SC_499:
                pUser->eStatus = CONFINFO_STATUS_NOTREACHABLE;
                break;
            case SipStatusCode::SC_500:
                pUser->eStatus = CONFINFO_STATUS_INTSERVERERROR;
                break;
            case SipStatusCode::SC_603:
                pUser->eStatus = CONFINFO_STATUS_REJECT;
                break;
            case SipStatusCode::SC_606:
                pUser->eStatus = CONFINFO_STATUS_NOTACCEPTABLE;
                break;

            default:
                pUser->eStatus = CONFINFO_STATUS_FAIL;
                break;
        }

        pUser->eStatusCode = nStatusCode.ToInt();
    }
    else
    {
        return ConferenceController::UpdateUserStatusByReferResult(pUser, piConfRef, nStatusCode);
    }

    IMS_TRACE_D("UpdateUserStatusByReferResult : [%s][%d]", pUser->aStrTarget.GetStr(),
            pUser->eStatusCode, 0);
}

PROTECTED VIRTUAL void ExpandController::NotifyCmdResult()
{
    IMS_SINT32 nOldState = GetState();
    ConferenceController::NotifyCmdResult();

    if (nOldState == STATE_EXPANDING)
    {
        IMS_TRACE_D("NotifyCmdResult : Expanding conf. is completed, join new members", 0, 0, 0);
        ProcessJoinAfterExpand();

        // TODO: control media manager.... params
        // m_objConfCallContext.GetMediaManager().Run();
    }
}

PRIVATE
void ExpandController::StopMedia1to1Session()
{
    if (ConferenceConfigurationWrapper::GetReferTypeForInvite() != REFER_INVITE_SINGLE)
    {
        return;
    }

    // TODO: control media manager.
    // GetConferenceCall()->GetCallContext().GetMediaManager().SetLocalTone(IMS_FALSE);
    // TODO: check if this is still required. No explicit 'stop' is needed.
    // 1to1 calls.GetMediaManager().();
}

PRIVATE
void ExpandController::Resume1to1Session()
{
    // TODO: how to control??
    // for (IMS_UINT32 index = 0; index < m_objCallManager->GetNum(); index++)
    // {
    //     IMtcCall* pSession = m_objCallManager->GetAt(index);
    //     if ((pSession != IMS_NULL) && (pSession != GetConferenceCall()))
    //     {
    //         pSession->GetMediaMngr()->GetMediaUtil()->ResumeVT();
    //         IMS_TRACE_D("Resume1to1Session : turn back to the existed 1-to-1 session", 0, 0, 0);
    //         return;
    //     }
    // }
}

PRIVATE
void ExpandController::ProcessJoinAfterExpand()
{
    // Invite other participants when conference session is expanded.
    if (IsReadyToPerformCmd() == IMS_FALSE)
    {
        m_objNotifier.NotifyJoinFailed(CallReasonInfo(CODE_UNSPECIFIED, -1), m_objParticipantList);
    }

    SetState(STATE_JOINING);

    IMS_UINT32 nStartIndex = 0;

    for (IMS_UINT32 i = 0; i < m_objParticipantList.GetSize(); i++)
    {
        ConfUser* pConfUser = m_objParticipantList.GetConfUsers().GetAt(i);
        if (pConfUser->eStatus == CONFINFO_STATUS_IDLE)
        {
            nStartIndex = i;
            break;
        }
    }

    for (IMS_UINT32 i = nStartIndex; i < m_objParticipantList.GetSize(); i++)
    {
        ConfUser* pConfUser = m_objParticipantList.GetConfUsers().GetAt(i);
        IMS_TRACE_D("ProcessJoinAfterExpand : user [%s]", pConfUser->aStrTarget.GetStr(), 0, 0);
        m_objOperationQueue.CreateNPut(CONTROL_OPERATION_REFER_INVITE, pConfUser);
    }

    m_objOperationQueue.SetAddingOperationSetCompleted();
}

PRIVATE
void ExpandController::RecoverOnCreating()
{
    if (GetState() != STATE_EXPANDING)
    {
        IMS_TRACE_D("RecoverOnCreating : nothing to be handled", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("RecoverOnCreating", 0, 0, 0);

    m_objNotifier.NotifyExpandFailed(CallReasonInfo(CODE_UNSPECIFIED, -1));
    m_objOperationQueue.Clear();
    SetState(STATE_IDLE);

    Resume1to1Session();
}

PRIVATE
void ExpandController::RecoverOnReferring()
{
    IMS_TRACE_D("RecoverOnReferring : state[%d]", GetState(), 0, 0);

    if (GetState() == STATE_EXPANDING)
    {
        m_objNotifier.NotifyExpandFailed(CallReasonInfo(CODE_UNSPECIFIED, -1));
        m_objOperationQueue.Clear();
        SetState(STATE_IDLE);

        if (ConferenceConfigurationWrapper::GetReferTypeForInvite() == REFER_INVITE_SINGLE)
        {
            Resume1to1Session();
            GetConferenceCall()->Terminate(CallReasonInfo(CODE_UNSPECIFIED, -1));
        }

        return;
    }

    IMSList<ConfUser*> objConfUsers = m_objOperationQueue.GetUsersOfCurrentOperation();

    for (IMS_UINT32 nIndex = 0; nIndex < objConfUsers.GetSize(); nIndex++)
    {
        ConfUser* pConfUser = objConfUsers.GetAt(nIndex);
        pConfUser->eStatus = CONFINFO_STATUS_FAIL;
    }

    NotifyUsersInfo();
}
