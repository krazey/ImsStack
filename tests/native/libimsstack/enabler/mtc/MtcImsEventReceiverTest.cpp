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

#include "ImsEventDef.h"
#include "MockIMtcImsEventReceiver.h"
#include "MtcImsEventReceiver.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;

// From MtcImsEventReceiver::RegisterSupportedEvents()
const LOCAL ImsEvent SUPPORTED_EVENT = IMS_EVENT_IMS_VOICE_OVER_PS_STATE;
const LOCAL ImsEvent INVALID_EVENT = -1;
const LOCAL IMS_SINT32 SLOT_ID = 0;

class MtcImsEventReceiverTest : public ::testing::Test
{
public:
    MtcImsEventReceiver* pEventReceiver;

protected:
    virtual void SetUp() override { pEventReceiver = new MtcImsEventReceiver(SLOT_ID); }

    virtual void TearDown() override { delete pEventReceiver; }
};

TEST_F(MtcImsEventReceiverTest, GetParamWithInvalidEventReturnsUnknownValue)
{
    EXPECT_EQ(MtcImsEventReceiver::UNKNOWN_VALUE, pEventReceiver->GetWParam(INVALID_EVENT));
    EXPECT_EQ(MtcImsEventReceiver::UNKNOWN_VALUE, pEventReceiver->GetLParam(INVALID_EVENT));
}

TEST_F(MtcImsEventReceiverTest, GetParamInitiallyReturnsUnknownValue)
{
    EXPECT_EQ(MtcImsEventReceiver::UNKNOWN_VALUE, pEventReceiver->GetWParam(SUPPORTED_EVENT));
    EXPECT_EQ(MtcImsEventReceiver::UNKNOWN_VALUE, pEventReceiver->GetLParam(SUPPORTED_EVENT));
}

TEST_F(MtcImsEventReceiverTest, GetParamReturnsNewValueAfterNotify)
{
    pEventReceiver->Event_NotifyEvent(SUPPORTED_EVENT, 1, 2);

    EXPECT_EQ(1, pEventReceiver->GetWParam(SUPPORTED_EVENT));
    EXPECT_EQ(2, pEventReceiver->GetLParam(SUPPORTED_EVENT));
}

TEST_F(MtcImsEventReceiverTest, AddListenerDoesNothingForInvalidEvent)
{
    MockIMtcImsEventListener objEventListener;

    pEventReceiver->AddListener(&objEventListener, INVALID_EVENT);
}

TEST_F(MtcImsEventReceiverTest, RemoveListenerDoesNothingForInvalidEvent)
{
    MockIMtcImsEventListener objEventListener;

    pEventReceiver->RemoveListener(&objEventListener, INVALID_EVENT);
}

TEST_F(MtcImsEventReceiverTest, Event_NotifyEventDoesNothingForInvalidEvent)
{
    MockIMtcImsEventListener objEventListener;

    EXPECT_CALL(objEventListener, OnImsEventNotified(_, _, _)).Times(0);

    pEventReceiver->AddListener(&objEventListener, SUPPORTED_EVENT);
    pEventReceiver->Event_NotifyEvent(INVALID_EVENT, 1, 2);
}

TEST_F(MtcImsEventReceiverTest, Event_NotifyEventDoesNothingForSameValue)
{
    MockIMtcImsEventListener objEventListener;

    EXPECT_CALL(objEventListener, OnImsEventNotified(SUPPORTED_EVENT, 1, 2)).Times(1);

    pEventReceiver->AddListener(&objEventListener, SUPPORTED_EVENT);
    pEventReceiver->Event_NotifyEvent(SUPPORTED_EVENT, 1, 2);
    pEventReceiver->Event_NotifyEvent(SUPPORTED_EVENT, 1, 2);
}

TEST_F(MtcImsEventReceiverTest, Event_NotifyEventNotifiesAddedListeners)
{
    MockIMtcImsEventListener objEventListener1;
    MockIMtcImsEventListener objEventListener2;

    EXPECT_CALL(objEventListener1, OnImsEventNotified(SUPPORTED_EVENT, 1, 2));
    EXPECT_CALL(objEventListener2, OnImsEventNotified(SUPPORTED_EVENT, 1, 2));

    pEventReceiver->AddListener(&objEventListener1, SUPPORTED_EVENT);
    pEventReceiver->AddListener(&objEventListener2, SUPPORTED_EVENT);
    pEventReceiver->Event_NotifyEvent(SUPPORTED_EVENT, 1, 2);
}

TEST_F(MtcImsEventReceiverTest, Event_NotifyEventNotNotifiesRemovedListeners)
{
    MockIMtcImsEventListener objEventListener1;
    MockIMtcImsEventListener objEventListener2;

    EXPECT_CALL(objEventListener1, OnImsEventNotified(SUPPORTED_EVENT, 1, 2));
    EXPECT_CALL(objEventListener2, OnImsEventNotified(_, _, _)).Times(0);

    pEventReceiver->AddListener(&objEventListener1, SUPPORTED_EVENT);
    pEventReceiver->AddListener(&objEventListener2, SUPPORTED_EVENT);
    pEventReceiver->RemoveListener(&objEventListener2, SUPPORTED_EVENT);
    pEventReceiver->Event_NotifyEvent(SUPPORTED_EVENT, 1, 2);
}

TEST_F(MtcImsEventReceiverTest, Event_NotifyEventNotifiesDuplicatedListenerOnce)
{
    MockIMtcImsEventListener objEventListener;

    EXPECT_CALL(objEventListener, OnImsEventNotified(SUPPORTED_EVENT, 1, 2));

    pEventReceiver->AddListener(&objEventListener, SUPPORTED_EVENT);
    pEventReceiver->AddListener(&objEventListener, SUPPORTED_EVENT);
    pEventReceiver->Event_NotifyEvent(SUPPORTED_EVENT, 1, 2);
}
