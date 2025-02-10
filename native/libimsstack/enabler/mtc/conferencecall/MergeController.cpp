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

#include "CarrierConfig.h"
#include "ServiceTrace.h"
#include "call/CallConnectionIdManager.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "conferencecall/ConferenceConfigurationHelper.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceOperationQueue.h"
#include "conferencecall/ConferenceUtils.h"
#include "conferencecall/MergeController.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcSupplementaryService.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MergeController::MergeController(IN CallKey nConfCallKey, IMtcContext& objContext,
        IN CallConnectionIdManager& objConnectionIdManager, IN ConferenceFactory& objFactory) :
        ConferenceController(nConfCallKey, objContext, objConnectionIdManager, objFactory),
        m_eStartCallType(CallType::VOIP)
{
    IMS_TRACE_I("+MergeController", 0, 0, 0);
}

PUBLIC VIRTUAL MergeController::~MergeController()
{
    IMS_TRACE_I("~MergeController", 0, 0, 0);
}

PROTECTED VIRTUAL void MergeController::ProcessMerge(IN ImsList<ConfUser*>& objUsers)
{
    IMS_TRACE_I("ProcessMerge user size[%d]", objUsers.GetSize(), 0, 0);

    if (IsReadyToPerformCmd() == IMS_FALSE)
    {
        m_pNotifier->NotifyMergeFailed(CallReasonInfo(CODE_NONE));
        // SendClosed(); required?
        return;
    }

    if (GetState() == STATE_CREATED && objUsers.GetSize() < 2)
    {
        m_pNotifier->NotifyMergeFailed(CallReasonInfo(CODE_NONE));
        SendClosed();
        return;
    }

    IMS_UINT32 nStartIndex = AddUserToParticipantList(objUsers, IMS_TRUE);
    if (ConferenceConfigurationHelper::GetReferTypeForInvite(
                m_objContext.GetConfigurationProxy()) == ConfigVoice::CONFERENCE_INVITE_COPYCONTROL)
    {
        return ProcessMergeWithoutRefer(objUsers);
    }

    IMS_BOOL bSubFirstAndRefer = ConferenceConfigurationHelper::IsSubscriptionFirst(
                                         m_objContext.GetConfigurationProxy()) ||
            ConferenceConfigurationHelper::IsSubscriptionNotifyRequiredForRefer(
                    m_objContext.GetConfigurationProxy());
    IMS_SINT32 nOldState = GetState();
    SetState(STATE_MERGING);

    if (nOldState == STATE_CREATED)
    {
        UpdateStartCallType(objUsers);
        ClearListForConfUsers(objUsers);

        // TODO: Check if it's okay to use CreateNPut() due to no users.
        m_pOperationQueue->CreateNPutWithUsers(CONTROL_OPERATION_CREATE_CONFERENCE_CALL, objUsers);

        if (bSubFirstAndRefer == IMS_TRUE &&
                ConferenceConfigurationHelper::IsConferenceSubscriptionRequired(
                        m_objContext.GetConfigurationProxy()))
        {
            m_pOperationQueue->CreateNPut(CONTROL_OPERATION_SUBSCRIBE);
        }
    }

    IMS_TRACE_I("ProcessMerge [%d]", m_pParticipantList->GetSize(), 0, 0);
    for (IMS_UINT32 i = nStartIndex; i < m_pParticipantList->GetSize(); i++)
    {
        m_pOperationQueue->CreateNPutWithUser(
                CONTROL_OPERATION_REFER_INVITE, m_pParticipantList->GetConfUsers().GetAt(i));
        m_pOperationQueue->CreateNPutWithId(CONTROL_OPERATION_TERMINATE_1TO1_CALL,
                m_pParticipantList->GetConfUsers().GetAt(i)->nConnectionId);
        if (ConferenceConfigurationHelper::IsSubscriptionNotifyRequiredForRefer(
                    m_objContext.GetConfigurationProxy()))
        {
            m_pOperationQueue->CreateNPutWithUser(
                    CONTROL_OPERATION_CHECK_CONNECTED, m_pParticipantList->GetConfUsers().GetAt(i));
        }
    }

    m_pOperationQueue->CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI);

    if (nOldState == STATE_CREATED)
    {
        if (bSubFirstAndRefer == IMS_FALSE &&
                ConferenceConfigurationHelper::IsConferenceSubscriptionRequired(
                        m_objContext.GetConfigurationProxy()))
        {
            m_pOperationQueue->CreateNPut(CONTROL_OPERATION_SUBSCRIBE);
        }
    }
    m_pOperationQueue->CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_MTCCALL);
    m_pOperationQueue->SetAddingOperationSetCompleted();
}

