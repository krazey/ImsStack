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
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "conferencecall/ConferenceOperationQueue.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ConferenceOperationQueue::ConferenceOperationQueue() :
        m_piListener(IMS_NULL),
        m_pIDelayedOperationTimer(IMS_NULL),
        m_nNextDelay(0),
        m_bNeedToStartOperationSet(IMS_FALSE)
{
    IMS_TRACE_D("+ConferenceOperationQueue", 0, 0, 0);
}

PUBLIC
ConferenceOperationQueue::~ConferenceOperationQueue()
{
    IMS_TRACE_D("~ConferenceOperationQueue", 0, 0, 0);
    ClearInternal();
    StopTimer();
}

PUBLIC VIRTUAL void ConferenceOperationQueue::Timer_TimerExpired(IN ITimer* piTimer)
{
    if ((m_pIDelayedOperationTimer != IMS_NULL) && (m_pIDelayedOperationTimer == piTimer))
    {
        IMS_TRACE_D("Timer_TimerExpired", 0, 0, 0);

        StopTimer();

        m_objOperationQueue.GetAt(0)->RemoveTimerValue();
        m_piListener->OnOperationReady();
    }
}

PUBLIC
void ConferenceOperationQueue::SetListener(IN IConferenceOperationQueueListener* piListener)
{
    IMS_TRACE_D("SetListener", 0, 0, 0);
    m_piListener = piListener;
}

PUBLIC
void ConferenceOperationQueue::AddDelay(IN IMS_UINT32 nDelayMillisec)
{
    IMS_TRACE_D("AddDelay : [%d]", nDelayMillisec, 0, 0);
    m_nNextDelay = nDelayMillisec;
}

PUBLIC
void ConferenceOperationQueue::CreateNPut(
        IN IMS_UINT32 nType, IN IMS_BOOL bStandAloneOperation /* = IMS_FALSE*/)
{
    Put(new ConferenceOperation(nType, GetAndResetDelay()), bStandAloneOperation);
}

PUBLIC
void ConferenceOperationQueue::CreateNPutWithUsers(IN IMS_UINT32 nType,
        IN const ImsList<ConfUser*>& objUsers, IN IMS_BOOL bStandAloneOperation /* = IMS_FALSE*/)
{
    ConferenceOperation* pConferenceOperation = new ConferenceOperation(nType, GetAndResetDelay());
    pConferenceOperation->SetConfUsers(objUsers);
    Put(pConferenceOperation, bStandAloneOperation);
}

PUBLIC
void ConferenceOperationQueue::CreateNPutWithUser(IN IMS_UINT32 nType, IN ConfUser* pConfUser,
        IN IMS_BOOL bStandAloneOperation /* = IMS_FALSE*/)
{
    ConferenceOperation* pConferenceOperation = new ConferenceOperation(nType, GetAndResetDelay());
    pConferenceOperation->SetConfUser(pConfUser);
    Put(pConferenceOperation, bStandAloneOperation);
}

PUBLIC
void ConferenceOperationQueue::CreateNPutWithStartParam(IN IMS_UINT32 nType,
        IN CallStartOperationParams* pParams, IN IMS_BOOL bStandAloneOperation /* = IMS_FALSE*/)
{
    ConferenceOperation* pConferenceOperation = new ConferenceOperation(nType, GetAndResetDelay());
    pConferenceOperation->SetParam(pParams);
    Put(pConferenceOperation, bStandAloneOperation);
}

PUBLIC
void ConferenceOperationQueue::CreateNPutWithId(IN IMS_UINT32 nType, IN IMS_UINT32 nConnectionId,
        IN IMS_BOOL bStandAloneOperation /* = IMS_FALSE*/)
{
    // IMS_TRACE_D("CreateNPut : nConnectionId [%d]", nType, 0, 0);

    ConferenceOperation* pConferenceOperation = new ConferenceOperation(nType, GetAndResetDelay());
    pConferenceOperation->SetConnectionId(nConnectionId);
    Put(pConferenceOperation, bStandAloneOperation);
}

PUBLIC
void ConferenceOperationQueue::CreateNPutWithReason(IN IMS_UINT32 nType,
        IN IMS_SINT32 nTerminateReason, IN IMS_BOOL bStandAloneOperation /* = IMS_FALSE*/)
{
    // IMS_TRACE_D("CreateNPut : nTerminateReason [%d]", nType, 0, 0);

    ConferenceOperation* pConferenceOperation = new ConferenceOperation(nType, GetAndResetDelay());
    pConferenceOperation->SetTerminateReason(nTerminateReason);
    Put(pConferenceOperation, bStandAloneOperation);
}

