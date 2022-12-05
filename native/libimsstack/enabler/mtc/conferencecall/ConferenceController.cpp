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

#include "ICoreService.h"
#include "IMessage.h"
#include "IMtcService.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "IuMtcCall.h"
#include "IuMtcService.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcSession.h"
#include "conferencecall/CallConnectionIdManager.h"
#include "conferencecall/ConferenceConfigurationWrapper.h"
#include "conferencecall/ConferenceController.h"
#include "conferencecall/ConferenceFactory.h"
#include "conferencecall/ConferenceReference.h"
#include "helper/ICallStateProxy.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ConferenceController::ConferenceController(IN CallKey nConfCallKey, IMtcContext& objContext,
        IN CallConnectionIdManager& objConnectionIdManager, IN ConferenceFactory& objFactory) :
        m_nConfCallKey(nConfCallKey),
        m_objContext(objContext),
        m_objCallManager(objContext.GetCallManager()),
        m_objConnectionIdManager(objConnectionIdManager),
        m_objFactory(objFactory),
        m_objParticipantList(*objFactory.CreateParticipantList()),
        m_objNotifier(*objFactory.CreateEventNotifier(
                GetConferenceCall()->GetCallContext(), objConnectionIdManager)),
        m_objOperationQueue(*objFactory.CreateOperationQueue()),
        m_pSubscription(IMS_NULL),
        m_objIConfReferences(IMSList<IConferenceReference*>()),
        m_piTimer(IMS_NULL),
        m_nConditionFinalSipfragTimer(CONDITION_NONE),
        m_nState(STATE_CREATED)
{
    IMS_TRACE_I("+ConferenceController", 0, 0, 0);

    Init();
}

PUBLIC VIRTUAL ConferenceController::~ConferenceController()
{
    IMS_TRACE_I("~ConferenceController", 0, 0, 0);

    m_objContext.GetCallStateProxy().RemoveListener(this);

    delete m_pSubscription;

    for (IMS_UINT32 i = 0; i < m_objIConfReferences.GetSize(); i++)
    {
        delete m_objIConfReferences.GetAt(i);
    }
    m_objIConfReferences.Clear();

    if (m_piTimer != IMS_NULL)
    {
        m_piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer);
    }

    // ParticipantList must be deleted after ConferenceSubscription is deleted.
    delete &m_objParticipantList;
    delete &m_objOperationQueue;
    delete &m_objNotifier;
}

PUBLIC VIRTUAL void ConferenceController::OnCallStateChanged(
        IN CallKey nCallKey, IN State eState, IN Type, IN IMS_BOOL, IN IMS_SINT32)
{
    switch (eState)
    {
        case State::ALERTING:
        case State::IDLE:
        case State::INCOMING:
        case State::OUTGOING:
        case State::UPDATING:
            break;
        case State::ESTABLISHED:
            // TODO: how to distinguish 'started' / 'updated'
            OnCallUpdated(CALL_STARTED, nCallKey);
            // OnCallUpdated(CALL_UPDATED, nCallKey);
            break;
        case State::TERMINATING:
            // TODO: how to distinguish 'start failed' / 'terminated'...
            OnCallUpdated(CALL_TERMINATED, nCallKey);
            // OnCallUpdated(CALL_STARTFAILED, nCallKey);
            break;
        default:
            break;
    }
}

PUBLIC VIRTUAL void ConferenceController::OnSubscriptionUpdated(IN SubscriptionUpdateType eType)
{
    IMS_TRACE_D("OnSubscriptionUpdated : type=[%d]", eType, 0, 0);

    switch (eType)
    {
        case SubscriptionUpdateType::SUCCEEDED:
            CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_SUBSCRIBE);
            break;
        case SubscriptionUpdateType::FAILED:
            CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_SUBSCRIBE);
            // do something???
            delete m_pSubscription;
            m_pSubscription = IMS_NULL;
            break;
        case SubscriptionUpdateType::UNSUBSCRIBED:
            CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_UNSUBSCRIBE);
            break;
        case SubscriptionUpdateType::TERMINATED:
            CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_UNSUBSCRIBE);  // to stop timer
            m_objOperationQueue.CreateNPutWithReason(
                    CONTROL_OPERATION_TERMINATE_CONFERENCE, CODE_NONE, IMS_TRUE);
            break;
        case SubscriptionUpdateType::NOTIFY_RECEIVED:
            if (m_objParticipantList.GetSize() <= 0)
            {
                break;
            }
            else if (m_objParticipantList.GetConnectedParticipantSize(IMS_TRUE) == 0)
            {
                IMS_TRACE_D("OnSubscriptionUpdated terminate conference by alone", 0, 0, 0);
                m_objOperationQueue.CreateNPutWithReason(
                        CONTROL_OPERATION_TERMINATE_CONFERENCE, CODE_USER_TERMINATED, IMS_TRUE);
            }

            NotifyConferenceInfo();
            NotifyUsersInfo();
            // TODO: check user entity status.
            CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_CHECK_CONNECTED);
            break;

        default:
            break;
    }
}

