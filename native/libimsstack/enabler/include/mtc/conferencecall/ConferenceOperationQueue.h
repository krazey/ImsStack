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

#ifndef CONFERENCE_OPERATION_QUEUE_H_
#define CONFERENCE_OPERATION_QUEUE_H_

#include "CallReasonInfo.h"
#include "ITimer.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/IConferenceOperationQueueListener.h"

struct CallInfo;

struct CallStartOperationParams
{
public:
    // TODO: copy or reference.
    // if copy and delete every time an operation is deleted, then too many copy.
    CallStartOperationParams(IN IMS_UINT32 _nType, IN CallInfo& _objCallInfo,
            IN MediaInfo& _objMediaInfo, IN ImsList<ConfUser*>& _objUsers,
            IN ImsMap<SuppType, SuppService*>& _objSuppServices) :
            nType(_nType),
            objCallInfo(_objCallInfo),
            objMediaInfo(_objMediaInfo),
            objUsers(_objUsers),
            objSuppServices(_objSuppServices)
    {
    }

public:
    IMS_UINT32 nType;
    CallInfo& objCallInfo;
    MediaInfo& objMediaInfo;
    ImsList<ConfUser*>& objUsers;
    ImsMap<SuppType, SuppService*>& objSuppServices;
};

class ConferenceOperationQueue : public ITimerListener
{
public:
    struct ConferenceOperation
    {
    public:
        // constructor
        inline ConferenceOperation(IN IMS_UINT32 nType, IN IMS_UINT32 nDelayMillisec) :
                m_nType(nType),
                m_nDelayMillisec(nDelayMillisec),
                m_objConfUsers(ImsList<ConfUser*>()),
                m_pParam(IMS_NULL),
                m_nConnectionId(0),
                m_nTerminateReason(CODE_NONE)
        {
        }

        // destructor
        inline ~ConferenceOperation() { delete m_pParam; }

        // setters
        inline void SetConfUsers(IN const ImsList<ConfUser*>& objConfUsers)
        {
            m_objConfUsers = objConfUsers;
        }

        inline void SetConfUser(IN ConfUser* pConfUser) { m_objConfUsers.Append(pConfUser); }

        inline void SetParam(IN CallStartOperationParams* pParam) { m_pParam = pParam; }

        inline void SetConnectionId(IN IMS_UINT32 nConnectionId)
        {
            m_nConnectionId = nConnectionId;
        }

        inline void SetTerminateReason(IN IMS_SINT32 nTerminateReason)
        {
            m_nTerminateReason = nTerminateReason;
        }

        inline void RemoveTimerValue() { m_nDelayMillisec = 0; }

        // getters
        inline IMS_UINT32 GetType() { return m_nType; }
        inline IMS_UINT32 GetDelayMilliSec() { return m_nDelayMillisec; }
        inline const ImsList<ConfUser*>& GetUsers() const { return m_objConfUsers; }
        inline CallStartOperationParams* GetParam() { return m_pParam; }
        inline IMS_UINT32 GetConnectionId() { return m_nConnectionId; }
        inline IMS_SINT32 GetTerminateReason() { return m_nTerminateReason; }

    private:
        IMS_UINT32 m_nType;
        IMS_UINT32 m_nDelayMillisec;
        ImsList<ConfUser*> m_objConfUsers;
        CallStartOperationParams* m_pParam;
        IMS_UINT32 m_nConnectionId;
        IMS_SINT32 m_nTerminateReason;
    };

public:
    explicit ConferenceOperationQueue();
    virtual ~ConferenceOperationQueue() override;
    ConferenceOperationQueue(IN const ConferenceOperationQueue&) = delete;
    ConferenceOperationQueue& operator=(IN const ConferenceOperationQueue&) = delete;

public:
    // implements ITimerListener interfaces.
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    void SetListener(IN IConferenceOperationQueueListener* piListener);
    void AddDelay(IN IMS_UINT32 nDelayMillisec);

    virtual void CreateNPut(IN IMS_UINT32 nType, IN IMS_BOOL bStandAloneOperation = IMS_FALSE);
    virtual void CreateNPutWithUsers(IN IMS_UINT32 nType, IN const ImsList<ConfUser*>& objUsers,
            IN IMS_BOOL bStandAloneOperation = IMS_FALSE);
    virtual void CreateNPutWithUser(IN IMS_UINT32 nType, IN ConfUser* pConfUser,
            IN IMS_BOOL bStandAloneOperation = IMS_FALSE);
    virtual void CreateNPutWithStartParam(IN IMS_UINT32 nType, IN CallStartOperationParams* pParams,
            IN IMS_BOOL bStandAloneOperation = IMS_FALSE);
    virtual void CreateNPutWithId(IN IMS_UINT32 nType, IN IMS_UINT32 nConnectionId,
            IN IMS_BOOL bStandAloneOperation = IMS_FALSE);
    virtual void CreateNPutWithReason(IN IMS_UINT32 nType, IN IMS_SINT32 nTerminateReason,
            IN IMS_BOOL bStandAloneOperation = IMS_FALSE);

    virtual void SetAddingOperationSetCompleted();

    virtual ConferenceOperationQueue::ConferenceOperation* GetNextOperation();
    virtual IMS_BOOL CompleteCurrentOperation(
            IN IMS_UINT32 nOperationType, IN ConfUser* pConfUser = IMS_NULL);
    virtual ConferenceOperationQueue::ConferenceOperation* GetCurrentOperation() const;
    virtual IMS_UINT32 GetTypeOfCurrentOperation() const;
    virtual const ImsList<ConfUser*>& GetUsersOfCurrentOperation() const;

    virtual IMS_BOOL HasPendingOperation() const;

    virtual void Clear();

private:
    void ClearInternal();
    void Put(IN ConferenceOperation* pOperation, IN IMS_BOOL bStandAloneOperation);
    void RemoveActiveOperation();
    IMS_BOOL IsSameOperation(IN IMS_UINT32 nOperationType, IN const ConfUser* pConfUser) const;
    IMS_UINT32 GetAndResetDelay();

    IMS_RESULT StartTimer(IN IMS_SINT32 nDuration);
    void StopTimer();

    static const IMS_CHAR* ConvertOperationToString(IN IMS_SINT32 nOperation);

private:
    static const IMS_UINT32 DELAY_IMMEDIATELY = 0;
    static const IMS_UINT32 ACTIVE_OPERATION_NUMBER = 0;  // 0th operation is the active one.
    static const IMS_UINT32 TIMER_PERFORM_DELAYED_OPERATION = 0;

    ImsList<ConferenceOperation*> m_objOperationQueue;
    IConferenceOperationQueueListener* m_piListener;
    ITimer* m_pIDelayedOperationTimer;
    IMS_UINT32 m_nNextDelay;
    IMS_BOOL m_bNeedToStartOperationSet;
};

#endif