PUBLIC
void ConferenceOperationQueue::SetAddingOperationSetCompleted()
{
    IMS_TRACE_D("SetAddingOperationSetCompleted", 0, 0, 0);
    if (m_bNeedToStartOperationSet)
    {
        m_bNeedToStartOperationSet = IMS_FALSE;
        m_piListener->OnOperationReady();
    }
}

PUBLIC
ConferenceOperationQueue::ConferenceOperation* ConferenceOperationQueue::GetNextOperation()
{
    if (m_objOperationQueue.GetSize() == 0)
    {
        return IMS_NULL;
    }

    ConferenceOperation* pFirstOperation = m_objOperationQueue.GetAt(0);
    if (pFirstOperation->GetDelayMilliSec() > DELAY_IMMEDIATELY)
    {
        StartTimer(pFirstOperation->GetDelayMilliSec());
        return IMS_NULL;
    }

    IMS_TRACE_D("GetNextOperation : size=[%d] type=[%s]", m_objOperationQueue.GetSize(),
            ConvertOperationToString(pFirstOperation->GetType()), 0);

    return pFirstOperation;
}

PUBLIC
IMS_BOOL ConferenceOperationQueue::CompleteCurrentOperation(
        IN IMS_UINT32 nOperationType, IN ConfUser* pConfUser /* = IMS_NULL*/)
{
    // this is called to check a type of operation done.
    IMS_TRACE_D("CompleteCurrentOperation : [%s]", ConvertOperationToString(nOperationType), 0, 0);

    if (m_objOperationQueue.GetSize() == 0)
    {
        IMS_TRACE_D("CompleteCurrentOperation : no active operation", 0, 0, 0);
        return IMS_FALSE;
    }

    if (IsSameOperation(nOperationType, pConfUser))
    {
        // then, this operation is completed. do the next.
        RemoveActiveOperation();
        StopTimer();
        return IMS_TRUE;
    }

    IMS_TRACE_I("CompleteCurrentOperation : waiting operation is [%s]",
            ConvertOperationToString(m_objOperationQueue.GetAt(ACTIVE_OPERATION_NUMBER)->GetType()),
            0, 0);
    return IMS_FALSE;
}

PUBLIC
ConferenceOperationQueue::ConferenceOperation* ConferenceOperationQueue::GetCurrentOperation() const
{
    return m_objOperationQueue.GetAt(ACTIVE_OPERATION_NUMBER);
}

PUBLIC
IMS_UINT32 ConferenceOperationQueue::GetTypeOfCurrentOperation() const
{
    if (m_objOperationQueue.GetSize() > 0)
    {
        return m_objOperationQueue.GetAt(ACTIVE_OPERATION_NUMBER)->GetType();
    }

    return CONTROL_OPERATION_NONE;
}

PUBLIC
const ImsList<ConfUser*>& ConferenceOperationQueue::GetUsersOfCurrentOperation() const
{
    return m_objOperationQueue.GetAt(ACTIVE_OPERATION_NUMBER)->GetUsers();
}

PUBLIC
IMS_BOOL ConferenceOperationQueue::HasPendingOperation() const
{
    for (IMS_UINT32 i = 0; i < m_objOperationQueue.GetSize(); i++)
    {
        if (m_objOperationQueue.GetAt(i) != IMS_NULL &&
                m_objOperationQueue.GetAt(i)->GetDelayMilliSec() > DELAY_IMMEDIATELY)
        {
            IMS_TRACE_D("HasPendingOperation : HAS!", 0, 0, 0);
            return IMS_TRUE;
        }
    }

    IMS_TRACE_D("HasPendingOperation : Nothing Pended!", 0, 0, 0);
    return IMS_FALSE;
}

PUBLIC
void ConferenceOperationQueue::Clear()
{
    ClearInternal();
}

PRIVATE
void ConferenceOperationQueue::ClearInternal()
{
    for (IMS_UINT32 i = 0; i < m_objOperationQueue.GetSize(); i++)
    {
        delete m_objOperationQueue.GetAt(i);
    }

    m_objOperationQueue.Clear();
    StopTimer();
}

PRIVATE
void ConferenceOperationQueue::Put(
        IN ConferenceOperation* pOperation, IN IMS_BOOL bStandAloneOperation)
{
    IMS_TRACE_D("Put : [%s]", ConvertOperationToString(pOperation->GetType()), 0, 0);

    m_objOperationQueue.Append(pOperation);

    if (m_objOperationQueue.GetSize() == 1)
    {
        if (bStandAloneOperation)
        {
            m_bNeedToStartOperationSet = IMS_FALSE;
            m_piListener->OnOperationReady();
        }
        else
        {
            m_bNeedToStartOperationSet = IMS_TRUE;
        }
    }
}