PUBLIC VIRTUAL void ConferenceController::OnReferenceStarted(IN IConferenceReference* piConfRef)
{
    IMS_TRACE_D("OnReferenceStarted", 0, 0, 0);

    if (piConfRef->GetType() == REFERENCE_TYPE_BYE)
    {
        ConfUser* pTempUser = m_objParticipantList.GetConfUser(piConfRef);
        UpdateUserStatusByReferResult(pTempUser, piConfRef);
        CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_REFER_BYE, pTempUser);
    }
    else if (piConfRef->GetType() == REFERENCE_TYPE_INVITE)
    {
        if (GetState() == STATE_JOINING)
        {
            ConfUser* pTempUser = IMS_NULL;
            if (ConferenceConfigurationWrapper::IsReferSubscriptionRequired())
            {
                pTempUser = m_objParticipantList.GetConfUser(piConfRef);
            }

            CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_REFER_INVITE, pTempUser);
        }
    }
}

PUBLIC VIRTUAL void ConferenceController::OnReferenceStartFailed(IN IConferenceReference* piConfRef)
{
    IMS_TRACE_D("OnReferenceStartFailed", 0, 0, 0);

    StopFinalSipfragWaitTimer();

    if (piConfRef->GetType() == REFERENCE_TYPE_BYE)
    {
        m_objNotifier.NotifyDropFailed(CallReasonInfo(CODE_NONE), m_objParticipantList);
        m_objOperationQueue.Clear();
        SetState(STATE_IDLE);
    }
    else
    {
        Recover();

        // remove participant in local when REFER is rejected case only. (not R-NOTIFY case)
        // this must be called after Recover()
        m_objParticipantList.RemoveUser(m_objParticipantList.GetConfUser(piConfRef));
    }

    RemoveReference(piConfRef);
}

PUBLIC VIRTUAL void ConferenceController::OnReferenceUpdated(IN IConferenceReference* piConfRef,
        IN SipStatusCode nSipFragCode, IN ReferSubscriptionState eState)
{
    // TODO: separate functions. : HandleSuccessSipFrag / HandleFailureSipFrag
    // ExpandController to override them.

    IMS_TRACE_D("OnReferenceUpdated : R-NOTIFY is received.", 0, 0, 0);

    // TODO: check confref is in the references list.

    if (piConfRef->GetType() == REFERENCE_TYPE_INVITE)
    {
        ConfUser* pTempUser = m_objParticipantList.GetConfUser(piConfRef);
        UpdateUserStatusByReferResult(pTempUser, piConfRef, nSipFragCode);

        if (SipStatusCode::IsFinalSuccess(nSipFragCode.ToInt()))
        {
            StopFinalSipfragWaitTimer();
            CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_REFER_INVITE, pTempUser);
        }
        else if (SipStatusCode::IsFinalFailure(nSipFragCode.ToInt()))
        {
            StopFinalSipfragWaitTimer();
            Recover();
        }
        else if (SipStatusCode::Is1XX(nSipFragCode.ToInt()))
        {
            CheckNStartFinalSipfragWaitTimer(CONDITION_SIPFRAG_100_RECEIVED);
        }
    }
    else
    {
        ConfUser* pTempUser = m_objParticipantList.GetConfUser(piConfRef);
        UpdateUserStatusByReferResult(pTempUser, piConfRef, nSipFragCode);
        // to check already handled by OnReferenceStarted
        CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_REFER_BYE, pTempUser);
    }

    if (eState == ReferSubscriptionState::TERMINATED)
    {
        RemoveReference(piConfRef);
    }
}

PUBLIC VIRTUAL void ConferenceController::Timer_TimerExpired(IN ITimer* piTimer)
{
    IMS_TRACE_D("Timer_TimerExpired", 0, 0, 0);

    if (m_piTimer == piTimer)
    {
        Recover();
    }
}

PUBLIC VIRTUAL void ConferenceController::ProcessCommand(IN IMS_UINT32 nCmd,
        IN IMSList<ConfUser*>& objUsers, IN CallInfo& objCallInfo, IN MediaInfo& objMediaInfo,
        IN IMSMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_D("ProcessCommand : [%d]", nCmd, 0, 0);

    switch (nCmd)
    {
        case IuMtcCall::STARTCONF:
            ProcessGroupCall(objUsers, objCallInfo, objMediaInfo, objSuppServices);
            break;

        default:
            IMS_TRACE_E(0, "invalid cmd. why here!!?", 0, 0, 0);
            break;
    }
}

