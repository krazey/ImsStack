/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHout WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ImsEventDef.h"
#include "ImsTypeDef.h"
#include "MockISystem.h"
#include "MockIEventReceiverListener.h"
#include "OsEventReceiver.h"
#include "PlatformContext.h"

using ::testing::_;

namespace android
{

class OsEventReceiverTest : public ::testing::Test
{
public:
    MockISystem m_objMockSystem;
    MockIEventReceiverListener m_objMockEventReceiverListener;

    ISystem* m_piDefaultSystem;
    ISystemListener* m_piSystemListener;
    OsEventReceiver* m_pOsEventReceiver;
    IEventReceiver* m_pIEventReceiver;

protected:
    virtual void SetUp() override
    {
        m_piDefaultSystem = PlatformContext::GetInstance()->SetSystem(&m_objMockSystem);

        m_pOsEventReceiver = new OsEventReceiver(IMS_SLOT_0);
        ASSERT_TRUE(m_pOsEventReceiver != nullptr);

        m_piSystemListener = static_cast<ISystemListener*>(m_pOsEventReceiver);
        m_pIEventReceiver = static_cast<IEventReceiver*>(m_pOsEventReceiver);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetSystem(m_piDefaultSystem);

        if (m_pOsEventReceiver != IMS_NULL)
        {
            delete m_pOsEventReceiver;
            m_pOsEventReceiver = IMS_NULL;
        }
    }
};

TEST_F(OsEventReceiverTest, ResetEvent)
{
    EXPECT_CALL(m_objMockSystem, ResetEvent(IMS_EVENT_ROAMING_STATE, _)).Times(1);
    m_pIEventReceiver->ResetEvent(IMS_EVENT_ROAMING_STATE);
}

TEST_F(OsEventReceiverTest, SetEvent)
{
    EXPECT_CALL(m_objMockSystem, SetEvent(IMS_EVENT_ROAMING_STATE, _)).Times(1);
    m_pIEventReceiver->SetEvent(IMS_EVENT_ROAMING_STATE);
}

TEST_F(OsEventReceiverTest, SetListener)
{
    EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(1);
    m_pIEventReceiver->SetListener(&m_objMockEventReceiverListener);

    EXPECT_CALL(m_objMockEventReceiverListener,
            EventReceiver_NotifyEvent(IMS_EVENT_ROAMING_STATE, _, _))
            .Times(1);
    m_piSystemListener->System_NotifyEvent(IMS_EVENT_ROAMING_STATE, 0, 0);
}

}  // namespace android
