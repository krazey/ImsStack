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

#include "CallReasonInfo.h"
#include "IJniEnabler.h"
#include "IJniMtcCallThread.h"
#include "IuMtcCall.h"
#include "JniEnablerConnector.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "call/CallConnectionIdManager.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcSession.h"
#include "call/IMtcUiNotifier.h"
#include "conferencecall/ConferenceEventNotifier.h"
#include "conferencecall/ConferenceParticipantList.h"
#include "helper/MtcSupplementaryService.h"
#include "media/IMtcMediaManager.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ConferenceEventNotifier::ConferenceEventNotifier(IN IMtcCallManager& objCallManager,
        IN CallKey nConferenceCallKey, IN CallConnectionIdManager& objConnectionIdManager) :
        m_objCallManager(objCallManager),
        m_nConferenceCallKey(nConferenceCallKey),
        m_objConnectionIdManager(objConnectionIdManager)
{
    IMS_TRACE_I("+ConferenceEventNotifier", 0, 0, 0);
}

PUBLIC VIRTUAL ConferenceEventNotifier::~ConferenceEventNotifier()
{
    IMS_TRACE_I("~ConferenceEventNotifier", 0, 0, 0);
}

PUBLIC
void ConferenceEventNotifier::NotifyMerged(
        IN ConferenceParticipantList& objParticipantList, IN IMS_BOOL bSubscribed)
{
    IMS_TRACE_I("NotifyMerged", 0, 0, 0);

    objParticipantList.LogLn();
    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread)
    {
        IMtcCallContext* piConferenceCallContext = GetConferenceCallContext();
        if (piConferenceCallContext == IMS_NULL)
        {
            return;
        }

        piThread->OnMerged(piConferenceCallContext->CreateJniCallInfo(),
                piConferenceCallContext->GetMediaManager().GetMediaInfo(
                        &piConferenceCallContext->GetSession()->GetISession()),
                piConferenceCallContext->GetSupplementaryService().GetServices(),
                bSubscribed ? objParticipantList.GetConfUsers() : ImsList<ConfUser*>());
    }
}

PUBLIC
void ConferenceEventNotifier::NotifyMergeFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("NotifyMergeFailed", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread)
    {
        piThread->OnMergeFailed(objReason);
    }
}

PUBLIC
void ConferenceEventNotifier::NotifyGroupCallStarted()
{
    IMS_TRACE_I("NotifyGroupCallStarted", 0, 0, 0);
}

PUBLIC
void ConferenceEventNotifier::NotifyGroupCallFailed(IN const CallReasonInfo& /*objReason*/)
{
    IMS_TRACE_I("NotifyGroupCallFailed", 0, 0, 0);
}

PUBLIC
void ConferenceEventNotifier::NotifyExpanded()
{
    IMS_TRACE_I("NotifyExpanded", 0, 0, 0);
}

PUBLIC
void ConferenceEventNotifier::NotifyExpandFailed(IN const CallReasonInfo& /*objReason*/)
{
    IMS_TRACE_I("NotifyExpandFailed", 0, 0, 0);
}

PUBLIC
void ConferenceEventNotifier::NotifyDropped(IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyDropped", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread)
    {
        piThread->OnConferenceParticipantRemoved();
    }

    NotifyUsersInfo(objParticipantList);
}

PUBLIC
void ConferenceEventNotifier::NotifyDropFailed(
        IN const CallReasonInfo& objReason, IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyDropFailed", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread)
    {
        piThread->OnConferenceParticipantRemoveFailed(objReason);
    }
    NotifyUsersInfo(objParticipantList);
}

PUBLIC
void ConferenceEventNotifier::NotifyJoined(IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyJoined", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread)
    {
        piThread->OnConferenceParticipantAdded();
    }
    NotifyUsersInfo(objParticipantList);
}

PUBLIC
void ConferenceEventNotifier::NotifyJoinFailed(
        IN const CallReasonInfo& objReason, IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyJoinFailed", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread)
    {
        piThread->OnConferenceParticipantAddFailed(objReason);
    }
    NotifyUsersInfo(objParticipantList);
}