PUBLIC VIRTUAL void ConferenceController::ProcessCommand(
        IN IMS_UINT32 nCmd, IN IMSList<ConfUser*>& objUsers)
{
    IMS_TRACE_D("ProcessCommand : [%d]", nCmd, 0, 0);

    switch (nCmd)
    {
        case IConferenceController::EXPAND:
            ProcessExpand(objUsers);
            break;
        case IConferenceController::MERGE:
            ProcessMerge(objUsers);
            break;
        case IConferenceController::ADD:
            ProcessJoin(objUsers);
            break;
        case IConferenceController::REMOVE:
            ProcessDrop(objUsers);
            break;
        case IConferenceController::JOINED:  // participant case
            ProcessSubscribeOnParticipant();
            break;

        default:
            IMS_TRACE_E(0, "invalid cmd. why here!!?", 0, 0, 0);
            break;
    }
}

PUBLIC VIRTUAL IMS_SINT32 ConferenceController::GetState() const
{
    return m_nState;
}

PUBLIC VIRTUAL IndividualCallState ConferenceController::GetCallStatusInConference(
        IN CallKey nKey) const
{
    if (m_nConfCallKey == nKey)
    {
        IMS_TRACE_D("GetCallStatusInConference : Host Call", 0, 0, 0);
        return IndividualCallState::HOST;
    }

    IMtcCall* piConfCall = GetConferenceCall();
    if (piConfCall->GetKey() == IMtcCall::CALL_KEY_INVALID)
    {
        IMS_TRACE_D("GetCallStatusInConference - Destroyed Host call", 0, 0, 0);
        return IndividualCallState::IDLE;
    }

    if (m_objOperationQueue.GetTypeOfCurrentOperation() == CONTROL_OPERATION_REFER_INVITE)
    {
        const IMSList<ConfUser*>& objUsers = m_objOperationQueue.GetCurrentOperation()->GetUsers();
        for (IMS_UINT32 i = 0; i < objUsers.GetSize(); i++)
        {
            if (nKey == m_objConnectionIdManager.GetCallKey(objUsers.GetAt(i)->nConnectionId))
            {
                IMS_TRACE_D("GetCallStatusInConference : Joining Call", 0, 0, 0);
                return IndividualCallState::JOINING;
            }
        }
    }

    if (m_objOperationQueue.GetTypeOfCurrentOperation() == CONTROL_OPERATION_TERMINATE_1TO1_CALL)
    {
        if (nKey ==
                m_objConnectionIdManager.GetCallKey(
                        m_objOperationQueue.GetCurrentOperation()->GetConnectionId()))
        {
            IMS_TRACE_D("GetCallStatusInConference : Joined and Terminating Call", 0, 0, 0);
            return IndividualCallState::JOINED;
        }
    }

    for (IMS_UINT32 i = 0; i < m_objParticipantList.GetSize(); i++)
    {
        ConfUser* pConfUser = m_objParticipantList.GetConfUser(i);
        if (pConfUser->eStatus == STATUS_DISCONNECTED)
        {
            // TODO: check if removing ConfUser when it's disconnected is okay.
            // Otherwise, just update the connection id of the disconnected ConfUser
            continue;
        }

        if (nKey == m_objConnectionIdManager.GetCallKey(pConfUser->nConnectionId))
        {
            IMS_TRACE_D("GetCallStatusInConference Invited Call key[%d] connectionid[%d]", nKey,
                    pConfUser->nConnectionId, 0);
            return IndividualCallState::INVITED;
        }
    }

    IMS_TRACE_D("GetCallStatusInConference : Idle", 0, 0, 0);
    return IndividualCallState::IDLE;
}

PUBLIC VIRTUAL void ConferenceController::OnOperationReady()
{
    IMS_TRACE_I("OnOperationReady", 0, 0, 0);
    DoNextOperation();
}

PROTECTED VIRTUAL void ConferenceController::ProcessJoin(IN IMSList<ConfUser*>& objUsers)
{
    IMS_TRACE_I("ProcessJoin", 0, 0, 0);

    IMS_UINT32 nStartIndex = AddUserToParticipantList(objUsers);
    ClearListForConfUsers(objUsers);

    if (IsReadyToPerformCmd() == IMS_FALSE)
    {
        m_objNotifier.NotifyJoinFailed(
                CallReasonInfo(CODE_LOCAL_ILLEGAL_STATE, -1), m_objParticipantList);
        return;
    }

    SetState(STATE_JOINING);
    IMS_SINT32 nReferType = ConferenceConfigurationWrapper::GetReferTypeForInvite();

    if (nReferType == REFER_INVITE_SINGLE)
    {
        for (IMS_UINT32 i = nStartIndex; i < m_objParticipantList.GetSize(); i++)
        {
            m_objOperationQueue.CreateNPutWithUser(
                    CONTROL_OPERATION_REFER_INVITE, m_objParticipantList.GetConfUsers().GetAt(i));
        }
    }
    else if (nReferType == REFER_INVITE_MULTIPLE)  // send REFER with resource list
    {
        IMSList<ConfUser*> objJoinList;
        for (IMS_UINT32 i = nStartIndex; i < m_objParticipantList.GetSize(); i++)
        {
            objJoinList.Append(m_objParticipantList.GetConfUsers().GetAt(i));
        }
        m_objOperationQueue.CreateNPutWithUsers(CONTROL_OPERATION_REFER_INVITE, objJoinList);
    }

    if (ConferenceConfigurationWrapper::IsReferSubscriptionRequired() == IMS_FALSE)
    {
        m_objOperationQueue.CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI);
    }

    m_objOperationQueue.SetAddingOperationSetCompleted();
}

