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

#ifndef MOCK_CONFERENCE_SUBSCRIPTION_H_
#define MOCK_CONFERENCE_SUBSCRIPTION_H_

#include "call/IMtcCall.h"
#include "conferencecall/ConferenceSubscription.h"
#include <gmock/gmock.h>

class ConferenceFactory;
class ConferenceParticipantList;
class ISession;
class ICoreService;
class ISubscription;
class IMtcContext;
enum class SubscriptionState;

class MockConferenceSubscription : public ConferenceSubscription
{
public:
    explicit MockConferenceSubscription(IN IMtcContext& objContext, IN CallKey nConfCallKey,
            IN ConferenceParticipantList& objList, IN IConferenceSubscriptionListener& objListener,
            IN ConferenceFactory& objFactory) :
            ConferenceSubscription(objContext, nConfCallKey, objList, objListener, objFactory)
    {
    }
    ~MockConferenceSubscription() override {}
    MOCK_METHOD(void, SubscriptionForkedNotify, (IN ISubscription*, IN ISubscription*), (override));
    MOCK_METHOD(void, SubscriptionNotify,
            (IN ISubscription* piSubscription, IN IMessage* piNotify), (override));
    MOCK_METHOD(void, SubscriptionStarted, (IN ISubscription* piSubscription), (override));
    MOCK_METHOD(void, SubscriptionStartFailed, (IN ISubscription* piSubscription), (override));
    MOCK_METHOD(void, SubscriptionTerminated, (IN ISubscription* piSubscription), (override));
    MOCK_METHOD(IMS_RESULT, Subscribe, (IN const AString& strTo), (override));
    MOCK_METHOD(void, UnSubscribe, (), (override));
    MOCK_METHOD(SubscriptionState, GetState, (), (override));
    MOCK_METHOD(void, SetState, (IN SubscriptionState nState), ());
    MOCK_METHOD(void, CreateSubscription, (), ());
    MOCK_METHOD(IMS_RESULT, Subscribe, (), ());
    MOCK_METHOD(void, ReSubscribe, (), ());
    MOCK_METHOD(void, Initialize, (), ());
    MOCK_METHOD(void, SetHeaders, (), ());
    MOCK_METHOD(void, UpdateConferenceInfo, (IN IMessage* piNotify), ());
    MOCK_METHOD(void, HandleUpdateResult, (IN IMS_UINT32 nResult), ());
    MOCK_METHOD(void, Notify, (), ());
    MOCK_METHOD(IMS_BOOL, OnReceiving403, (IN ISubscription* piSubscription), ());
    MOCK_METHOD(IMS_BOOL, OnReceiving423, (IN ISubscription* piSubscription), ());
    MOCK_METHOD(void, ReleaseISubscription, (), ());
};

#endif
