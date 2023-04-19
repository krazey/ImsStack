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
#include "IuMtcCall.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "call/CallConnectionIdManager.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcUiNotifier.h"
#include "conferencecall/ConferenceEventNotifier.h"
#include "conferencecall/ConferenceParticipantList.h"
#include "helper/MtcSupplementaryService.h"
#include "media/IMtcMediaManager.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ConferenceEventNotifier::ConferenceEventNotifier(IN IMtcCallContext& objConfCallContext,
        IN CallConnectionIdManager& objConnectionIdManager) :
        m_objConfCallContext(objConfCallContext),
        m_objConnectionIdManager(objConnectionIdManager)
{
    // TODO: memory leak.
    IMS_TRACE_I("+ConferenceEventNotifier", 0, 0, 0);
}

PUBLIC VIRTUAL ConferenceEventNotifier::~ConferenceEventNotifier()
{
    IMS_TRACE_I("~ConferenceEventNotifier", 0, 0, 0);
}

PUBLIC
void ConferenceEventNotifier::NotifyMerged(IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyMerged", 0, 0, 0);

    objParticipantList.Login();

    ImsList<ConfUser*> objUsers = objParticipantList.GetConfUsers();
    m_objConfCallContext.GetUiNotifier().SendMerged(IMS_NULL,
            m_objConfCallContext.GetMediaManager().GetMediaInfo(),
            m_objConfCallContext.GetSupplementaryService().GetServices(), objUsers);
}

PUBLIC
void ConferenceEventNotifier::NotifyMergeFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("NotifyMergeFailed", 0, 0, 0);

    m_objConfCallContext.GetUiNotifier().SendMergeFailed(objReason);
}

PUBLIC
void ConferenceEventNotifier::NotifyGroupCallStarted()
{
    IMS_TRACE_I("NotifyGroupCallStarted", 0, 0, 0);

    // piCall->SetStartedToUI();
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

    // piCall->SetStartedToUI();
}

PUBLIC
void ConferenceEventNotifier::NotifyExpandFailed(IN const CallReasonInfo& /*objReason*/)
{
    IMS_TRACE_I("NotifyExpandFailed", 0, 0, 0);
}

PUBLIC
void ConferenceEventNotifier::NotifyDropped(
        IN const CallReasonInfo& objReason, IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyDropped", 0, 0, 0);

    m_objConfCallContext.GetUiNotifier().SendDropped(IMS_TRUE, objReason);

    NotifyUsersInfo(objParticipantList);
}

PUBLIC
void ConferenceEventNotifier::NotifyDropFailed(
        IN const CallReasonInfo& objReason, IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyDropFailed", 0, 0, 0);

    m_objConfCallContext.GetUiNotifier().SendDropped(IMS_FALSE, objReason);

    NotifyUsersInfo(objParticipantList);
}

PUBLIC
void ConferenceEventNotifier::NotifyJoined(
        IN const CallReasonInfo& objReason, IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyJoined", 0, 0, 0);

    m_objConfCallContext.GetUiNotifier().SendJoined(IMS_TRUE, objReason);

    NotifyUsersInfo(objParticipantList);
}

PUBLIC
void ConferenceEventNotifier::NotifyJoinFailed(
        IN const CallReasonInfo& objReason, IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyJoinFailed", 0, 0, 0);

    m_objConfCallContext.GetUiNotifier().SendJoined(IMS_FALSE, objReason);

    NotifyUsersInfo(objParticipantList);
}

PUBLIC
void ConferenceEventNotifier::NotifyConferenceInfo(IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyConferenceInfo : max-user-count=[%d]", objParticipantList.GetMaxUserCount(),
            0, 0);

    m_objConfCallContext.GetUiNotifier().SendNotifyConfInfo(
            "", "", objParticipantList.GetMaxUserCount(), objParticipantList.GetSize(), "");
}

PUBLIC
void ConferenceEventNotifier::NotifyUsersInfo(IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyUsersInfo", 0, 0, 0);
    objParticipantList.Login();

    ImsList<ConfUser*> objUsers = objParticipantList.GetConfUsers();
    m_objConfCallContext.GetUiNotifier().SendNotifyUsersInfo(objUsers);
    CheckDisconnectedConfUsersInfo(objParticipantList, objUsers);
}

PUBLIC
void ConferenceEventNotifier::NotifyIndividualCallTerminated(IN CallKey nKey)
{
    IMS_TRACE_I("NotifyIndividualCallTerminated ", 0, 0, 0);

    IMtcCall* piCall = m_objConfCallContext.GetCallManager().GetCallByCallKey(nKey);
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
            // ??
            pParticipant->SetDisconnectionNotified(IMS_FALSE);
        }
    }
}