PROTECTED VIRTUAL void ConferenceController::ProcessDrop(IN IMSList<ConfUser*>& objUsers)
{
    IMS_TRACE_I("ProcessDrop", 0, 0, 0);

    IMS_SINT32 nIndex = m_objParticipantList.FindParticipant(objUsers.GetAt(0)->nConnectionId);
    ClearListForConfUsers(objUsers);

    if (nIndex < 0)
    {
        // cannot find the user in participant list.
        m_objNotifier.NotifyDropFailed(CallReasonInfo(CODE_NONE), m_objParticipantList);
        return;
    }

    if (IsReadyToPerformCmd() == IMS_FALSE)
    {
        m_objNotifier.NotifyDropFailed(CallReasonInfo(CODE_NONE), m_objParticipantList);
        return;
    }

    SetState(STATE_DROPPING);

    m_objOperationQueue.CreateNPutWithUser(CONTROL_OPERATION_REFER_BYE,
            m_objParticipantList.GetConfUsers().GetAt((IMS_UINT32)nIndex));
    m_objOperationQueue.CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI);
    m_objOperationQueue.SetAddingOperationSetCompleted();
}

PROTECTED
void ConferenceController::ProcessSubscribeOnParticipant()
{
    if (ConferenceConfigurationWrapper::IsSubscriptionForParticipantRequired() == IMS_FALSE)
    {
        return;
    }

    IMS_TRACE_I("ProcessSubscribeOnParticipant", 0, 0, 0);

    m_objOperationQueue.CreateNPut(CONTROL_OPERATION_SUBSCRIBE, IMS_TRUE);
}

PROTECTED
IMS_UINT32 ConferenceController::AddUserToParticipantList(
        IN IMSList<ConfUser*>& objConfUsers, IN IMS_BOOL bReOrder /* = IMS_FALSE*/)
{
    IMS_UINT32 nStartIndex = m_objParticipantList.GetSize();
    IMS_UINT32 nAddingSize = objConfUsers.GetSize();
    IMS_TRACE_D("AddUserToParticipantList start[%d] size[%d]", nStartIndex, nAddingSize, 0);

    for (IMS_UINT32 i = 0; i < nAddingSize; i++)
    {
        m_objParticipantList.AddUser(objConfUsers.GetAt(i));
    }
    IMS_TRACE_D("AddUserToParticipantList size[%d]", m_objParticipantList.GetSize(), 0, 0);

    if (bReOrder && nAddingSize > 1)
    {
        m_objParticipantList.ReOrder(m_objCallManager, m_objConnectionIdManager);  // TODO: callid
    }
    IMS_TRACE_D("AddUserToParticipantList size[%d]", m_objParticipantList.GetSize(), 0, 0);
    m_objParticipantList.Login();
    return nStartIndex;
}

PROTECTED
void ConferenceController::ClearListForConfUsers(IN IMSList<ConfUser*>& objUsers)
{
    for (IMS_UINT32 i = 0; i < objUsers.GetSize(); i++)
    {
        ConfUser* pUser = objUsers.GetAt(i);
        if (pUser != IMS_NULL)
        {
            delete pUser;
        }
    }

    objUsers.Clear();
}

#if _REAL_OPERATIONS_
#endif

PROTECTED VIRTUAL IMS_BOOL ConferenceController::CreateSubscription()
{
    IMS_TRACE_D("CreateSubscription", 0, 0, 0);

    if (m_pSubscription != IMS_NULL)
    {
        return IMS_FALSE;
    }

    m_pSubscription = m_objFactory.CreateSubscription(m_nConfCallKey, m_objParticipantList, *this);

    return IMS_TRUE;
}

PROTECTED VIRTUAL void ConferenceController::StartSubscription()
{
    IMS_TRACE_D("StartSubscription", 0, 0, 0);
    if (m_pSubscription == IMS_NULL)
    {
        return;
    }

    // dialog type is set in subscription.
    AString strTo;
    GetFocusAddress(strTo);
    IMS_RESULT nResult = m_pSubscription->Subscribe(strTo);

    if (nResult == IMS_FAILURE)
    {
        Recover();
    }

    m_objParticipantList.Login();
}

PROTECTED VIRTUAL void ConferenceController::StopSubscription()
{
    IMS_TRACE_D("StopSubscription", 0, 0, 0);
    if (m_pSubscription == IMS_NULL)
    {
        return;
    }

    if (m_pSubscription->GetState() == SubscriptionState::SUBSCRIBING ||
            m_pSubscription->GetState() == SubscriptionState::ACTIVE)
    {
        m_pSubscription->UnSubscribe();
    }
}

