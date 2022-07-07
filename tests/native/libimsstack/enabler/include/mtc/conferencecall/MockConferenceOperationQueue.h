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

#ifndef MOCK_CONFERENCE_OPERATION_QUEUE_H_
#define MOCK_CONFERENCE_OPERATION_QUEUE_H_

#include <gmock/gmock.h>
#include "ImsList.h"
#include "ImsMap.h"
#include "ITimer.h"
#include "IuMtcCall.h"
#include "MtcDef.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/IConferenceOperationQueueListener.h"
#include "conferencecall/ConferenceOperationQueue.h"

class MockConferenceOperationQueue : public ConferenceOperationQueue
{
public:
    ~MockConferenceOperationQueue() {}
    MOCK_METHOD(void, Timer_TimerExpired, (IN ITimer * piTimer), (override));
    MOCK_METHOD(void, SetListener, (IN IConferenceOperationQueueListener * piListener), ());
    MOCK_METHOD(void, AddDelay, (IN IMS_UINT32 nDelayMillisec), ());
    MOCK_METHOD(
            void, CreateNPut, (IN IMS_UINT32 nType, IN IMS_BOOL bStandAloneOperation), (override));
    MOCK_METHOD(void, CreateNPutWithUsers,
            (IN IMS_UINT32 nType, IN IMSList<ConfUser*> objUsers, IN IMS_BOOL bStandAloneOperation),
            (override));
    MOCK_METHOD(void, CreateNPutWithUser,
            (IN IMS_UINT32 nType, IN ConfUser* pConfUser, IN IMS_BOOL bStandAloneOperation),
            (override));
    MOCK_METHOD(void, CreateNPutWithStartParam,
            (IN IMS_UINT32 nType, IN CallStartOperationParams* pParams,
                    IN IMS_BOOL bStandAloneOperation),
            (override));
    MOCK_METHOD(void, CreateNPutWithId,
            (IN IMS_UINT32 nType, IN IMS_UINT32 nConnectionId, IN IMS_BOOL bStandAloneOperation),
            (override));
    MOCK_METHOD(void, CreateNPutWithReason,
            (IN IMS_UINT32 nType, IN IMS_SINT32 nTerminateReason, IN IMS_BOOL bStandAloneOperation),
            (override));
    MOCK_METHOD(void, SetAddingOperationSetCompleted, (), ());
    MOCK_METHOD(ConferenceOperationQueue::ConferenceOperation*, GetNextOperation, (), (override));
    MOCK_METHOD(IMS_BOOL, CompleteCurrentOperation,
            (IN IMS_UINT32 nOperationType, IN ConfUser* pConfUser), (override));
    MOCK_METHOD(ConferenceOperationQueue::ConferenceOperation*, GetCurrentOperation, (),
            (const, override));
    MOCK_METHOD(IMS_UINT32, GetTypeOfCurrentOperation, (), (const, override));
    MOCK_METHOD(const IMSList<ConfUser*>&, GetUsersOfCurrentOperation, (), (const, override));
    MOCK_METHOD(IMS_BOOL, HasPendingOperation, (), (const, override));
    MOCK_METHOD(void, Remove, (IN ConferenceOperation * pOperation), ());
    MOCK_METHOD(void, Clear, (), ());
    MOCK_METHOD(
            void, Put, (IN ConferenceOperation * pOperation, IN IMS_BOOL bStandAloneOperation), ());
    MOCK_METHOD(void, RemoveActiveOperation, (), ());
    MOCK_METHOD(IMS_BOOL, IsSameOperation, (IN IMS_UINT32 nOperationType, IN ConfUser* pConfUser),
            (const));
    MOCK_METHOD(IMS_UINT32, GetAndResetDelay, (), ());
    MOCK_METHOD(IMS_RESULT, StartTimer, (IN IMS_SINT32 nDuration), ());
    MOCK_METHOD(void, StopTimer, (), ());
    MOCK_METHOD(const IMS_CHAR*, ConvertOperationToString, (IN IMS_SINT32 nOperation), (const));
};

#endif