PRIVATE
void ConferenceOperationQueue::RemoveActiveOperation()
{
    IMS_UINT32 nSize = m_objOperationQueue.GetSize();
    IMS_TRACE_D("RemoveActiveOperation : size=[%d] type=[%s]", nSize,
            ConvertOperationToString(m_objOperationQueue.GetAt(ACTIVE_OPERATION_NUMBER)->GetType()),
            0);

    if (nSize == 0)
    {
        return;
    }

    delete m_objOperationQueue.GetAt(ACTIVE_OPERATION_NUMBER);
    m_objOperationQueue.RemoveAt(ACTIVE_OPERATION_NUMBER);
}

PRIVATE
IMS_BOOL ConferenceOperationQueue::IsSameOperation(
        IN IMS_UINT32 nOperationType, IN const ConfUser* pConfUser) const
{
    ConferenceOperation* pCurrentOperation = m_objOperationQueue.GetAt(ACTIVE_OPERATION_NUMBER);
    if (pCurrentOperation->GetType() == nOperationType)
    {
        if (pConfUser == IMS_NULL || pCurrentOperation->GetUsers().GetAt(0) == pConfUser)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_UINT32 ConferenceOperationQueue::GetAndResetDelay()
{
    IMS_UINT32 nTemp = m_nNextDelay;
    m_nNextDelay = 0;

    return nTemp;
}

PRIVATE
IMS_RESULT ConferenceOperationQueue::StartTimer(IN IMS_SINT32 nDuration)
{
    IMS_TRACE_D("StartTimer : duration[%d]", nDuration, 0, 0);

    if (m_pIDelayedOperationTimer == IMS_NULL)
    {
        m_pIDelayedOperationTimer = TimerService::GetTimerService()->CreateTimer();

        if (m_pIDelayedOperationTimer == IMS_NULL)
        {
            return IMS_FAILURE;
        }

        m_pIDelayedOperationTimer->SetTimer(nDuration, this);
    }

    return IMS_SUCCESS;
}

PRIVATE
void ConferenceOperationQueue::StopTimer()
{
    if (m_pIDelayedOperationTimer == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("StopTimer", 0, 0, 0);

    m_pIDelayedOperationTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_pIDelayedOperationTimer);
    m_pIDelayedOperationTimer = IMS_NULL;
}

PRIVATE
const IMS_CHAR* ConferenceOperationQueue::ConvertOperationToString(IN IMS_SINT32 nOperation)
{
    switch (nOperation)
    {
        case CONTROL_OPERATION_NONE:
            return "CONTROL_OPERATION_NONE";
        case CONTROL_OPERATION_CREATE_CONFERENCE_CALL:
            return "CONTROL_OPERATION_CREATE_CONFERENCE_CALL";
        case CONTROL_OPERATION_SUBSCRIBE:
            return "CONTROL_OPERATION_SUBSCRIBE";
        case CONTROL_OPERATION_UNSUBSCRIBE:
            return "CONTROL_OPERATION_UNSUBSCRIBE";
        case CONTROL_OPERATION_REFER_INVITE:
            return "CONTROL_OPERATION_REFER_INVITE";
        case CONTROL_OPERATION_REFER_BYE:
            return "CONTROL_OPERATION_REFER_BYE";
        case CONTROL_OPERATION_CHECK_CONNECTED:
            return "CONTROL_OPERATION_CHECK_CONNECTED";
        case CONTROL_OPERATION_NOTIFY_RESULT_TO_UI:
            return "CONTROL_OPERATION_NOTIFY_RESULT_TO_UI";
        case CONTROL_OPERATION_TERMINATE_1TO1_CALL:
            return "CONTROL_OPERATION_TERMINATE_1TO1_CALL";
        case CONTROL_OPERATION_TERMINATE_CONFERENCE:
            return "CONTROL_OPERATION_TERMINATE_CONFERENCE";
        case CONTROL_OPERATION_DESTROY_CONTROLLER:
            return "CONTROL_OPERATION_DESTROY_CONTROLLER";
        case CONTROL_OPERATION_NOTIFY_RESULT_TO_MTCCALL:
            return "CONTROL_OPERATION_NOTIFY_RESULT_TO_MTCCALL";

        default:
            return "__INVALID__";
    }
}