PROTECTED VIRTUAL IConferenceReference* ConferenceController::CreateReference(IN ConfUser* pUser)
{
    IMS_TRACE_D("CreateReference", 0, 0, 0);

    IConferenceReference* piConfRefer = m_objFactory.CreateReference(m_nConfCallKey, pUser, *this);
    m_objParticipantList.SetReference(piConfRefer, pUser);

    return piConfRefer;
}

PROTECTED VIRTUAL IConferenceReference* ConferenceController::CreateReference(
        IN IMSList<ConfUser*>& objUsers)
{
    IMS_TRACE_D("CreateReference", 0, 0, 0);

    IConferenceReference* piConfRefer =
            m_objFactory.CreateReference(m_nConfCallKey, objUsers, *this);

    for (IMS_UINT32 index = 0; index < objUsers.GetSize(); index++)
    {
        ConfUser* pUser = objUsers.GetAt(index);
        m_objParticipantList.SetReference(piConfRefer, pUser);
    }

    return piConfRefer;
}

PROTECTED
void ConferenceController::ClearOngoingReferences()
{
    IMS_UINT32 nSize = m_objIConfReferences.GetSize();
    IMS_TRACE_I("ClearOngoingReferences reference size=[%d]", nSize, 0, 0);

    for (IMS_SINT32 i = nSize - 1; i >= 0; i--)
    {
        IMS_TRACE_I("ClearOngoingReferences index=[%d]", i, 0, 0);
        IConferenceReference* piTemp = m_objIConfReferences.GetAt(i);

        if (piTemp == IMS_NULL || piTemp->GetType() == REFERENCE_TYPE_INVITE ||
                SipStatusCode::IsFinalFailure(piTemp->GetResponseCode()))
        {
            continue;
        }

        // to cover abnormal network issue.
        // REFER-BYE received 202 response but no R-NOTIFY case.
        // if the next REFER(invite) receives R-NOTIFY without event id, this R-NOTIFY is matched
        // with the previous REFER.
        m_objParticipantList.ResetReference(piTemp);
        piTemp->SetForceToTerminateInterface(IMS_TRUE);
        delete piTemp;
        m_objIConfReferences.RemoveAt(i);
    }
}

PROTECTED
void ConferenceController::RemoveReference(IN IConferenceReference* piReference)
{
    IMS_TRACE_I("RemoveReference [%" PFLS_u "]", piReference, 0, 0);

    for (IMS_UINT32 i = 0; i < m_objIConfReferences.GetSize(); i++)
    {
        if (m_objIConfReferences.GetAt(i) == piReference)
        {
            m_objParticipantList.ResetReference(piReference);
            delete piReference;
            m_objIConfReferences.RemoveAt(i);
            break;
        }
    }
}

PROTECTED VIRTUAL void ConferenceController::UpdateUserStatusByReferResult(IN ConfUser* pUser,
        IN IConferenceReference* piConfRef,
        IN SipStatusCode nStatusCode /* = SipStatusCode::SC_200*/)
{
    IMS_TRACE_D("UpdateUserStatusByReferResult", 0, 0, 0);

    if (pUser == IMS_NULL)
    {
        return;
    }

    if (nStatusCode == SipStatusCode::SC_100)
    {
        if (piConfRef->GetType() == REFERENCE_TYPE_INVITE)
        {
            pUser->eStatus = STATUS_DIALING_OUT;
        }
    }
    else if (nStatusCode == SipStatusCode::SC_200)
    {
        if (piConfRef->GetType() == REFERENCE_TYPE_INVITE)
        {
            pUser->eStatus = STATUS_CONNECTED;
        }
        else if (piConfRef->GetType() == REFERENCE_TYPE_BYE)
        {
            pUser->eStatus = STATUS_DISCONNECTED;
        }
    }

    pUser->eStatusCode = nStatusCode.ToInt();
}

PROTECTED
void ConferenceController::NotifyConferenceInfo()
{
    m_objNotifier.NotifyConferenceInfo(m_objParticipantList);
}

PROTECTED
void ConferenceController::NotifyUsersInfo()
{
    m_objNotifier.NotifyUsersInfo(m_objParticipantList);
}

PROTECTED VIRTUAL void ConferenceController::SubscribeConference(IN IMS_BOOL bUnsub /*= IMS_FALSE*/)
{
    IMS_TRACE_D("SubscribeConference [%s]", _TRACE_B_(bUnsub), 0, 0);

    if (bUnsub)
    {
        StopSubscription();
        return;
    }

    if (CreateSubscription() == IMS_TRUE)
    {
        StartSubscription();
    }
    else
    {
        if (m_pSubscription != IMS_NULL)
        {
            if (m_pSubscription->GetState() == SubscriptionState::ACTIVE)
            {
                // re-SUBSCRIBTION
            }
        }
        else if (IMS_FALSE /*not support conference w/o subscription*/)
        {
            Recover();
            return;
        }
        else
        {
            // carry on w/o subscription
            CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_SUBSCRIBE);
        }
    }
}

