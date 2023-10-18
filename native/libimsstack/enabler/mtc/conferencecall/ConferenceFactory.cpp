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

#include "IMtcContext.h"
#include "ImsList.h"
#include "ServiceTrace.h"
#include "conferencecall/ConferenceEventNotifier.h"
#include "conferencecall/ConferenceFactory.h"
#include "conferencecall/ConferenceInfo.h"
#include "conferencecall/ConferenceInfoUpdater.h"
#include "conferencecall/ConferenceOperationQueue.h"
#include "conferencecall/ConferenceParticipantList.h"
#include "conferencecall/ConferenceReference.h"
#include "conferencecall/ConferenceSubscription.h"
#include "conferencecall/IConferenceReference.h"
#include "conferencecall/IConferenceReferenceListener.h"
#include "conferencecall/IConferenceSubscriptionListener.h"
#include "configuration/MtcConfigurationProxy.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ConferenceFactory::ConferenceFactory(IN IMtcContext& objContext) :
        m_objContext(objContext)
{
    IMS_TRACE_I("+ConferenceFactory", 0, 0, 0);
}

PUBLIC VIRTUAL ConferenceFactory::~ConferenceFactory()
{
    IMS_TRACE_I("~ConferenceFactory", 0, 0, 0);
}

PUBLIC VIRTUAL ConferenceSubscription* ConferenceFactory::CreateSubscription(
        IN CallKey nConfCallKey, IN ConferenceParticipantList& objList,
        IN IConferenceSubscriptionListener& objListener)
{
    return new ConferenceSubscription(m_objContext, nConfCallKey, objList, objListener, *this);
}

PUBLIC VIRTUAL IConferenceReference* ConferenceFactory::CreateReference(IN CallKey nConfCallKey,
        IN ConfUser* pConfUser, IN IConferenceReferenceListener& objListener)
{
    return new ConferenceReference(m_objContext, nConfCallKey, pConfUser, objListener);
}

PUBLIC VIRTUAL IConferenceReference* ConferenceFactory::CreateReference(IN CallKey nConfCallKey,
        IN ImsList<ConfUser*>& objConfUsers, IN IConferenceReferenceListener& objListener)
{
    return new ConferenceReference(m_objContext, nConfCallKey, objConfUsers, objListener);
}

PUBLIC VIRTUAL ConferenceParticipantList* ConferenceFactory::CreateParticipantList()
{
    return new ConferenceParticipantList();
}

PUBLIC VIRTUAL ConferenceOperationQueue* ConferenceFactory::CreateOperationQueue()
{
    return new ConferenceOperationQueue();
}

PUBLIC VIRTUAL ConferenceEventNotifier* ConferenceFactory::CreateEventNotifier(
        IN IMtcCallContext& objConfCallContext, IN CallConnectionIdManager& objConnectionIdManager)
{
    return new ConferenceEventNotifier(objConfCallContext, objConnectionIdManager);
}

PUBLIC VIRTUAL ConferenceInfoUpdater* ConferenceFactory::CreateInfoUpdater()
{
    return new ConferenceInfoUpdater(*this, m_objContext.GetConfigurationProxy());
}

PUBLIC VIRTUAL ConferenceInfo* ConferenceFactory::CreateInfo()
{
    return new ConferenceInfo();
}
