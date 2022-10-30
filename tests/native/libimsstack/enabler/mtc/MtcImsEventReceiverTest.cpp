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
#include "MtcImsEventReceiver.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// From MtcImsEventReceiver::RegisterSupportedEvents()
const ImsEvent SUPPORTED_EVENT = IMS_EVENT_IMS_VOICE_OVER_PS_STATE;
const IMS_SINT32 SLOT_ID = 0;

class TestImsEventListener : public IMtcImsEventListener
{
public:
    virtual ~TestImsEventListener() {}

    virtual void OnImsEventNotified(
            IN ImsEvent nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
    {
        m_nNotifyCount += 1;
        m_nEvent = nEvent;
        m_nWParam = nWParam;
        m_nLParam = nLParam;
    }

    int m_nNotifyCount = 0;
    ImsEvent m_nEvent = 0;
    IMS_UINT32 m_nWParam = 0;
    IMS_UINT32 m_nLParam = 0;
};

TEST(MtcImsEventReceiverTest, GetParamInitiallyReturnsUnknownValue)
{
    MtcImsEventReceiver objEventReceiver(SLOT_ID);

    EXPECT_EQ(MtcImsEventReceiver::UNKNOWN_VALUE, objEventReceiver.GetWParam(SUPPORTED_EVENT));
    EXPECT_EQ(MtcImsEventReceiver::UNKNOWN_VALUE, objEventReceiver.GetLParam(SUPPORTED_EVENT));
}

TEST(MtcImsEventReceiverTest, GetParamReturnsNewValueAfterNotify)
{
    MtcImsEventReceiver objEventReceiver(SLOT_ID);

    objEventReceiver.Event_NotifyEvent(SUPPORTED_EVENT, 1, 2);

    EXPECT_EQ(1, objEventReceiver.GetWParam(SUPPORTED_EVENT));
    EXPECT_EQ(2, objEventReceiver.GetLParam(SUPPORTED_EVENT));
}

TEST(MtcImsEventReceiverTest, AddedListenerNotifiedOnRegisteredEvent)
{
    MtcImsEventReceiver objEventReceiver(SLOT_ID);
    TestImsEventListener objEventListener;

    objEventReceiver.AddListener(&objEventListener, SUPPORTED_EVENT);
    objEventReceiver.Event_NotifyEvent(SUPPORTED_EVENT, 1, 2);

    EXPECT_EQ(SUPPORTED_EVENT, objEventListener.m_nEvent);
    EXPECT_EQ(1, objEventListener.m_nWParam);
    EXPECT_EQ(2, objEventListener.m_nLParam);
}

TEST(MtcImsEventReceiverTest, RemovedListenerNotNotifiedOnRegisteredEvent)
{
    MtcImsEventReceiver objEventReceiver(SLOT_ID);
    TestImsEventListener objEventListener;

    objEventReceiver.AddListener(&objEventListener, SUPPORTED_EVENT);
    objEventReceiver.RemoveListener(&objEventListener, SUPPORTED_EVENT);
    objEventReceiver.Event_NotifyEvent(SUPPORTED_EVENT, 1, 2);

    EXPECT_EQ(0, objEventListener.m_nNotifyCount);
}

TEST(MtcImsEventReceiverTest, DuplicatedListenerNotifiedOnceOnRegisteredEvent)
{
    MtcImsEventReceiver objEventReceiver(SLOT_ID);
    TestImsEventListener objEventListener;

    objEventReceiver.AddListener(&objEventListener, SUPPORTED_EVENT);
    objEventReceiver.AddListener(&objEventListener, SUPPORTED_EVENT);
    objEventReceiver.Event_NotifyEvent(SUPPORTED_EVENT, 1, 2);

    EXPECT_EQ(1, objEventListener.m_nNotifyCount);
}