PROTECTED VIRTUAL void ConferenceController::CheckUserEntityConnected(IN ConfUser* pConfUser)
{
    if (m_objParticipantList.IsConnectedUser(pConfUser))
    {
        CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_CHECK_CONNECTED);
    }
}

PROTECTED VIRTUAL void ConferenceController::InviteParticipants(IN IMSList<ConfUser*> objUsers)
{
    IMS_TRACE_D("InviteParticipants : [%d] users", objUsers.GetSize(), 0, 0);

    IMS_SINT32 nReferTypeForInvite = ConferenceConfigurationWrapper::GetReferTypeForInvite();
    ClearOngoingReferences();

    if (nReferTypeForInvite == REFER_INVITE_SINGLE)
    {
        StopFinalSipfragWaitTimer();

        // for single case, no need to use for statement. there is only one user in REFER.
        ConfUser* pTemp = objUsers.GetAt(0);
        IConferenceReference* piConfRefer = CreateReference(pTemp);
        m_objIConfReferences.Append(piConfRefer);

        AString strReferInviteUri;
        IMS_RESULT nResult = piConfRefer->SendInvite(strReferInviteUri, m_objConnectionIdManager);
        m_objParticipantList.SetReferInviteUri(strReferInviteUri, pTemp);

        if (nResult == IMS_FAILURE)
        {
            Recover();
        }
    }
    else if (nReferTypeForInvite == REFER_INVITE_MULTIPLE)  // REFER with resource list
    {
        IConferenceReference* piConfRefer = CreateReference(objUsers);
        AString strReferInviteUri;  // not used.
        IMS_RESULT nResult = piConfRefer->SendInvite(strReferInviteUri, m_objConnectionIdManager);
        m_objIConfReferences.Append(piConfRefer);  // TODO: check api call order.

        for (IMS_UINT32 i = 0; i < objUsers.GetSize(); i++)
        {
            ConfUser* pConfUser = objUsers.GetAt(i);
            strReferInviteUri =
                    (pConfUser != IMS_NULL) ? pConfUser->strTarget : AString::ConstNull();
            m_objParticipantList.SetReferInviteUri(strReferInviteUri, pConfUser);
        }

        if (nResult == IMS_FAILURE)
        {
            Recover();
            if (GetState() == STATE_EXPANDING)
            {
                SendClosed();
            }
        }
    }

    m_objParticipantList.Login();
}

PROTECTED VIRTUAL void ConferenceController::RemoveParticipants(IN IMSList<ConfUser*> objUsers)
{
    IMS_TRACE_D("RemoveParticipants", 0, 0, 0);

    for (IMS_UINT32 i = 0; i < objUsers.GetSize(); i++)
    {
        ConfUser* pTemp = objUsers.GetAt(i);
        IConferenceReference* piConfRefer = CreateReference(pTemp);
        m_objIConfReferences.Append(piConfRefer);

        IMS_RESULT nResult = piConfRefer->SendBye(m_objParticipantList.GetReferInviteUri(pTemp));
        if (nResult == IMS_FAILURE)
        {
            // recovery
        }
    }

    m_objParticipantList.Login();
}

PROTECTED VIRTUAL void ConferenceController::NotifyCmdResult()
{
    IMS_TRACE_D("NotifyCmdResult", 0, 0, 0);

    m_objParticipantList.Login();

    switch (GetState())
    {
        case STATE_CREATED:
            break;
        case STATE_GROUPCALLING:
            m_objNotifier.NotifyGroupCallStarted();
            break;
        case STATE_EXPANDING:
            m_objNotifier.NotifyExpanded();
            NotifyUsersInfo();
            break;
        case STATE_MERGING:
            m_objNotifier.NotifyMerged(m_objParticipantList);
            break;
        case STATE_JOINING:
            m_objNotifier.NotifyJoined(CallReasonInfo(CODE_NONE), m_objParticipantList);
            break;
        case STATE_DROPPING:
            m_objNotifier.NotifyDropped(CallReasonInfo(CODE_NONE), m_objParticipantList);
            break;
        case STATE_IDLE:
            break;

        default:
            break;
    }

    if (CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI) == IMS_TRUE)
    {
        SetState(STATE_IDLE);
    }
}

PROTECTED VIRTUAL void ConferenceController::TerminateIndividualCall(IN IMS_UINT32 nConnectionId)
{
    IMS_TRACE_I("TerminateIndividualCall : [%d]", nConnectionId, 0, 0);

    m_objCallManager.GetCallByCallKey(m_objConnectionIdManager.GetCallKey(nConnectionId))
            ->Terminate(CallReasonInfo(CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE, -1));
    CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_TERMINATE_1TO1_CALL);
}

