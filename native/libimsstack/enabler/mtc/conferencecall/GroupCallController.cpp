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

#include "MtcDef.h"
#include "ImsList.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "conferencecall/ConferenceConfigurationHelper.h"
#include "conferencecall/ConferenceOperationQueue.h"
#include "conferencecall/GroupCallController.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
GroupCallController::GroupCallController(IN CallKey nConfCallKey, IMtcContext& objContext,
        IN CallConnectionIdManager& objConnectionIdManager, IN ConferenceFactory& objFactory) :
        ConferenceController(nConfCallKey, objContext, objConnectionIdManager, objFactory)
{
    IMS_TRACE_I("+GroupCallController", 0, 0, 0);
}

PUBLIC VIRTUAL GroupCallController::~GroupCallController()
{
    IMS_TRACE_I("~GroupCallController", 0, 0, 0);
}

PUBLIC VIRTUAL void GroupCallController::OnReferenceStartFailed(IN IConferenceReference* piConfRef)
{
    IMS_TRACE_D("OnReferenceStartFailed", 0, 0, 0);

    if (piConfRef->GetType() != REFERENCE_TYPE_INVITE)
    {
        return ConferenceController::OnReferenceStartFailed(piConfRef);
    }

    StopFinalSipfragWaitTimer();
    ConfUser* pTempUser = m_pParticipantList->GetConfUser(piConfRef);

    if (pTempUser != IMS_NULL)
    {
        pTempUser->eStatus = STATUS_FAIL;
    }

    Recover();
    CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_REFER_INVITE, pTempUser);
    RemoveReference(piConfRef);

    if ((GetState() == STATE_JOINING) && (m_objIConfReferences.GetSize() <= 0))
    {
        m_pOperationQueue->CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, IMS_TRUE);
    }
}

#ifdef _PROTECTED_METHOD_
#endif

PROTECTED VIRTUAL void GroupCallController::ProcessGroupCall(IN ImsList<ConfUser*>& objUsers,
        IN CallInfo& objCallInfo, IN MediaInfo& objMediaInfo,
        IN ImsList<SuppService*>& objSuppServices)
{
    IMS_TRACE_I("ProcessGroupCall", 0, 0, 0);
    if (IsReadyToPerformCmd() == IMS_FALSE)
    {
        m_pNotifier->NotifyGroupCallFailed(CallReasonInfo(CODE_LOCAL_ILLEGAL_STATE, -1));
        return;
    }

    SetState(STATE_GROUPCALLING);
    AddUserToParticipantList(objUsers);
    ClearListForConfUsers(objUsers);

    CallStartOperationParams* pParams = new CallStartOperationParams(
            CONF_CREATE_START, objCallInfo, objMediaInfo, objUsers, objSuppServices);

    m_pOperationQueue->CreateNPutWithStartParam(CONTROL_OPERATION_CREATE_CONFERENCE_CALL, pParams);
    m_pOperationQueue->CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI);
    m_pOperationQueue->CreateNPut(CONTROL_OPERATION_SUBSCRIBE);

    m_pOperationQueue->SetAddingOperationSetCompleted();
}

PUBLIC VIRTUAL void GroupCallController::StartConferenceCall(
        IN ConferenceOperationQueue::ConferenceOperation*)
{
    /*
    IMtcCall* piCall = m_objCallManager.GetCallByCallKey(m_objConfCallContext.GetCallKey());
    if (piCall == IMS_NULL)
    {
        delete pParams;
        Recover();
        SendClosed();
    }

    piCall->StartConference(CallType::VOIP, pParams->objSuppServices, &(pParams->objMediaInfo),
            pParams->objUsers);
    */
}

PROTECTED VIRTUAL void GroupCallController::Recover()
{
    IMS_TRACE_I("Recover", 0, 0, 0);

    switch (m_pOperationQueue->GetTypeOfCurrentOperation())
    {
        case CONTROL_OPERATION_CREATE_CONFERENCE_CALL:
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

PRIVATE
void GroupCallController::RecoverOnCreating()
{
    if (GetState() != STATE_GROUPCALLING)
    {
        IMS_TRACE_D("RecoverOnCreating : nothing to be handled", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("RecoverOnCreating", 0, 0, 0);

    m_pNotifier->NotifyGroupCallFailed(CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR, -1));
    m_pOperationQueue->Clear();
    SetState(STATE_IDLE);
}

PRIVATE
void GroupCallController::RecoverOnReferring()
{
    if (GetState() != STATE_JOINING)
    {
        IMS_TRACE_D("RecoverOnReferring : nothing to be handled", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("RecoverOnReferring : state[%d]", GetState(), 0, 0);
    NotifyUsersInfo();
}