PUBLIC
void ConferenceEventNotifier::NotifyConferenceInfo(IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyConferenceInfo : max-user-count=[%d]", objParticipantList.GetMaxUserCount(),
            0, 0);
    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread)
    {
        piThread->OnConferenceInfoChanged(
                "", "", objParticipantList.GetSize(), objParticipantList.GetMaxUserCount(), "");
    }
}

PUBLIC
void ConferenceEventNotifier::NotifyUsersInfo(IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyUsersInfo", 0, 0, 0);
    objParticipantList.LogLn();

    ImsList<ConfUser*> objUsers = objParticipantList.GetConfUsers();
    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread)
    {
        piThread->OnConferenceParticipantsInfoChanged(objUsers);
    }
    CheckDisconnectedConfUsersInfo(objParticipantList, objUsers);
}

PUBLIC
void ConferenceEventNotifier::NotifyIndividualCallTerminated(IN CallKey nKey)
{
    IMS_TRACE_I("NotifyIndividualCallTerminated ", 0, 0, 0);

    IMtcCall* piCall = m_objCallManager.GetCallByCallKey(nKey);
    if (piCall->GetKey() == IMtcCall::CALL_KEY_INVALID)
    {
        IMS_TRACE_I("NotifyIndividualCallTerminated - call is already deleted.", 0, 0, 0);
        return;
    }

    piCall->GetCallContext().GetUiNotifier().SendTerminated(
            CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, CODE_USER_TERMINATED_BY_REMOTE));
}

PRIVATE
void ConferenceEventNotifier::CheckDisconnectedConfUsersInfo(
        IN ConferenceParticipantList& objParticipantList, IN_OUT ImsList<ConfUser*>& /*objUsers*/)
{
    for (IMS_SINT32 i = static_cast<IMS_SINT32>(objParticipantList.GetSize()) - 1; i >= 0; i--)
    {
        ConferenceParticipantList::ConferenceParticipant* pParticipant =
                objParticipantList.GetAt(i);

        if (pParticipant == IMS_NULL)
        {
            continue;
        }

        IMS_UINT32 nStatus = pParticipant->GetConfUser()->eStatus;
        if (nStatus == STATUS_DISCONNECTED || nStatus == STATUS_DISCONNECTING ||
                (nStatus >= STATUS_FAIL && nStatus <= STATUS_INTSERVERERROR))
        {
            m_objConnectionIdManager.OnConferenceParticipantDisconnected(
                    pParticipant->GetConfUser()->nConnectionId);
            objParticipantList.RemoveUser(i);
        }
        else
        {
            pParticipant->SetDisconnectionNotified(IMS_FALSE);
        }
    }
}

PRIVATE
IMtcCallContext* ConferenceEventNotifier::GetConferenceCallContext() const
{
    IMtcCall* piConferenceCall = m_objCallManager.GetCallByCallKey(m_nConferenceCallKey);
    if (piConferenceCall->GetKey() == IMtcCall::CALL_KEY_INVALID)
    {
        return IMS_NULL;
    }
    return &piConferenceCall->GetCallContext();
}

PRIVATE
IJniMtcCallThread* ConferenceEventNotifier::GetCallThread() const
{
    const IMtcCallContext* piConferenceCallContext = GetConferenceCallContext();
    if (piConferenceCallContext == IMS_NULL)
    {
        return IMS_NULL;
    }

    const IJniEnabler* piJniMtcCall = JniEnablerConnector::GetInstance().GetJniEnabler(
            piConferenceCallContext->GetSlotId(), EnablerType::MTC_CALL, m_nConferenceCallKey);
    if (piJniMtcCall == IMS_NULL)
    {
        IMS_TRACE_D("GetCallThread : No JniMtcCall", 0, 0, 0);
        return IMS_NULL;
    }

    return reinterpret_cast<IJniMtcCallThread*>(piJniMtcCall->GetJniThread());
}