PROTECTED VIRTUAL void ConferenceController::TerminateConference(IN IMS_SINT32 nTerminateReason)
{
    IMS_TRACE_I("TerminateConference", 0, 0, 0);
    GetConferenceCall()->Terminate(CallReasonInfo(nTerminateReason, -1));
    CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_TERMINATE_CONFERENCE);
}

PROTECTED
IMS_BOOL ConferenceController::CompleteCurrentAndDoNextOperation(
        IN IMS_UINT32 nOperationType, IN ConfUser* pConfUser /* = IMS_NULL*/)
{
    IMS_BOOL bResult = m_objOperationQueue.CompleteCurrentOperation(nOperationType, pConfUser);

    if (bResult)
    {
        DoNextOperation();
    }

    return bResult;
}

PROTECTED
void ConferenceController::DoNextOperation()
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            m_objOperationQueue.GetNextOperation();

    if (pOperation == IMS_NULL)
    {
        return;
    }

    switch (pOperation->GetType())
    {
        case CONTROL_OPERATION_CREATE_CONFERENCE_CALL:
            StartConferenceCall(pOperation);
            break;
        case CONTROL_OPERATION_SUBSCRIBE:
            SubscribeConference();
            break;
        case CONTROL_OPERATION_UNSUBSCRIBE:
            SubscribeConference(IMS_TRUE);
            break;
        case CONTROL_OPERATION_REFER_INVITE:
            InviteParticipants(pOperation->GetUsers());
            break;
        case CONTROL_OPERATION_REFER_BYE:
            RemoveParticipants(pOperation->GetUsers());
            break;
        case CONTROL_OPERATION_CHECK_CONNECTED:
            CheckUserEntityConnected(pOperation->GetUsers().GetAt(0));
            break;
        case CONTROL_OPERATION_NOTIFY_RESULT_TO_UI:
            NotifyCmdResult();
            break;
        case CONTROL_OPERATION_TERMINATE_1TO1_CALL:
            TerminateIndividualCall(pOperation->GetConnectionId());
            break;
        case CONTROL_OPERATION_TERMINATE_CONFERENCE:
            TerminateConference(pOperation->GetTerminateReason());
            break;
        case CONTROL_OPERATION_DESTROY_CONTROLLER:
            SendClosed();
            break;
        case CONTROL_OPERATION_NOTIFY_RESULT_TO_MTCCALL:
            NotifyResultToConferenceCall();
            break;

        default:
            break;
    }
}

PROTECTED
void ConferenceController::CheckNStartFinalSipfragWaitTimer(IN IMS_UINT32 nNewCondition)
{
    m_nConditionFinalSipfragTimer |= nNewCondition;

    IMS_TRACE_I("CheckNStartFinalSipfragWaitTimer : [%d]", m_nConditionFinalSipfragTimer, 0, 0);

    if (IsStartFinalSipfragWaitTimer() == IMS_FALSE)
    {
        return;
    }

    StopFinalSipfragWaitTimer();

    m_piTimer = TimerService::GetTimerService()->CreateTimer();

    if (m_piTimer == IMS_NULL)
    {
        return;
    }

    m_piTimer->SetTimer(TIME_FINAL_SIPFRAG_WAIT, this);
}

PROTECTED
void ConferenceController::StopFinalSipfragWaitTimer()
{
    m_nConditionFinalSipfragTimer = CONDITION_NONE;

    if (m_piTimer == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("StopFinalSipfragWaitTimer", 0, 0, 0);

    m_piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTimer);
    m_piTimer = IMS_NULL;
}

PROTECTED VIRTUAL IMS_BOOL ConferenceController::IsStartFinalSipfragWaitTimer() const
{
    IMS_TRACE_I("IsStartFinalSipfragWaitTimer : Base Class returns false.", 0, 0, 0);
    return IMS_FALSE;
}

PROTECTED VIRTUAL void ConferenceController::Recover()
{
    IMS_TRACE_I("Recover : Base Class does nothing.", 0, 0, 0);
}

PROTECTED VIRTUAL void ConferenceController::OnCallUpdated(
        IN IMS_UINT32 nType, IN IMS_UINTP nCallKey)
{
    IMS_TRACE_I("OnCallUpdated : [%d]", nType, 0, 0);

    // check if it's conference session event.
    if (nCallKey != m_nConfCallKey)
    {
        IMS_TRACE_I("OnCallUpdated : not a conference call state update", 0, 0, 0);

        if (nType == CALL_TERMINATED)
        {
            OnIndividualCallTerminated(nCallKey);
        }
        return;
    }

    switch (nType)
    {
        case CALL_STARTED:
            CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_CREATE_CONFERENCE_CALL);
            break;
        case CALL_STARTFAILED:
            Recover();
            SendClosed();
            break;
        case CALL_TERMINATED:
            Recover();
            if (m_pSubscription && m_pSubscription->GetState() == SubscriptionState::ACTIVE)
            {
                IMS_SINT32 nWaitTime =
                        ConferenceConfigurationWrapper::GetWaitTimeNotifyTerminated();
                if (nWaitTime >= 0)
                {
                    m_objOperationQueue.AddDelay(nWaitTime);
                    m_objOperationQueue.CreateNPut(CONTROL_OPERATION_UNSUBSCRIBE, IMS_TRUE);
                }
            }
            SendClosed();
            break;

        default:
            break;
    }
}