PROTECTED VIRTUAL void MergeController::StartConferenceCall(
        IN ConferenceOperationQueue::ConferenceOperation* pOperation)
{
    ImsList<ConfUser*> objUsers = pOperation->GetUsers();

    IMS_TRACE_D("StartConferenceCall CallType[%d] UserSize[%d]", m_eStartCallType,
            objUsers.GetSize(), 0);

    GetConferenceCall()->StartConference(m_eStartCallType, AString::ConstNull(), objUsers);
    ClearListForConfUsers(objUsers);
}

PROTECTED VIRTUAL IMS_BOOL MergeController::IsStartFinalSipfragWaitTimer() const
{
    IMS_TRACE_I("IsStartFinalSipfragWaitTimer : [%d]", m_nConditionFinalSipfragTimer, 0, 0);

#if 0  // to cover the case there is not even R-NOTIFY [100 Trying]
    if (IsConditionMet(CONDITION_SIPFRAG_100_RECEIVED) == IMS_FALSE)
    {
        return IMS_FALSE;
    }
#endif

    if (IsConditionMet(CONDITION_1TO1_TERMINATED) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void MergeController::Recover()
{
    // do not call SendClosed() in Recovery logic.
    // it must be called in each logic based on the context.
    IMS_TRACE_I("Recover", 0, 0, 0);

    switch (m_pOperationQueue->GetTypeOfCurrentOperation())
    {
        case CONTROL_OPERATION_CREATE_CONFERENCE_CALL:
            RecoverOnCreating();
            break;

        case CONTROL_OPERATION_TERMINATE_1TO1_CALL:
        case CONTROL_OPERATION_REFER_INVITE:
            RecoverOnReferring();
            break;
        case CONTROL_OPERATION_REFER_BYE:
            // to be handled in OnReferenceStartFailed.
            break;

        default:
            IMS_TRACE_I("Recover : not handled.", 0, 0, 0);
            break;
    }
}

PROTECTED VIRTUAL void MergeController::OnIndividualCallTerminated(IN IMS_UINTP nCallKey)
{
    ConferenceController::OnIndividualCallTerminated(nCallKey);

    if (m_pSubscription == IMS_NULL &&
            ConferenceConfigurationHelper::GetReferTypeForInvite(
                    m_objContext.GetConfigurationProxy()) ==
                    ConfigVoice::CONFERENCE_INVITE_COPYCONTROL)
    {
        UpdateUserStateByCallTerminated(nCallKey);
    }
}

PRIVATE
void MergeController::ProcessMergeWithoutRefer(IN ImsList<ConfUser*>& objUsers)
{
    IMS_TRACE_I("ProcessMergeWithoutRefer", 0, 0, 0);

    IMS_SINT32 nOldState = GetState();
    SetState(STATE_MERGING);

    if (nOldState == STATE_CREATED)
    {
        UpdateStartCallType(objUsers);
        m_pOperationQueue->CreateNPutWithUsers(CONTROL_OPERATION_CREATE_CONFERENCE_CALL, objUsers);
        m_pOperationQueue->CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI);
        m_pOperationQueue->SetAddingOperationSetCompleted();
    }
    else
    {
        ClearListForConfUsers(objUsers);
    }
}

PRIVATE
void MergeController::UpdateUserStateByCallTerminated(IN IMS_UINTP nCallKey)
{
    IMS_TRACE_I("UpdateUserStateByCallTerminated : key[%d]", nCallKey, 0, 0);

    // KDDI
    for (IMS_UINT32 i = 0; i < m_pParticipantList->GetSize(); i++)
    {
        ConfUser* pUser = m_pParticipantList->GetConfUser(i);
        if (m_objConnectionIdManager.GetCallKey(pUser->nConnectionId) == nCallKey)
        {
            pUser->eStatus = STATUS_DISCONNECTED;
            NotifyUsersInfo();
            break;
        }
    }

    if (m_pParticipantList->GetConnectedParticipantSize(IMS_TRUE) == 0)
    {
        IMS_TRACE_D("UpdateUserStateByCallTerminated : terminate conference by alone", 0, 0, 0);
        m_pOperationQueue->CreateNPutWithReason(
                CONTROL_OPERATION_TERMINATE_CONFERENCE, CODE_USER_TERMINATED, IMS_TRUE);
    }
}

PRIVATE
void MergeController::RecoverOnCreating()
{
    RecoverOnConferenceCallFailed();
}

PRIVATE
void MergeController::RecoverOnReferring()
{
    if (RecoverOnConferenceCallFailed())
    {
        return;
    }

    ConfUser* pConfUser = m_pOperationQueue->GetUsersOfCurrentOperation().GetAt(0);
    if (pConfUser && pConfUser->eStatus == STATUS_CONNECTED)
    {
        // abnormal case. network doesn't send R-NOTIFY 200 but C-NOTIFY connected is received
        // the timer must be stopped.
        CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_REFER_INVITE, pConfUser);
        return;
    }

    if (m_pParticipantList->GetConnectedParticipantSize() == 0)
    {
        IMS_TRACE_I("RecoverOnReferring : failure before the first member joined.", 0, 0, 0);
        ClearIndividualCallOnMergeFailed();
        m_pNotifier->NotifyMergeFailed(CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR, -1));
        m_pOperationQueue->Clear();
        SetState(STATE_IDLE);
        return;
    }

    if (pConfUser && m_pParticipantList->GetSize() <= 2)
    {
        IMS_TRACE_I("RecoverOnReferring : failure after at least one member added.", 0, 0, 0);

        pConfUser->eStatus = STATUS_DISCONNECTED;
        CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_REFER_INVITE, pConfUser);
    }
    else
    {
        IMS_TRACE_I("RecoverOnReferring : failure during additional adding.", 0, 0, 0);
        ClearIndividualCallOnMergeFailed();
        m_pNotifier->NotifyMergeFailed(CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR, -1));
        m_pOperationQueue->Clear();
        SetState(STATE_IDLE);
        return;
    }
}

