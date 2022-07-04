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

#ifndef CONFERENCE_FACTORY_H_
#define CONFERENCE_FACTORY_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

class IConferenceReference;
class ConferenceSubscription;
class IMtcContext;
class ConferenceParticipantList;
class IConferenceSubscriptionListener;
class IConferenceReferenceListener;
class CallConnectionIdManager;
class IMtcCallContext;
class ConferenceOperationQueue;
class ConferenceEventNotifier;

class ConferenceFactory
{
public:
    explicit ConferenceFactory(IN IMtcContext& objContext);
    virtual ~ConferenceFactory();
    ConferenceFactory(IN const ConferenceFactory&) = delete;
    ConferenceFactory& operator=(IN const ConferenceFactory&) = delete;

    virtual ConferenceSubscription* CreateSubscription(IN CallKey nConfCallKey,
            IN ConferenceParticipantList& objList, IN IConferenceSubscriptionListener& objListener);
    virtual IConferenceReference* CreateReference(IN CallKey nConfCallKey, IN ConfUser* pConfUser,
            IN IConferenceReferenceListener& objListener);
    virtual IConferenceReference* CreateReference(IN CallKey nConfCallKey,
            IN IMSList<ConfUser*>& objConfUsers, IN IConferenceReferenceListener& objListener);
    virtual ConferenceParticipantList* CreateParticipantList();
    virtual ConferenceOperationQueue* CreateOperationQueue();
    virtual ConferenceEventNotifier* CreateEventNotifier(IN IMtcCallContext& objConfCallContext,
            IN CallConnectionIdManager& objConnectionIdManager);

private:
    IMtcContext& m_objContext;
};

#endif
