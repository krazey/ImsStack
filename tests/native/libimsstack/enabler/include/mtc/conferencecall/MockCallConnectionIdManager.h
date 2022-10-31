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

#ifndef MOCK_CALL_CONNECTION_ID_MANAGER_H_
#define MOCK_CALL_CONNECTION_ID_MANAGER_H_

#include "ImsTypeDef.h"
#include "conferencecall/CallConnectionIdManager.h"
#include <gmock/gmock.h>

class IMtcContext;

class MockCallConnectionIdManager : public CallConnectionIdManager
{
public:
    explicit MockCallConnectionIdManager(IN IMtcContext& objContext) :
            CallConnectionIdManager(objContext)
    {
    }
    ~MockCallConnectionIdManager() {}
    // MOCK_METHOD(void, OnCallStateChanged, (IN CallKey nCallKey, IN State eState, IN Type eType,
    // IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason), (override)); MOCK_METHOD(void,
    // OnTotalCallStateChanged, (IN State eState), (override)); MOCK_METHOD(IMS_BOOL,
    // IsSynchronousCallRequired, (), (override)); MOCK_METHOD(void, OnConferenceCallStarted, (IN
    // IConferenceController* piController, IN IMS_BOOL bStarted), ());
    MOCK_METHOD(
            void, OnConferenceParticipantDisconnected, (IN IMS_UINT32 nConnectionId), (override));
    // MOCK_METHOD(IMS_SINT32, GetIndex, (IN CallKey nKey), ());
    MOCK_METHOD(CallKey, GetCallKey, (IN IMS_UINT32 nConnectionId), (override));
    // MOCK_METHOD(IMS_UINT32, GetNewIndex, (), ());
    // MOCK_METHOD(IMS_SINT32, GetListIndexByCallKey, (IN CallKey nCallKey), ());
    // MOCK_METHOD(IMS_SINT32, GetListIndexByConnectionId, (IN IMS_UINT32 nConnectionId), ());
    // MOCK_METHOD(void, AddKeyConnectionId, (IN CallKey nCallKey), ());
    // MOCK_METHOD(void, RemoveKeyConnectionId, (IN IMS_SINT32 nIndex), ());
    // MOCK_METHOD(IMS_BOOL, IsConferenceParticipant, (IN CallKey nCallKey), ());
    // MOCK_METHOD(AString, GetIds, (), ());
    // MOCK_METHOD(explicit, CallKeyConnection, (IN CallKey nKey_, IN IMS_UINT32 nConnectionId_),
    // ()); MOCK_METHOD(~, CallKeyConnection, (), ());
};

#endif