PRIVATE
IMS_BOOL MergeController::RecoverOnConferenceCallFailed()
{
    IMS_TRACE_I("RecoverOnConferenceCallFailed", 0, 0, 0);

    IMtcCall* piConfCall = GetConferenceCall();
    if ((piConfCall->GetState() != IMtcCall::State::ESTABLISHED &&
                piConfCall->GetState() != IMtcCall::State::UPDATING))
    {
        ClearIndividualCallOnMergeFailed();
        m_pNotifier->NotifyMergeFailed(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, -1));
        m_pOperationQueue->Clear();
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void MergeController::ClearIndividualCallOnMergeFailed()
{
    // this must be called before 'merge failed' event.
    IMS_TRACE_I("ClearIndividualCallOnMergeFailed", 0, 0, 0);

    for (IMS_UINT32 i = 0; i < m_pParticipantList->GetSize(); i++)
    {
        if (m_pParticipantList->GetAt(i)->IsInfoUpdated())
        {
            continue;
        }

        ConfUser* pTempUser = m_pParticipantList->GetConfUser(i);
        if (pTempUser == IMS_NULL)
        {
            continue;
        }
        CallKey nTempCallKey = m_objConnectionIdManager.GetCallKey(pTempUser->nConnectionId);

        IConferenceReference* piConfReference = m_pParticipantList->GetAt(i)->GetReference();

        if (piConfReference != IMS_NULL)
        {
            IMS_UINT32 nResponseCode = piConfReference->GetResponseCode();
            if (SipStatusCode::IsFinalFailure(nResponseCode) ||
                    nResponseCode == SipStatusCode::SC_INVALID)
            {
                // refer was rejected or no refer was sent so, call can be maintained
                IMS_TRACE_I("ClearIndividualCallOnMergeFailed : refer failed.", 0, 0, 0);
                continue;
            }

            // refer is sent. so, call is considered terminated
            IMS_TRACE_I("ClearIndividualCallOnMergeFailed : refer is sent.", 0, 0, 0);
            m_pNotifier->NotifyIndividualCallTerminated(nTempCallKey);
            continue;
        }

        if (pTempUser->eStatus != STATUS_IDLE)
        {
            // state is updated already
            IMS_TRACE_I("ClearIndividualCallOnMergeFailed : user state changed", 0, 0, 0);
            m_pNotifier->NotifyIndividualCallTerminated(nTempCallKey);
            continue;
        }

        IMtcCall* piTemp = m_objCallManager.GetCallByCallKey(nTempCallKey);
        if (piTemp == IMS_NULL || piTemp->GetState() == IMtcCall::State::TERMINATING)
        {
            IMS_TRACE_I("ClearIndividualCallOnMergeFailed : 1-to-1 is terminating", 0, 0, 0);
            m_pNotifier->NotifyIndividualCallTerminated(nTempCallKey);
        }
    }
}

PRIVATE
void MergeController::UpdateStartCallType(IN const ImsList<ConfUser*> objUsers)
{
    IMS_BOOL bVoip = IMS_FALSE;
    IMS_BOOL bVt = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < objUsers.GetSize(); i++)
    {
        CallKey nkey = m_objConnectionIdManager.GetCallKey(objUsers.GetAt(i)->nConnectionId);
        CallType eIndividualType = m_objCallManager.GetCallByCallKey(nkey)->GetCallType();

        if (eIndividualType == CallType::VT || eIndividualType == CallType::VIDEO_RTT)
        {
            bVt = IMS_TRUE;
        }
        else
        {
            bVoip = IMS_TRUE;
        }
    }

    if (bVoip && bVt)
    {
        m_eStartCallType = static_cast<CallType>(m_objContext.GetConfigurationProxy().GetInt(
                ConfigVoice::KEY_CALL_TYPE_AFTER_AUDIO_AND_VIDEO_CALL_MERGED_INT));
    }
    else if (bVt)
    {
        m_eStartCallType = CallType::VT;
    }
}