PROTECTED VIRTUAL void ConferenceController::OnIndividualCallTerminated(IN IMS_UINTP nCallKey)
{
    IMS_TRACE_I("OnIndividualCallTerminated", 0, 0, 0);

    CheckNStartFinalSipfragWaitTimer(CONDITION_1TO1_TERMINATED);

    if (m_objOperationQueue.GetTypeOfCurrentOperation() == CONTROL_OPERATION_TERMINATE_1TO1_CALL)
    {
        if (nCallKey ==
                m_objConnectionIdManager.GetCallKey(
                        m_objOperationQueue.GetCurrentOperation()->GetConnectionId()))
        {
            CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_TERMINATE_1TO1_CALL);
            return;
        }

        // failure
        Recover();
    }
}

PROTECTED
void ConferenceController::SendClosed()
{
    if (m_objOperationQueue.HasPendingOperation())
    {
        m_objOperationQueue.CreateNPut(CONTROL_OPERATION_DESTROY_CONTROLLER, IMS_TRUE);
    }
    else
    {
        if (m_piListener != IMS_NULL)
        {
            m_piListener->OnClosed(this);
            m_piListener = IMS_NULL;
        }
    }
}

PROTECTED
void ConferenceController::NotifyResultToConferenceCall()
{
    // TODO: resume conference call if held.
    // piConfSession->OnConferenceCompleted(IMS_SUCCESS);
    CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_NOTIFY_RESULT_TO_MTCCALL);
}

PROTECTED
void ConferenceController::GetFocusAddress(OUT AString& strAddress) const
{
    IMtcSession* pMtcSession = GetConferenceCall()->GetCallContext().GetSession();

    if (pMtcSession == IMS_NULL)
    {
        // for unit test case.
        IMS_TRACE_E(0, "GetFocusAddress empty", 0, 0, 0);
        return;
    }

    ISession& objSession = pMtcSession->GetISession();
    IMessage* piMessage = IMS_NULL;

    if (GetConferenceCall()->GetCallContext().GetCallInfo().ePeerType == PeerType::MO)
    {
        piMessage = objSession.GetPreviousResponse(IMessage::SESSION_START);
    }
    else
    {
        piMessage = objSession.GetPreviousRequest(IMessage::SESSION_START);
    }

    MessageUtil::GetUri(piMessage, IMS_TRUE, ISipHeader::CONTACT_NORMAL, strAddress);
}

PROTECTED
void ConferenceController::Init()
{
    IMS_TRACE_I("Initialize", 0, 0, 0);

    m_objOperationQueue.SetListener(this);
    m_objContext.GetCallStateProxy().AddListener(this);

    AString strLocalId =
            GetConferenceCall()->GetCallContext().GetService().GetICoreService()->GetLocalUserId();
    IMS_TRACE_I("Initialize : local user id=[%s]", strLocalId.GetStr(), 0, 0);
    m_objParticipantList.SetLocalUri(strLocalId);
}

PROTECTED
IMtcCall* ConferenceController::GetConferenceCall() const
{
    return m_objCallManager.GetCallByCallKey(m_nConfCallKey);
}

PROTECTED
IMS_BOOL ConferenceController::IsReadyToPerformCmd() const
{
    if (m_nState == STATE_CREATED || m_nState == STATE_IDLE)
    {
        return IMS_TRUE;
    }

    IMS_TRACE_I("IsReadyToPerformCmd : [NOT READY] %s", ConvertStateToString(m_nState), 0, 0);
    return IMS_FALSE;
}

PROTECTED
IMS_BOOL ConferenceController::IsConditionMet(IN IMS_UINT32 nCondition) const
{
    if ((m_nConditionFinalSipfragTimer & nCondition) == nCondition)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
void ConferenceController::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I(
            "SetState : %s to %s", ConvertStateToString(m_nState), ConvertStateToString(nState), 0);
    m_nState = nState;
}

PROTECTED
const IMS_CHAR* ConferenceController::ConvertStateToString(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case STATE_CREATED:
            return "STATE_CREATED";
        case STATE_GROUPCALLING:
            return "STATE_GROUPCALLING";
        case STATE_EXPANDING:
            return "STATE_EXPANDING";
        case STATE_MERGING:
            return "STATE_MERGING";
        case STATE_JOINING:
            return "STATE_JOINING";
        case STATE_DROPPING:
            return "STATE_DROPPING";
        case STATE_IDLE:
            return "STATE_IDLE";
        default:
            return "__INVALID__";
    }
}
